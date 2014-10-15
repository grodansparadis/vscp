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
