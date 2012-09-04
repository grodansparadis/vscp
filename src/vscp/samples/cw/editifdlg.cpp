// editifdlg.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
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
// $RCSfile: editifdlg.cpp,v $                                       
// $Date: 2005/03/10 23:01:29 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

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

#include <wx/valgen.h>
#include "cw.h"
#include "msglistwnd.h"
#include "editifdlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(CEditIfDlg, wxDialog)
  //EVT_BUTTON( wxID_OK, CEditIfDlg::OnOK )
  //EVT_BUTTON( wxID_CANCEL, CEditIfDlg::OnCancel )
  EVT_BUTTON( EditIfBtnBrowsePath, CEditIfDlg::OnButtonDriverPath )
 
  // We have to implement this to force closing
  // the dialog when the 'x' system button is pressed
  //EVT_CLOSE( CEditIfDlg::OnCancel)
  END_EVENT_TABLE()


  ///////////////////////////////////////////////////////////////////////////////
  // CEditIfDlg
  //

  CEditIfDlg::CEditIfDlg( wxWindow *parent )
    : wxDialog( parent, 
                  -1, 
                  _("Settings"), 
                  wxDefaultPosition,
		              wxSize( 570, 200 ), 
                  wxDEFAULT_DIALOG_STYLE )
{
  m_pOKButton = new wxButton( this, wxID_OK, _("OK"), wxPoint( 480, 10 ) );
  m_pCancelButton = new wxButton( this, wxID_CANCEL, _("Cancel"), wxPoint( 480, 40 ) );

	
  // * * * Driver Name * * *
  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "Driver Name" ),
			                        wxPoint( 12, 7 ) );

  m_pEditCtrlName = new wxTextCtrl( this, 
				                            EditIfDeviceName, 
				                            _(""), 
				                            wxPoint( 10, 20 ), 
				                            wxSize( 350, 20 ),
				                            0,
				                            wxTextValidator( wxFILTER_NONE, 
						                                          &m_wxStrDriverName ) );	

  // * * * Driver Path * * *

  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "Driver Path" ),
			                        wxPoint( 12, 47 ) );

  m_pEditCtrlPath = new wxTextCtrl( this, 
				                              EditIfDevicePath, 
				                              _(""), 
				                              wxPoint( 10, 60 ), 
				                              wxSize( 350, 20 ),
				                              0,
				                              wxTextValidator( wxFILTER_NONE, &m_wxStrDriverPath ) );

  m_pButtonBrowsePath = new wxButton( this, 
				                                EditIfBtnBrowsePath, 
				                                _("..."), 
				                                wxPoint( 360, 60 ),
				                                wxSize( 20, 20 ) );


  // * * * Device String * * *
  ( void ) new wxStaticText( this, 
			     -1, 
			     _T( "Device Configuration String" ),
			     wxPoint( 12, 87 ) );

  m_pEditCtrlDeviceString = new wxTextCtrl( this, 
					                                    EditIfDeviceString, 
					                                    _(""), 
					                                    wxPoint( 10, 100 ), 
					                                    wxSize( 350, 20 ),
					                                    0,
					                                    wxTextValidator( wxFILTER_NONE, 
							                                &m_wxStrDriverDeviceString ) );

  // * * * Device Flags * * *
  ( void ) new wxStaticText( this, 
			                          -1, 
			                          _( "Device Flags" ),
			                          wxPoint( 12, 127 ) );

  m_pEditCtrlFlags = new wxTextCtrl( this, 
				                              EditIfDeviceFlags, 
				                              _(""), 
				                              wxPoint( 10, 140 ), 
				                              wxSize( 70, 20 ),
				                              0,
				                              wxTextValidator( wxFILTER_NUMERIC, 
						                          &m_wxStrDriverFlags ) );

  // * * * Filter * * *
  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "Device Filter" ),
			                        wxPoint( 118, 127 ) );

  m_pEditCtrlFilter = new wxTextCtrl( this, 
				                                EditIfDeviceFilter, 
				                                _(""), 
				                                wxPoint( 120, 140 ), 
				                                wxSize( 70, 20 ),
				                                0,
				                                wxTextValidator( wxFILTER_NUMERIC, 
						                            &m_wxStrDriverFilter ) );

  // * * * Mask * * *
  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "Device Mask" ),
			                        wxPoint( 202, 127 ) );

  m_pEditCtrlMask = new wxTextCtrl( this, 
				                            EditIfDeviceMask, 
				                            _(""), 
				                            wxPoint( 200, 140 ), 
				                            wxSize( 70, 20 ),
				                            0,
				                            wxTextValidator( wxFILTER_NUMERIC, 
						                        &m_wxStrDriverMask ) );



  // * * * In buffer size * * *
  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "In Buf. Size" ),
			                        wxPoint( 298, 127 ) );

  m_pEditCtrlInBufSize = new wxTextCtrl( this, 
					                                EditIfDeviceInBufSize, 
					                                _(""), 
					                                wxPoint( 300, 140 ), 
					                                wxSize( 70, 20 ),
					                                0,
					                                wxTextValidator( wxFILTER_NUMERIC, 
							                            &m_wxStrDriverInBufSize ) );

  // * * * Out buffer size * * *
  ( void ) new wxStaticText( this, 
			                        -1, 
			                        _( "Out Buf. Size" ),
			                        wxPoint( 378, 127 ) );

  m_pEditCtrlOutBufSize = new wxTextCtrl( this, 
					                                EditIfDeviceOutBufSize, 
					                                _(""), 
					                                wxPoint( 380, 140 ), 
					                                wxSize( 70, 20 ),
					                                0,
					                                wxTextValidator( wxFILTER_NUMERIC, 
							                            &m_wxStrDriverOutBufSize ) );

}

///////////////////////////////////////////////////////////////////////////////
// ~CEditIfDlg
//

CEditIfDlg::~CEditIfDlg()
{

}

///////////////////////////////////////////////////////////////////////////////
// OnSave
//

void CEditIfDlg::OnOK( wxCommandEvent& event )
{
  Validate();
  TransferDataFromWindow();
  Destroy();
}

///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void CEditIfDlg::OnCancel( wxCommandEvent& event )
{
  // --> Don't use Close with a wxDialog,
  // use Destroy instead.
  Destroy();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonDriverPath
//

void CEditIfDlg::OnButtonDriverPath( wxCommandEvent& event )
{
  wxFileDialog *dlg = new wxFileDialog( this,
					                              _("Path to driver"),
					                              _("c:\\"),
					                              _(""),
					                              _("*.dll") );
  if ( wxID_OK == dlg->ShowModal() ) {
    wxString wxstr = dlg->GetPath(); 
    m_pEditCtrlPath->SetValue( wxstr );		
  }

  dlg->Destroy();

}


