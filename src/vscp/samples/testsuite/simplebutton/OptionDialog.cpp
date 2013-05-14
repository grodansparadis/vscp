// OptionDialog.cpp: implementation of the COptionDialog class.
//
//////////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2011 Ake Hedman akhe@eurosource.se 
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

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/colordlg.h"
#include "wx/filedlg.h"
#include "wx/dirdlg.h"
#include "wx/fontdlg.h"
#include "wx/choicdlg.h"
#include "wx/tipdlg.h"
#include "wx/progdlg.h"
#include "wx/fdrepdlg.h"
#include "wx/busyinfo.h"

#define wxTEST_GENERIC_DIALOGS_IN_MSW 0

#if defined(__WXMSW__) && wxTEST_GENERIC_DIALOGS_IN_MSW
#include "wx/generic/colrdlgg.h"
#include "wx/generic/fontdlgg.h"
#endif

#define wxUSE_DIRDLGG 0

#if !defined(__WXMSW__) || defined(wxUSE_DIRDLGG) && wxUSE_DIRDLGG
#include "wx/generic/dirdlgg.h"
#endif

#include <wx/spinctrl.h>
#include <wx/valgen.h>

#include "OptionDialog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(COptionDialog, wxDialog)
// We have to implement this to force closing
// the dialog when the 'x' system button is pressed
EVT_SPINCTRL( EditLsbAddress, OnAddressChange )
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
// COptionDialog
//

COptionDialog::COptionDialog( wxWindow *parent )
	: wxDialog( parent, -1, _("Settings"), wxDefaultPosition,
				wxSize( 500, 300 ), wxDEFAULT_DIALOG_STYLE )
{
	
	m_pOKButton = new wxButton( this, wxID_OK, _("OK"), wxPoint( 255, 225 ) );
	m_pCancelButton = new wxButton( this, wxID_CANCEL, _("Cancel"), wxPoint( 335, 225 ) );
	
	// * * * Class * * *
	( void ) new wxStaticText( this, 
								-1, 
								_( "Class" ),
								wxPoint( 12, 7 ) );

	m_pSpinCtrlClass = new wxSpinCtrl( this, 
											EditClass, 
											wxEmptyString, 
											wxPoint( 10, 24 ), 
											wxSize( 50, 20 ),
											wxSP_ARROW_KEYS,
											0,
											255,
											0 ); 

	// * * * Type * * *
	( void ) new wxStaticText( this, 
								-1, 
								_T( "Type" ),
								wxPoint( 82, 7 ) );

	m_pSpinCtrlType = new wxSpinCtrl( this, 
										EditType, 
										wxEmptyString, 
										wxPoint( 80, 24 ), 
										wxSize( 50, 20 ),
										wxSP_ARROW_KEYS,
										0,
										255,
										0 );

	// * * * Address * * *
	( void ) new wxStaticText( this, 
								-1, 
								_T( "LSB of VSCP address" ),
								wxPoint( 152, 7 ) );

	m_pSpinCtrlLsbAddress = new wxSpinCtrl( this, 
											EditLsbAddress, 
											wxEmptyString, 
											wxPoint( 150, 24 ), 
											wxSize( 50, 20 ),
											wxSP_ARROW_KEYS,
											0,
											255,
											0 );

	// * * * Data * * *

	( void ) new wxStaticText( this, 
								-1, 
								_T( "Data :" ),
								wxPoint( 40, 70 ) );


	 m_pSpinCtrlData[ 0 ] = new wxSpinCtrl( this, 
												EditData0, 
												wxEmptyString, 
												wxPoint( 80, 70 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 1 ] = new wxSpinCtrl( this, 
												EditData1, 
												wxEmptyString, 
												wxPoint( 80, 92 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 2 ] = new wxSpinCtrl( this, 
												EditData2, 
												wxEmptyString, 
												wxPoint( 80, 114 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 3 ] = new wxSpinCtrl( this, 
												EditData3, 
												wxEmptyString, 
												wxPoint( 80, 136 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 4 ] = new wxSpinCtrl( this, 
												EditData4, 
												wxEmptyString, 
												wxPoint( 80, 158 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 5 ] = new wxSpinCtrl( this, 
												EditData5, 
												wxEmptyString, 
												wxPoint( 80, 180 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 6 ] = new wxSpinCtrl( this, 
												EditData6, 
												wxEmptyString, 
												wxPoint( 80, 202 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 m_pSpinCtrlData[ 7 ] = new wxSpinCtrl( this, 
												EditData7, 
												wxEmptyString, 
												wxPoint( 80, 224 ), 
												wxSize( 50, 20 ),
												wxSP_ARROW_KEYS,
												0,
												255,
												0 );

	 // * * * Size * * *

	( void ) new wxStaticText( this, 
								-1, 
								_T( "Number of databytes" ),
								wxPoint( 150, 70 ) );

	m_pSpinCtrlDataSize = new wxSpinCtrl( this, 
											EditDataSize, 
											wxEmptyString, 
											wxPoint( 150, 90 ), 
											wxSize( 50, 20 ),
											wxSP_ARROW_KEYS,
											0,
											8,
											2 );

}

///////////////////////////////////////////////////////////////////////////////
// ~COptionDialog
//

COptionDialog::~COptionDialog()
{

}

///////////////////////////////////////////////////////////////////////////////
// OnSave
//

void COptionDialog::OnOK( wxCommandEvent& event )
{
	Validate();
	TransferDataFromWindow();
	Destroy();
}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void COptionDialog::OnCancel( wxCommandEvent& event )
{
   // --> Don't use Close with a wxDialog,
   // use Destroy instead.
   Destroy();
}


///////////////////////////////////////////////////////////////////////////////
// OnAddressChange
//

void COptionDialog::OnAddressChange( wxSpinEvent& event )
{
	m_vscpaddr[ 0 ] = m_pSpinCtrlLsbAddress->GetValue();
	printVSCPAddress();   
}

///////////////////////////////////////////////////////////////////////////////
// printVSCPAddress
//

void COptionDialog::printVSCPAddress()
{
	wxChar buf[ 128 ];


	// * * * FULL VSCP Address * * *
	( void ) new wxStaticText( this, 
								-1, 
								_( "Full VSCP address (MSB->LSB)" ),
								wxPoint( 282, 7 ) );

	sprintf( buf,
				_(" %02X %02X %02X %02X %02X %02X %02X %02X"),
				m_vscpaddr[ 15 ],
				m_vscpaddr[ 14 ],
				m_vscpaddr[ 13 ],
				m_vscpaddr[ 12 ],
				m_vscpaddr[ 11 ],
				m_vscpaddr[ 10 ],
				m_vscpaddr[ 9 ],
				m_vscpaddr[ 8 ] );
	( void ) new wxStaticText( this, 
								-1, 
								buf,
								wxPoint( 280, 27 ) );

	sprintf( buf, _(" %02X %02X %02X %02X %02X %02X %02X %02X"),
				m_vscpaddr[ 7 ],
				m_vscpaddr[ 6 ],
				m_vscpaddr[ 5 ],
				m_vscpaddr[ 4 ],
				m_vscpaddr[ 3 ],
				m_vscpaddr[ 2 ],
				m_vscpaddr[ 1 ],
				m_vscpaddr[ 0 ] );
	( void ) new wxStaticText( this, 
								-1, 
								buf,
								wxPoint( 280, 47 ) ); 
	
}