///////////////////////////////////////////////////////////////////////////////
// guid.cpp: 
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
// SOFTWARE.




#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h" 

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include <wx/tokenzr.h>

#include <algorithm> 
#include <functional> 
#include <memory>    
#include <deque>
#include <vector>
#include <string>
#include <cctype>
#include <locale>

#include "vscphelper.h"
#include "guid.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cguid::cguid()
{
    clear();
}


cguid::~cguid()
{  
    ;
}

///////////////////////////////////////////////////////////////////////////////
// operator=
//

cguid& cguid::operator=( const cguid& guid )
{
    // Check for self-assignment!
    if ( this == &guid ) {  // Same object?
        return *this;       // Yes, so skip assignment, and just return *this.
    }

    memcpy( m_id, guid.m_id, 16 );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// operator==
//

bool cguid::operator==( const cguid &guid )
{
    if ( 0 != memcmp( m_id, guid.m_id, 16 ) ) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// operator!=
//

bool cguid::operator!=(const cguid &guid) 
{
    return !(*this == guid);
}

///////////////////////////////////////////////////////////////////////////////
// getFromString
//
 

void cguid::getFromString( const wxString& strGUID )
{
    unsigned long val;
    wxString wxstr = strGUID;

    // Check for default string (all nills)
    wxstr.Trim();
    if ( "-" == wxstr ) {
        clear();
        return;
    }
    
    wxStringTokenizer tkz( strGUID, wxT ( ":" ) );
    for ( int i=0; i<16; i++ ) {
        tkz.GetNextToken().ToULong( &val, 16 );
        m_id[ i ] = ( uint8_t ) val;
        // If no tokens left no meaning to continue
        if ( !tkz.HasMoreTokens() ) break;
    }
}

#ifdef VSCP_NO_WX
void cguid::getFromString( const std::string& strGUID )
{
    unsigned long val;
    std::string str = strGUID;

    // Check for default string (all nills)
    vscp2_trim( str );
    if ( "-" == str ) {
        clear();
        return;
    }
    
    std::deque<std::string> tokens;
    vscp2_split( tokens, str, ":" );
    for ( int i=0; i<16; i++ ) {
        if ( tokens.size() ) {
            std::string tok = tokens.front();
            try {
                std::size_t pos;
                m_id[ i ] = ( uint8_t )std::stoul( tok, &pos, 16 );
            }
            catch (std::invalid_argument) {
                m_id[ i ] = 0;
            }
            tokens.pop_front();
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// getFromString
//


void cguid::getFromString( const char *pszGUID )
 {
    wxString str;
    str = wxString::FromUTF8( pszGUID );
    getFromString( str );
 }


///////////////////////////////////////////////////////////////////////////////
// getFromArray
//

void cguid::getFromArray( const uint8_t *pguid )
{
    memcpy(m_id, pguid, 16 );
}



///////////////////////////////////////////////////////////////////////////////
// toString
//


void cguid::toString( wxString& strGUID  )
{
    strGUID.Printf( _( "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X" ),
                    m_id[0], m_id[1], m_id[2], m_id[3],
                    m_id[4], m_id[5], m_id[6], m_id[7],
                    m_id[8], m_id[9], m_id[10], m_id[11],
                    m_id[12], m_id[13], m_id[14], m_id[15] );
}

#ifdef VSCP_NO_WX
void cguid::toString( std::string& strGUID  )
{
    strGUID = vscp_string_format( "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                    m_id[0], m_id[1], m_id[2], m_id[3],
                    m_id[4], m_id[5], m_id[6], m_id[7],
                    m_id[8], m_id[9], m_id[10], m_id[11],
                    m_id[12], m_id[13], m_id[14], m_id[15] );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// isSameGUID
//

bool cguid::isSameGUID( const unsigned char *pguid )
{
    if ( NULL == pguid ) return false;
    if ( 0 != memcmp( m_id, pguid, 16 ) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isNULL
//

bool cguid::isNULL( void )
{
    for ( int i=0; i<16; i++ ) {
        if ( m_id[ i ] ) return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeGUID
//

void cguid::writeGUID( uint8_t *pArray )
{
    // Check pointer
    if (NULL == pArray) return;
    
    memcpy( pArray, m_id, 16 );
}

///////////////////////////////////////////////////////////////////////////////
// writeGUID_reverse
//

void cguid::writeGUID_reverse( uint8_t *pArray )
{
    // Check pointer
    if (NULL == pArray) return;
    
    for ( int i=0; i<16; i++ ) {
        pArray[ 15-i ] = m_id[ i ];
    }
}

///////////////////////////////////////////////////////////////////////////////
// setClientID
//

void cguid::setClientID( uint16_t clientid )
{
    m_id[12] = ( clientid >> 8 ) & 0xff;
    m_id[13] = clientid & 0xff;
}

///////////////////////////////////////////////////////////////////////////////
// getClientID
//

uint16_t cguid::getClientID( void ) 
{
    return (uint16_t)( m_id[12] << 8 ) + m_id[13];
}

///////////////////////////////////////////////////////////////////////////////
// setNicknameID
//

void cguid::setNicknameID( uint16_t nicknameid )
{
    m_id[14] = ( nicknameid >> 8 ) & 0xff;
    m_id[15] = nicknameid & 0xff;
}
