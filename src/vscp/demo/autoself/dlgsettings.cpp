/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsettings.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Mon Jun 18 14:52:05 2007
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource <ake@eurosource.se>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2009 Ake Hedman, eurosource, <ake@eurosource.se>
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
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
// 
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
// 
// Alternative licenses for VSCP & Friends may be arranged by contacting 
// eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgsettings.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "dlgsettings.h"

////@begin XPM images
////@end XPM images

extern int g_cntNodes;


#include "../../common/canaltcpif.h"


/*!
 * dlgSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgSettings, wxDialog )


/*!
 * dlgSettings event table definition
 */

BEGIN_EVENT_TABLE( dlgSettings, wxDialog )

////@begin dlgSettings event table entries
  EVT_BUTTON( ID_BUTTON_TEST, dlgSettings::OnButtonTestClick )

////@end dlgSettings event table entries

END_EVENT_TABLE()


/*!
 * dlgSettings constructors
 */

dlgSettings::dlgSettings()
{
  Init();
}

dlgSettings::dlgSettings( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * Dialog creator
 */

bool dlgSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgSettings creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgSettings creation
  return true;
}


/*!
 * dlgSettings destructor
 */

dlgSettings::~dlgSettings()
{
////@begin dlgSettings destruction
////@end dlgSettings destruction
}


/*!
 * Member initialisation
 */

void dlgSettings::Init()
{
////@begin dlgSettings member initialisation
  m_editVSCPServer = NULL;
  m_editVSCPPort = NULL;
  m_editVSCPUsername = NULL;
  m_editVSCPPassword = NULL;
  m_bCheckLogFile = NULL;
  m_editPathLogFile = NULL;
////@end dlgSettings member initialisation
}


/*!
 * Control creation for Dialog
 */

void dlgSettings::CreateControls()
{    
////@begin dlgSettings content construction
  dlgSettings* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText5 = new wxStaticText;
  itemStaticText5->Create( itemDialog1, wxID_STATIC, _("VSCP Server Address :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_editVSCPServer = new wxTextCtrl;
  m_editVSCPServer->Create( itemDialog1, ID_TEXTCTRL_SETTINGS_VSCP_SERVER, _("localhost"), wxDefaultPosition, wxSize(150, -1), 0 );
  itemBoxSizer4->Add(m_editVSCPServer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Port :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_editVSCPPort = new wxTextCtrl;
  m_editVSCPPort->Create( itemDialog1, ID_TEXTCTRL_SETTINGS_PORT, _("9598"), wxDefaultPosition, wxSize(150, -1), 0 );
  itemBoxSizer7->Add(m_editVSCPPort, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer10, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemDialog1, wxID_STATIC, _("Username :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_editVSCPUsername = new wxTextCtrl;
  m_editVSCPUsername->Create( itemDialog1, ID_TEXTCTRL_SETTINGS_USERNAME, _("admin"), wxDefaultPosition, wxSize(150, -1), 0 );
  itemBoxSizer10->Add(m_editVSCPUsername, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer13, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText14 = new wxStaticText;
  itemStaticText14->Create( itemDialog1, wxID_STATIC, _("Password :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_editVSCPPassword = new wxTextCtrl;
  m_editVSCPPassword->Create( itemDialog1, ID_TEXTCTRL_PASSWORD_SETTINGS, _("secret"), wxDefaultPosition, wxSize(150, -1), wxTE_PASSWORD );
  itemBoxSizer13->Add(m_editVSCPPassword, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_bCheckLogFile = new wxCheckBox;
  m_bCheckLogFile->Create( itemDialog1, ID_BCheckLogFile, _("Enable Logfile"), wxDefaultPosition, wxDefaultSize, 0 );
  m_bCheckLogFile->SetValue(true);
  itemBoxSizer3->Add(m_bCheckLogFile, 0, wxALIGN_RIGHT|wxALL, 5);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer17, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Path to logfile :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_editPathLogFile = new wxTextCtrl;
  m_editPathLogFile->Create( itemDialog1, ID_EditPathLogFile, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
  itemBoxSizer17->Add(m_editPathLogFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer20, 0, wxALIGN_TOP|wxALL, 5);

  wxButton* itemButton21 = new wxButton;
  itemButton21->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemButton21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxButton* itemButton22 = new wxButton;
  itemButton22->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemButton22, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxButton* itemButton23 = new wxButton;
  itemButton23->Create( itemDialog1, ID_BUTTON_TEST, _("Test..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer20->Add(itemButton23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end dlgSettings content construction
}


/*!
 * Should we show tooltips?
 */

bool dlgSettings::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgSettings::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgSettings bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgSettings bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgSettings::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgSettings icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgSettings icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////
// OnButtonTestClick
//

void dlgSettings::OnButtonTestClick( wxCommandEvent& event )
{
	CanalTcpIf srvif;
	
	wxString strServer = m_editVSCPServer->GetValue();
	wxString strPort = m_editVSCPPort->GetValue();
	wxString strUsername = m_editVSCPUsername->GetValue();
  wxString strPassword = m_editVSCPPassword->GetValue();
	
	wxString strParam;
	strParam = strUsername + _(";");
	strParam += strPassword + _(";");
	strParam += strServer + _(";");
	strParam += strPort;
	
	// Connect to the server
	if ( srvif.doCmdOpen( strParam ) ) {
		srvif.doCmdDataAvailable();	
		wxMessageBox(_("Successfully connected to server, parameters are correct."));
		srvif.doCmdOpen();
	}
	else {
		wxMessageBox(_("Unable to connect to server. Check username/password, address, port (default 9598)"));
	}
	
	event.Skip(); 
}

