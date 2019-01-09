// FILE: register.h
//
// This file is part of the VSCP (http://www.vscp.org) 
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


#include <string>
#include <set>
#include <map>

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
    ;
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

std::string CStandardRegisters::getFirmwareVersionString( void )
{
    std::string str;

    str = str = vscp_str_format( "%d.%d.%d", m_reg[ 0x94 - 0x80 ],
                                        m_reg[ 0x95 - 0x80 ],
                                        m_reg[ 0x96 - 0x80 ] ); 
    return str;
}


///////////////////////////////////////////////////////////////////////////////
//  getMDF
// 

void CStandardRegisters::getMDF( std::string& remoteFile )
{
    char url[33];

    memset( url, 0, sizeof( url ) );
    memcpy( url, ( m_reg + 0xe0 - 0x80 ), 32 );
    remoteFile = "http://";
    remoteFile += std::string( url );
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

CRegisterPage::CRegisterPage( uint8_t level )
{
    m_level = level;
}

CRegisterPage::~CRegisterPage()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// readReg
//

int CRegisterPage::readReg( uint32_t reg )
{
    // Check validity of register pointer
    if ( VSCP_LEVEL1 == m_level ) {
        if ( reg > 128 ) return -1;      // Invalid reg offset for Level I device
    }
    else if ( VSCP_LEVEL2 == m_level ) {
        ;
    }
    else {
        // Level is wrong
        return -1;
    }

    return readReg( reg );
}

///////////////////////////////////////////////////////////////////////////////
// writeReg
//

bool CRegisterPage::writeReg( uint32_t reg, uint8_t value )
{
    // Check validity of register pointer
    if ( VSCP_LEVEL1 == m_level ) {
        if ( reg > 128 ) return false;       // Invalid reg offset for Level I device
    }
    else if ( VSCP_LEVEL2 == m_level ) {
        ;
    }
    else {
        // Level is wrong
        return false;
    }

    m_registers[reg] = value;    // Assign value
    return true;
}


//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
// 

CUserRegisters::CUserRegisters( uint8_t level )
{
    m_level = level;
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
// 

CUserRegisters::~CUserRegisters()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// readReg
//

int CUserRegisters::readReg( uint32_t reg, uint32_t page )
{
    // Check validity of register pointer
    if ( VSCP_LEVEL1 == m_level ) {
        if ( page > 128 ) return -1;     // invalid page for Level I device
        if ( reg > 128 ) return -1;      // Invalid reg offset for Level I device
    }
    else if ( VSCP_LEVEL2 == m_level ) {
        if ( page > 0xfffffff0 ) return -1;  // Invalid page for level II device
    }
    else {
        // Level is wrong
        return -1;
    }

    // Check if page exists
    std::map<uint32_t,CRegisterPage *>::iterator it;
    if ( m_registerPages.end() == ( it = m_registerPages.find( page ) ) ) {
        return -1;
    }

    // Get page
    CRegisterPage *pPage = it->second;
    if ( NULL == pPage ) return -1;

    return pPage->readReg( reg ); // Return value
}

///////////////////////////////////////////////////////////////////////////////
// writeReg
//

bool CUserRegisters::writeReg( uint32_t reg, uint32_t page, uint8_t value )
{
    // Check validity of register pointer
    if ( VSCP_LEVEL1 == m_level ) {
        if ( page > 128 ) return false;      // invalid page for Level I device
        if ( reg > 128 ) return false;       // Invalid reg offset for Level I device
    }
    else if ( VSCP_LEVEL2 == m_level ) {
        if ( page > 0xfffffff0 ) return false;  // Invalid page for level II device
    }
    else {
        // Level is wrong
        return false;
    }

    std::map<uint32_t,CRegisterPage *>::iterator it;
    if ( m_registerPages.end() != ( it = m_registerPages.find( page ) ) ) {
        
        // Page already exist

        CRegisterPage * pPage = it->second;
        if ( NULL == pPage ) return false;  // Invalid page

        pPage->writeReg( reg, value );      // Assign value    
 
    }
    else {

        // Page does not exist

        // Create page
        CRegisterPage *pPage  = new CRegisterPage( m_level );
        if ( NULL == pPage ) return false;   // Unable to create page
        m_registerPages[ page ] = pPage;

        pPage->writeReg( reg, value );      // Assign value
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
//  abstractionValueFromRegsToString
//

bool CUserRegisters::abstractionValueFromRegsToString( CMDF_Abstraction *pAbstraction, 
                                                            std::string &strValue,
                                                            uint8_t format )
{
    
    bool rv = false;
    uint8_t *pReg;
/*
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
            strValue = (const char *)pStr;
            delete [] pStr;
            return true;
        }
        break;

    case type_boolval:
        {
            strValue = (pReg[pAbstraction->m_nOffset] ? "true" : "false" );
        }
        break;

    case type_bitfield:
        for ( int i=0; i<pAbstraction->m_nWidth; i++ ) {
            for ( int j=7; j>0; j-- ) {
                if ( *(pReg + pAbstraction->m_nOffset + i) & (1 << j) ) {
                    strValue += "1";
                }
                else {
                    strValue += "0";
                }
            }
        }
        break;

    case type_int8_t:
        {
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_sting_format( "%d", *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue = vscp_str_format( "0x%02x", *( pReg + pAbstraction->m_nOffset ) );
            }
        }
        break;

    case type_uint8_t:
        {
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%ud", *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue = vscp_str_format( "0x%02x" , *( pReg + pAbstraction->m_nOffset ) );
            }
        }
        break;

    case type_int16_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            int16_t val = ( p[0] << 8 ) + p[1];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%d", val );
            }
            else {
                strValue = vscp_str_format( "%04x", val );
                strValue = "0x";
                strValue += strValue.substr( strValue.length() - 4 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint16_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            uint16_t val = ( p[0] << 8 ) + p[1];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%ud", val );
            }
            else {
                strValue = vscp_str_format( "0x%04x", val );
            }
        }
        break;

    case type_int32_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            int32_t val = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%ld", *( pReg + pAbstraction->m_nOffset ) );
            }
            else {
                strValue = vscp_str_format( "%08lx", *( pReg + pAbstraction->m_nOffset ) );
                strValue = "0x";
                strValue += strValue.substr( strValue.length() - 8 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint32_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            uint32_t val = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%uld", val );
            }
            else {
                strValue = vscp_str_format( "0x%08lx", val );
            }
        }
        break;

    case type_int64_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            *p = VSCP_UINT64_SWAP_ON_LE( *p );

            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%lld", *p );
            }
            else {
                strValue = vscp_str_format( "%llx", *p );
                strValue = "0x";
                strValue += strValue.substr( strValue.length() - 8 ); // Handles negative numbers correct
            }
        }
        break;

    case type_uint64_t:
        {
            uint8_t *p = pReg + pAbstraction->m_nOffset;
            *p = wxUINT64_SWAP_ON_LE( *p );
            if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
                strValue = vscp_str_format( "%ulld", *p );
            }
            else {
                strValue = vscp_str_format( "0x%ullx", *p );
            }
        }
        break;

    case type_float:
        {
            uint8_t *p = (uint8_t *)(pReg + pAbstraction->m_nOffset ); 
            uint32_t n = wxUINT32_SWAP_ON_LE( *( (uint32_t *)p ) );
            float f = *( (float *)((uint8_t *)&n ) );
            strValue = vscp_str_format( "%f", f );
        }
        break;

    case type_double:
        {
            uint8_t *p = (uint8_t *)(pReg + pAbstraction->m_nOffset );
            uint64_t n = wxUINT64_SWAP_ON_LE( *( (uint32_t *)p ) );
            double f = *( (double *)((uint8_t *)&n ) );
            strValue = vscp_str_format( "%g"), f );
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
            cguid guid;
            guid.getFromArray( pReg + pAbstraction->m_nOffset );
            guid.toString( strValue );
        }
        break;

    case type_unknown:
    default:
        strValue = "";
        break;
    }
*/
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  abstractionValueFromStringToRegs
//

bool CUserRegisters::abstractionValueFromStringToRegs( CMDF_Abstraction *pAbstraction, 
                                                        std::string &strValue )
{   
    bool rv = false;
    uint8_t *pReg;
/*
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
*/    
    return rv;
}


