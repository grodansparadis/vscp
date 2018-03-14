// FILE: register.h
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
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmain.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#if wxUSE_GUI!=0
#include <wx/progdlg.h>
#include <wx/imaglist.h>
#endif
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/url.h>
#include <wx/listimpl.cpp>

#include <canal.h>
#include <vscp.h>
#include <vscphelper.h>
#include <mdf.h>
#include <register.h>


///////////////////////////////////////////////////////////////////////////////
//  Constructor
//
/*
CDecisionMatrix::CDecisionMatrix( CMDF_DecisionMatrix *pdm )
{
    m_pdm = NULL;

    // Check if this matrix is indexed
    if ( ( 1 == pdm->m_nLevel ) && ( 120 == pdm->m_nStartOffset ) ) {
        m_bIndexed = true;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
// 

CDecisionMatrix::~CDecisionMatrix( void )
{
    if ( NULL != m_pdm ) delete m_pdm;
}

///////////////////////////////////////////////////////////////////////////////
//  getRow
// 

bool getRow( uint32_t row, uint8_t *pRow )
{
    
}
*/



//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
// 

CStandardRegisters::CStandardRegisters()
{
    memset( m_reg, 0, sizeof( m_reg ) );
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
// 

CStandardRegisters::~CStandardRegisters()
{

}

///////////////////////////////////////////////////////////////////////////////
//  getFirmwareVersionString.
// 

wxString CStandardRegisters::getFirmwareVersionString( void )
{
    wxString str;

    str = str.Format( _("%d.%d.%d"), m_reg[ 0x94 - 0x80 ],
                                    m_reg[ 0x95 - 0x80 ],
                                    m_reg[ 0x96 - 0x80 ] ); 
    return str;
}


///////////////////////////////////////////////////////////////////////////////
//  getMDF
// 

void CStandardRegisters::getMDF( wxString& remoteFile )
{
    char url[33];

    memset( url, 0, sizeof( url ) );
    memcpy( url, ( m_reg + 0xe0 - 0x80 ), 32 );
    remoteFile = _("http://");
    remoteFile += wxString::From8BitData( url );
}

///////////////////////////////////////////////////////////////////////////////
//  getStandardReg
//

uint8_t CStandardRegisters::getStandardReg( uint8_t reg )
{
    if ( reg < 128 ) return 0;

    return m_reg[ reg-0x80 ];
}



//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
// 

CUserRegisters::CUserRegisters()
{
    m_reg = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
// 

CUserRegisters::~CUserRegisters()
{
    if ( NULL != m_reg ) {
        delete [] m_reg;
    }

    m_size = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  init
// 

void CUserRegisters::init( wxArrayLong &pagesArray )
{
    m_arrayPages = pagesArray;

    // Delete possible previous allocation
    if ( NULL != m_reg ) {
        delete [] m_reg;
    }


    m_size = m_arrayPages.Count() * 128; 
    m_reg = new unsigned char [ m_arrayPages.Count() * 128 ];
}

///////////////////////////////////////////////////////////////////////////////
// getRegs4Page
// 

uint8_t *CUserRegisters::getRegs4Page( uint16_t page )
{
    for (uint16_t i=0; i<m_arrayPages.Count(); i++ ) {
        if ( page ==  m_arrayPages[ i ] ) {
            return m_reg + i*128;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

uint8_t CUserRegisters::getValue( uint16_t page, uint8_t offset )
{
    uint8_t *p;

    if ( offset > 127 ) return 0;
    if ( NULL == ( p = getRegs4Page( page ) ) ) return 0;

    return p[offset];
}


///////////////////////////////////////////////////////////////////////////////
// setValue
//

uint8_t CUserRegisters::setValue( uint16_t page, uint8_t offset, uint8_t value )
{
    uint8_t *p;

    if ( offset > 127 ) return 0;
    if ( NULL == ( p = getRegs4Page( page ) ) ) return 0;

    return ( p[offset] = value );
}

///////////////////////////////////////////////////////////////////////////////
//  abstractionValueFromRegsToString
//

bool CUserRegisters::abstractionValueFromRegsToString( CMDF_Abstraction *pAbstraction, 
                                                            wxString &strValue,
                                                            uint8_t format )
{
    bool rv = false;
    uint8_t *pReg;

    if ( NULL == pAbstraction ) return false;

    // Get register page
    if ( NULL == ( pReg = getRegs4Page( pAbstraction->m_nPage ) ) ) return false;

    switch ( pAbstraction->m_nType ) {

    case type_string: 
        {            
            uint8_t *pStr;
            
            pStr = new uint8_t[ pAbstraction->m_nWidth + 1 ];
            if ( NULL == pStr ) return false;
            memset( pStr, 0, pAbstraction->m_nWidth + 1 );                
            memcpy( pStr, pReg + pAbstraction->m_nOffset, pAbstraction->m_nWidth );
            strValue.From8BitData( (const char *)pStr );
            delete [] pStr;
            return true;
        }
        break;

    case type_boolval:
        {
            strValue = (pReg[pAbstraction->m_nOffset] ? _("true") : _("false") );
        }
        break;

    case type_bitfield:
        for ( int i=0; i<pAbstraction->m_nWidth; i++ ) {
            for ( int j=7; j>0; j-- ) {
                if ( *(pReg + pAbstraction->m_nOffset + i) & (1 << j) ) {
                    strValue += _("1");
                }
                else {
                    strValue += _("0");
                }
            }
        }
        break;

    case type_int8_t:
        {
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%d" ), *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue.Printf( _( "0x%02x" ), *( pReg + pAbstraction->m_nOffset ) );
            }
        }
        break;

    case type_uint8_t:
        {
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%ud" ), *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue.Printf( _( "0x%02x" ), *( pReg + pAbstraction->m_nOffset ) );
            }
        }
        break;

    case type_int16_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            int16_t val = ( p[0] << 8 ) + p[1];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%d" ), val );
            }
            else {
                strValue.Printf( _( "%04x" ), val );
                strValue = _("0x") + strValue.Right( 4 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint16_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            uint16_t val = ( p[0] << 8 ) + p[1];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%ud" ), val );
            }
            else {
                strValue.Printf( _( "0x%04x" ), val );
            }
        }
        break;

    case type_int32_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            int32_t val = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%ld" ), *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue.Printf( _( "%08lx" ), *( pReg + pAbstraction->m_nOffset ) );
                strValue = _("0x") + strValue.Right( 8 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint32_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            uint32_t val = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%uld" ), val );
            }
            else {
                strValue.Printf( _( "0x%08lx" ), val );
            }
        }
        break;

    case type_int64_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            *p = wxUINT64_SWAP_ON_LE( *p );

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%lld" ), *p );
            }
            else {
                strValue.Printf( _( "%llx" ), *p );
                strValue = _("0x") + strValue.Right( 8 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint64_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            *p = wxUINT64_SWAP_ON_LE( *p );
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue.Printf( _( "%ulld" ), *p );
            }
            else {
                strValue.Printf( _( "0x%ullx" ), *p );
            }
        }
        break;

    case type_float:
        {
            uint8_t *p = (uint8_t *)(pReg + pAbstraction->m_nOffset ); 
            uint32_t n = wxUINT32_SWAP_ON_LE( *( (uint32_t *)p ) );
            float f = *( (float *)((uint8_t *)&n ) );
            strValue.Printf( _("%f"), f );
        }
        break;

    case type_double:
        {
            uint8_t *p = (uint8_t *)(pReg + pAbstraction->m_nOffset );
            uint64_t n = wxUINT64_SWAP_ON_LE( *( (uint32_t *)p ) );
            double f = *( (double *)((uint8_t *)&n ) );
            strValue.Printf( _("%g"), f );
        }
        break;

    case type_date:
        {
            wxDateTime date;
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            uint8_t year = ( p[ 0 ] << 8 ) + p[ 1 ];
            date.SetYear( year );
            date.SetMonth( wxDateTime::Month( p[ 2 ] ) );
            date.SetDay( p[ 3 ] );
            strValue = date.FormatISODate();
        }
        break;

    case type_time:
        {
            wxDateTime time;
            time.SetHour( *(pReg + pAbstraction->m_nOffset ) );
            time.SetMinute( *(pReg + pAbstraction->m_nOffset + 1 ) );
            time.SetSecond( *(pReg + pAbstraction->m_nOffset + 2 ) );
            strValue = time.FormatISOTime();
        }
        break;

    case type_guid:
        {
            cguid val;
            val.getFromArray( pReg + pAbstraction->m_nOffset );
            val.toString( strValue );
        }
        break;

    case type_unknown:
    default:
        strValue = _("");
        break;
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  abstractionValueFromStringToRegs
//

bool CUserRegisters::abstractionValueFromStringToRegs( CMDF_Abstraction *pAbstraction, 
                                                        wxString &strValue )
{   
    bool rv = false;
    uint8_t *pReg;

    if ( NULL == pAbstraction ) return false;

    // Get register page
    if ( NULL == ( pReg = getRegs4Page( pAbstraction->m_nPage ) ) ) return false;

    switch ( pAbstraction->m_nType ) {

    case type_string: 

        break;

    case type_boolval:
    
        break;

    case type_bitfield:

        break;

    case type_int8_t:

        break;

    case type_uint8_t:

        break;

    case type_int16_t:
  
        break;

    case type_uint16_t:

        break;

    case type_int32_t:

        break;

    case type_uint32_t:

        break;

    case type_int64_t:

        break;

    case type_uint64_t:

        break;

    case type_float:

        break;

    case type_double:

        break;

    case type_date:
 
        break;

    case type_time:

        break;

    case type_guid:

        break;

    case type_unknown:

        break;
    }
    
    return rv;
}


