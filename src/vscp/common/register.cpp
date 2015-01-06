// FILE: register.h
//
// Copyright (C) 2012-2014 Ake Hedman akhe@grodansparadis.com 
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

#include "canal.h"
#include "vscp.h"
#include "vscphelper.h"
#include "mdf.h"
#include "register.h"


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

wxString CStandardRegisters::getMDF( void )
{
	wxString remoteFile;
    char url[33];

    memset( url, 0, sizeof( url ) );
    memcpy( url, ( m_reg + 0xe0 - 0x80 ), 32 );
    remoteFile = _("http://");
	remoteFile += wxString::From8BitData( url );

	return remoteFile;
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
//  getAbstractionValueAsString
//

bool CUserRegisters::getAbstractionValueAsString( CMDF_Abstraction *pAbstraction, wxString &strValue )
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
			strValue.Printf( _("0x%02x"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_uint8_t:
		{
			strValue.Printf( _("0x%02x"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_int16_t:
		{
			strValue.Printf( _("0x%04x"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_uint16_t:
		{
			strValue.Printf( _("0x%04x"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_int32_t:
		{
			strValue.Printf( _("0x%08lx"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_uint32_t:
		{
			strValue.Printf( _("0x%08lx"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_int64_t:
		{
			strValue.Printf( _("0x%llx"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_uint64_t:
		{
			strValue.Printf( _("0x%llx"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_float:
		{
			strValue.Printf( _("%f"), *(pReg + pAbstraction->m_nOffset ) );
		}
		break;

	case type_double:
		{
			strValue.Printf( _("%g"), *(pReg + pAbstraction->m_nOffset ) );
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