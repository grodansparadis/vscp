/////////////////////////////////////////////////////////////////////////////
// Name:        dlgifsettings.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:48:03
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
#pragma implementation "dlgifsettings.h"
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

#include "dlgifsettings.h"

////@begin XPM images
////@end XPM images

/*!
 * dlgInterfaceSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgInterfaceSettings, wxDialog )

/*!
 * dlgInterfaceSettings event table definition
 */

BEGIN_EVENT_TABLE( dlgInterfaceSettings, wxDialog )

////@begin dlgInterfaceSettings event table entries
  EVT_INIT_DIALOG( dlgInterfaceSettings::OnInitDialog )
  EVT_CLOSE( dlgInterfaceSettings::OnCloseWindow )

////@end dlgInterfaceSettings event table entries

END_EVENT_TABLE()

/*!
 * dlgInterfaceSettings constructors
 */

dlgInterfaceSettings::dlgInterfaceSettings()
{
    Init();
}

dlgInterfaceSettings::dlgInterfaceSettings( wxWindow* parent, 
                                                wxWindowID id, 
                                                const wxString& caption, 
                                                const wxPoint& pos, 
                                                const wxSize& size, 
                                                long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * DlgInterfaceSettings creator
 */

bool dlgInterfaceSettings::Create( wxWindow* parent, 
                                        wxWindowID id, 
                                        const wxString& caption, 
                                        const wxPoint& pos, 
                                        const wxSize& size, 
                                        long style )
{
////@begin dlgInterfaceSettings creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgInterfaceSettings creation
    return true;
}

/*!
 * dlgInterfaceSettings destructor
 */

dlgInterfaceSettings::~dlgInterfaceSettings()
{
////@begin dlgInterfaceSettings destruction
////@end dlgInterfaceSettings destruction
}

/*!
 * Member initialisation 
 */

void dlgInterfaceSettings::Init()
{
////@begin dlgInterfaceSettings member initialisation
////@end dlgInterfaceSettings member initialisation
}
/*!
 * Control creation for DlgInterfaceSettings
 */

void dlgInterfaceSettings::CreateControls()
{    
////@begin dlgInterfaceSettings content construction
  dlgInterfaceSettings* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText4 = new wxStaticText;
  itemStaticText4->Create( itemDialog1, wxID_STATIC, _("Device name"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl5 = new wxTextCtrl;
  itemTextCtrl5->Create( itemDialog1, ID_TEXTCTRL19, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
  if (ShowToolTips())
    itemTextCtrl5->SetToolTip(_("Select a descriptive name for the iterface"));
  itemBoxSizer3->Add(itemTextCtrl5, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer6, 0, wxGROW|wxALL, 1);

  wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Device path"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl9 = new wxTextCtrl;
  itemTextCtrl9->Create( itemDialog1, ID_TEXTCTRL20, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
  if (ShowToolTips())
    itemTextCtrl9->SetToolTip(_("Enter the path to the CANAL driver"));
  itemBoxSizer7->Add(itemTextCtrl9, 0, wxGROW|wxALL, 5);

  wxButton* itemButton10 = new wxButton;
  itemButton10->Create( itemDialog1, ID_BUTTON11, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer6->Add(itemButton10, 0, wxALIGN_BOTTOM|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemDialog1, wxID_STATIC, _("Device configuration string"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl12 = new wxTextCtrl;
  itemTextCtrl12->Create( itemDialog1, ID_TEXTCTRL21, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  if (ShowToolTips())
    itemTextCtrl12->SetToolTip(_("Enter a configuration string. Usually a ; seperated list"));
  itemBoxSizer3->Add(itemTextCtrl12, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText13 = new wxStaticText;
  itemStaticText13->Create( itemDialog1, wxID_STATIC, _("Device flags"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl14 = new wxTextCtrl;
  itemTextCtrl14->Create( itemDialog1, ID_TEXTCTRL22, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  if (ShowToolTips())
    itemTextCtrl14->SetToolTip(_("Enter numerical flag values"));
  itemBoxSizer3->Add(itemTextCtrl14, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer15, 0, wxGROW|wxALL, 1);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer15->Add(itemBoxSizer16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer16->Add(itemBoxSizer17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Device filter"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl19 = new wxTextCtrl;
  itemTextCtrl19->Create( itemDialog1, ID_TEXTCTRL_FILTER, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer17->Add(itemTextCtrl19, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton20 = new wxButton;
  itemButton20->Create( itemDialog1, ID_BUTTON_DEVICE_FILTER, _("..."), wxDefaultPosition, wxSize(20, -1), 0 );
  itemBoxSizer16->Add(itemButton20, 0, wxALIGN_BOTTOM|wxALL, 5);

  wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer15->Add(itemBoxSizer21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer21->Add(itemBoxSizer22, 0, wxALIGN_BOTTOM|wxALL, 0);

  wxStaticText* itemStaticText23 = new wxStaticText;
  itemStaticText23->Create( itemDialog1, wxID_STATIC, _("Device filter"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 0);

  wxTextCtrl* itemTextCtrl24 = new wxTextCtrl;
  itemTextCtrl24->Create( itemDialog1, ID_TEXTCTRL_MASK, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemTextCtrl24, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton25 = new wxButton;
  itemButton25->Create( itemDialog1, ID_BUTTON_DEVICE_MASK, _("..."), wxDefaultPosition, wxSize(20, -1), 0 );
  itemBoxSizer21->Add(itemButton25, 0, wxALIGN_BOTTOM|wxALL, 5);

  wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer26, 0, wxALIGN_TOP|wxALL, 5);

  itemBoxSizer26->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer28, 0, wxALIGN_TOP|wxALL, 5);

  wxButton* itemButton29 = new wxButton;
  itemButton29->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer28->Add(itemButton29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton30 = new wxButton;
  itemButton30->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer28->Add(itemButton30, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end dlgInterfaceSettings content construction
}

/*!
 * wxEVT_INIT_DIALOG event handler for ID_DIALOG_INTERFACE_SETTINGS
 */

void dlgInterfaceSettings::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_INTERFACE_SETTINGS in DlgInterfaceSettings.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_INTERFACE_SETTINGS in DlgInterfaceSettings. 
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_INTERFACE_SETTINGS
 */

void dlgInterfaceSettings::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_INTERFACE_SETTINGS in DlgInterfaceSettings.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_INTERFACE_SETTINGS in DlgInterfaceSettings. 
}

/*!
 * Should we show tooltips?
 */

bool dlgInterfaceSettings::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgInterfaceSettings::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgInterfaceSettings bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgInterfaceSettings bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgInterfaceSettings::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgInterfaceSettings icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgInterfaceSettings icon retrieval
}
