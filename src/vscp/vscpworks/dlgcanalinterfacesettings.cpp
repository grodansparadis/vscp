/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanalinterfacesettings.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 20:45:11 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgcanalinterfacesettings.h"
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
#include "wx/bookctrl.h"
////@end includes

#include "dlgcanalinterfacesettings.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalInterfaceSettings type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgCanalInterfaceSettings, wxPropertySheetDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalInterfaceSettings event table definition
//

BEGIN_EVENT_TABLE( dlgCanalInterfaceSettings, wxPropertySheetDialog )

////@begin dlgCanalInterfaceSettings event table entries
////@end dlgCanalInterfaceSettings event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalInterfaceSettings constructors
//

dlgCanalInterfaceSettings::dlgCanalInterfaceSettings()
{
  Init();
}

dlgCanalInterfaceSettings::dlgCanalInterfaceSettings( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalInterfaceSettings creator
//

bool dlgCanalInterfaceSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgCanalInterfaceSettings creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  SetSheetStyle(wxPROPSHEET_DEFAULT);
  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  LayoutDialog();
  Centre();
////@end dlgCanalInterfaceSettings creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalInterfaceSettings destructor
//

dlgCanalInterfaceSettings::~dlgCanalInterfaceSettings()
{
////@begin dlgCanalInterfaceSettings destruction
////@end dlgCanalInterfaceSettings destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgCanalInterfaceSettings::Init()
{
////@begin dlgCanalInterfaceSettings member initialisation
  m_DirectFlags = NULL;
  m_DriverName = NULL;
  m_PathToDriver = NULL;
  m_DeviceConfigurationString = NULL;
  m_Flags = NULL;
  m_idFilter = NULL;
  m_C = NULL;
  m_idMask = NULL;
  m_RemoteServerURL = NULL;
  m_RemoteServerPort = NULL;
  m_RemoteServerUsername = NULL;
  m_RemoteServerPassword = NULL;
////@end dlgCanalInterfaceSettings member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgCanalInterfaceSettings
//

void dlgCanalInterfaceSettings::CreateControls()
{    
////@begin dlgCanalInterfaceSettings content construction
  dlgCanalInterfaceSettings* itemPropertySheetDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel;
  itemPanel2->Create( GetBookCtrl(), ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
  wxStaticText* itemStaticText6 = new wxStaticText;
  itemStaticText6->Create( itemPanel2, wxID_STATIC, _("Flags:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DirectFlags = new wxTextCtrl;
  m_DirectFlags->Create( itemPanel2, ID_DirectFlags, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT );
  itemBoxSizer5->Add(m_DirectFlags, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer5->Add(300, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  GetBookCtrl()->AddPage(itemPanel2, _("Direct"));

  wxPanel* itemPanel9 = new wxPanel;
  itemPanel9->Create( GetBookCtrl(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
  itemPanel9->SetSizer(itemBoxSizer10);

  wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer11, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText12 = new wxStaticText;
  itemStaticText12->Create( itemPanel9, wxID_STATIC, _("Name of driver :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DriverName = new wxTextCtrl;
  m_DriverName->Create( itemPanel9, ID_DriverName, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer11->Add(m_DriverName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer11->Add(50, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer15, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemPanel9, wxID_STATIC, _("Path to driver :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_PathToDriver = new wxTextCtrl;
  m_PathToDriver->Create( itemPanel9, ID_PathToDriver, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer15->Add(m_PathToDriver, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton19 = new wxButton;
  itemButton19->Create( itemPanel9, ID_BUTTON_CANAL_DRIVER_PATH, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer15->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer15->Add(20, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer21, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText22 = new wxStaticText;
  itemStaticText22->Create( itemPanel9, wxID_STATIC, _("Device configuration string :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer21->Add(itemStaticText22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DeviceConfigurationString = new wxTextCtrl;
  m_DeviceConfigurationString->Create( itemPanel9, ID_DeviceConfigurationString, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer21->Add(m_DeviceConfigurationString, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton24 = new wxButton;
  itemButton24->Create( itemPanel9, ID_BUTTON_CANAL_CONFIGURATION, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer21->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer21->Add(20, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer26, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText27 = new wxStaticText;
  itemStaticText27->Create( itemPanel9, wxID_STATIC, _("Flags :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer26->Add(itemStaticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_Flags = new wxTextCtrl;
  m_Flags->Create( itemPanel9, ID_Flags, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
  itemBoxSizer26->Add(m_Flags, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer26->Add(302, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer30, 0, wxALIGN_RIGHT|wxALL, 1);
  m_idFilter = new wxStaticText;
  m_idFilter->Create( itemPanel9, wxID_STATIC, _("ID Filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(m_idFilter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_C = new wxTextCtrl;
  m_C->Create( itemPanel9, ID_C, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
  itemBoxSizer30->Add(m_C, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText33 = new wxStaticText;
  itemStaticText33->Create( itemPanel9, wxID_STATIC, _("ID Mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemStaticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_idMask = new wxTextCtrl;
  m_idMask->Create( itemPanel9, ID_IdMask, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
  itemBoxSizer30->Add(m_idMask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton35 = new wxButton;
  itemButton35->Create( itemPanel9, ID_BUTTON4, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer30->Add(itemButton35, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer30->Add(158, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer10->Add(itemBoxSizer37, 0, wxALIGN_RIGHT|wxALL, 1);
  wxButton* itemButton38 = new wxButton;
  itemButton38->Create( itemPanel9, ID_BUTTON_CANAL_DRIVER_WIZARD, _("Wizard..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer37->Add(itemButton38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer37->Add(270, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  GetBookCtrl()->AddPage(itemPanel9, _("Driver"));

  wxPanel* itemPanel40 = new wxPanel;
  itemPanel40->Create( GetBookCtrl(), ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxVERTICAL);
  itemPanel40->SetSizer(itemBoxSizer41);

  wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer41->Add(itemBoxSizer42, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer41->Add(itemBoxSizer43, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText44 = new wxStaticText;
  itemStaticText44->Create( itemPanel40, wxID_STATIC, _("Server URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer43->Add(itemStaticText44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerURL = new wxTextCtrl;
  m_RemoteServerURL->Create( itemPanel40, ID_RemoteServerURL, _T(""), wxDefaultPosition, wxSize(400, -1), wxTE_RIGHT );
  itemBoxSizer43->Add(m_RemoteServerURL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer46 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer41->Add(itemBoxSizer46, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText47 = new wxStaticText;
  itemStaticText47->Create( itemPanel40, wxID_STATIC, _("Server port:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer46->Add(itemStaticText47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerPort = new wxTextCtrl;
  m_RemoteServerPort->Create( itemPanel40, ID_RemoteServerPort, _T(""), wxDefaultPosition, wxSize(50, -1), wxTE_RIGHT );
  itemBoxSizer46->Add(m_RemoteServerPort, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer46->Add(348, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer41->Add(itemBoxSizer50, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText51 = new wxStaticText;
  itemStaticText51->Create( itemPanel40, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemStaticText51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerUsername = new wxTextCtrl;
  m_RemoteServerUsername->Create( itemPanel40, ID_RemoteServerUsername, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_RIGHT );
  itemBoxSizer50->Add(m_RemoteServerUsername, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer50->Add(200, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer54 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer41->Add(itemBoxSizer54, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText55 = new wxStaticText;
  itemStaticText55->Create( itemPanel40, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer54->Add(itemStaticText55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerPassword = new wxTextCtrl;
  m_RemoteServerPassword->Create( itemPanel40, ID_RemoteServerPassword, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD|wxTE_RIGHT );
  itemBoxSizer54->Add(m_RemoteServerPassword, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer54->Add(200, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer41->Add(itemBoxSizer58, 0, wxALIGN_RIGHT|wxALL, 5);
  wxButton* itemButton59 = new wxButton;
  itemButton59->Create( itemPanel40, ID_BUTTON1, _("Test connection"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer58->Add(itemButton59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer58->Add(285, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  GetBookCtrl()->AddPage(itemPanel40, _("Remote"));

////@end dlgCanalInterfaceSettings content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgCanalInterfaceSettings::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgCanalInterfaceSettings::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgCanalInterfaceSettings bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgCanalInterfaceSettings bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgCanalInterfaceSettings::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgCanalInterfaceSettings icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgCanalInterfaceSettings icon retrieval
}
