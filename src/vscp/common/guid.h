///////////////////////////////////////////////////////////////////////////////
// guid.h:
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
// SOFTWARE./


#if !defined(AFX_GUID_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_GUID_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#include "wx/wx.h"

#include <vscp.h>


// GUID
class cguid  
{

public:

    cguid();
    virtual ~cguid();

public:

    /*!
        Assignment overload
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
    void getFromString( const std::string& strGUID );

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
    void toString( std::string& strGUID  );

    /*!
        GUID to string
        @return GUID as string
    */ 
    wxString getAsString( void ) { wxString str; toString( str ); return str; };

    /*!
        Return pointer to GUID
        \return pointer to GUID.
    */
    const uint8_t *getGUID( void ) { return m_id; };

    /*!
        Get GUID for specific position
        \param pos Position 0-15 for GUID byte to get.
        \return GUID for position
    */
    uint8_t getGUID( const unsigned char n ) 
                { int pos; pos = n & 0xf; return m_id[ pos ]; };


    /*!
        Get a byte from a specific GUID position
    */
    uint8_t getAt( const unsigned char n ) 
                { int pos; pos = ( n & 0x0f ); return m_id[ pos ]; };

    /*!
        Set a byte at a specific GUID position
    */
    void setAt( const unsigned char n, const unsigned char value ) 
                { int pos; pos = ( n & 0x0f ); m_id[ pos ] = value; };

    /*!
        Set LSB GUID position
    */
    void setLSB( const unsigned char value ) { m_id[ 15 ] = value; };

    /*!
        Get LSB GUID position
    */
    uint8_t getLSB( void ) { return m_id[ 15 ]; };

    /*!
        Get Nickname GUID position
    */
    uint16_t getNicknameID( void ) { return (m_id[14] << 8) + m_id[15]; };
    
    /*!
        Get Byte Nickname GUID position
    */
    uint8_t getNickname( void ) { return m_id[ 15 ]; };
    
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
     *  Get Client ID in GUID
     *  This is mostly for use by the VSCP daemon.
     * @return clientid 
     */
    uint16_t getClientID( void );
    
    /*!
     *  Set nickname ID in GUID
     * @param clientid Client id ti write
     */
    void setNicknameID( uint16_t nicknameid );    
    
              
    // GUID id  MSB (index=0) -> LSB (index=15)
    uint8_t m_id[16];
    
};

#endif  

