///////////////////////////////////////////////////////////////////////////////
// guid.h:
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://can.sourceforge.net)
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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


#if !defined(AFX_GUID_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_GUID_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#include "wx/wx.h"

#include <vscp.h>

class cguid  
{

public:

    cguid();
    virtual ~cguid();

public:

	/*!
		Assignement overload
	*/
	cguid& operator=( const cguid& guid);

	/*!
		Positive compare
	*/
	bool operator==(const cguid &guid);

	/*!
		Negative compare
	*/
	bool operator!=(const cguid &guid);

    /*!
        Nill the GUID
    */
    void clear( void ) { memset( &m_id, 0, 16 ); };
    

    /*!
        Get GUID from string
        @param strGUID The GUID in string form
    */
    void getFromString( const wxString& strGUID );

    /*!
        Get GUID from string
        @param pszGUID Zero terminated ASCII string pointing at GUID
    */
    void getFromString( const char *pszGUID );

	/*!
		Set GUID from array
		@param pguid Pointer to array that holds 16 GUID bytes on
						MSB->LSB form.
	*/
	void getFromArray( const uint8_t *pguid );

    /*!
        GUID to string
        @param pszGUID Zero terminated ASCII string pointing at GUID
    */
    void toString( wxString& strGUID  );

    /*!
        Return pointer to GUID
        \return pointer to GUID.
    */
    const uint8_t* getGUID( void ) { return m_id; };

    /*!
        Get GUID for specific position
        \param pos Position 0-15 for GUID byte to get.
        \return GUID for position
    */
    uint8_t getGUID( const unsigned char n ) { int pos; pos = n & 0xf; return m_id[ pos ]; };


    /*!
        Get a byte from a specific GUID position
    */
    uint8_t getAt( const unsigned char n ) { int pos; pos = ( n & 0x0f ); return m_id[ pos ]; };

    /*!
        Set a byte at a specific GUID position
    */
    void setAt( const unsigned char n, const unsigned char value ) { int pos; pos = ( n & 0x0f ); m_id[ pos ] = value; };

    /*!
        Set LSB GUID position
    */
    void setLSB( const unsigned char value ) { m_id[ 15 ] = value; };

    /*!
        Get LSB GUID position
    */
    uint8_t getLSB( void ) { return m_id[ 15 ]; };
    
    /*!
        Fill array with GUID MSB first (STANDARD way)
    */
    void writeGUID( uint8_t *pArray );
    

    /*!
        Fill array with GUID LSB first
    */
    void writeGUID_reverse( uint8_t *pArray );
    
    
    /*!
        Check if same as supplied GUID
        \return true if same.
    */
    bool isSameGUID( const unsigned char *pguid );

    /*!
        Check if GUID is all zero
        @return True if all bytes of GUID is zero, else false.
    */
    bool isNULL( void );
    
    /*!
     *  Set Client ID in GUID
     *  This is mostly for use by the VSCP daemon.
     * @param clientid Client id ti write
     */
    void setClientID( uint16_t clientid );
    
    /*!
     *  Set nickname ID in GUID
     * @param clientid Client id ti write
     */
    void setNicknameID( uint16_t nicknameid );    
              
    // GUID id  MSB (index=0) -> LSB (index=15)
	uint8_t m_id[16];
 	
};

#endif  

