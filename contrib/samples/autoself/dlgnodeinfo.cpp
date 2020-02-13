/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnodeinfo.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 12:52:33 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource <ake@eurosource.se>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (https://www.vscp.org) 
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
#pragma implementation "dlgnodeinfo.h"
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

#include "dlgnodeinfo.h"

////@begin XPM images
////@end XPM images

/*!
 * dlgNodeInfo type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgNodeInfo, wxDialog )

/*!
 * dlgNodeInfo event table definition
 */

BEGIN_EVENT_TABLE( dlgNodeInfo, wxDialog )

////@begin dlgNodeInfo event table entries
  EVT_BUTTON( wxID_OK, dlgNodeInfo::OnOkClick )

////@end dlgNodeInfo event table entries

END_EVENT_TABLE()

/*!
 * dlgNodeInfo constructors
 */

dlgNodeInfo::dlgNodeInfo()
{
    Init();
}

dlgNodeInfo::dlgNodeInfo( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * dlgNodeInfo creator
 */

bool dlgNodeInfo::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgNodeInfo creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgNodeInfo creation
    return true;
}

/*!
 * dlgNodeInfo destructor
 */

dlgNodeInfo::~dlgNodeInfo()
{
////@begin dlgNodeInfo destruction
////@end dlgNodeInfo destruction
}

/*!
 * Member initialisation 
 */

void dlgNodeInfo::Init()
{
////@begin dlgNodeInfo member initialisation
  m_ctrlEditGUID15 = NULL;
  m_ctrlEditGUID14 = NULL;
  m_ctrlEditGUID13 = NULL;
  m_ctrlEditGUID12 = NULL;
  m_ctrlEditGUID11 = NULL;
  m_ctrlEditGUID10 = NULL;
  m_ctrlEditGUID9 = NULL;
  m_ctrlEditGUID8 = NULL;
  m_ctrlEditGUID7 = NULL;
  m_ctrlEditGUID6 = NULL;
  m_ctrlEditGUID5 = NULL;
  m_ctrlEditGUID4 = NULL;
  m_ctrlEditGUID3 = NULL;
  m_ctrlEditGUID2 = NULL;
  m_ctrlEditGUID1 = NULL;
  m_ctrlEditGUID0 = NULL;
  m_ctrlEditRealName = NULL;
  m_ctrlEditHostname = NULL;
  m_ctrlEditZone = NULL;
////@end dlgNodeInfo member initialisation
}

/*!
 * Control creation for dlgNodeInfo
 */

void dlgNodeInfo::CreateControls()
{    
////@begin dlgNodeInfo content construction
  dlgNodeInfo* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 1);

  wxStaticText* itemStaticText5 = new wxStaticText;
  itemStaticText5->Create( itemDialog1, wxID_STATIC, _("GUID"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText5->SetForegroundColour(wxColour(165, 42, 42));
  itemStaticText5->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlEditGUID15 = new wxTextCtrl;
  m_ctrlEditGUID15->Create( itemDialog1, ID_TEXTCTRL_GUID15, _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID15->SetToolTip(_("GUID 15 MSB (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID14 = new wxTextCtrl;
  m_ctrlEditGUID14->Create( itemDialog1, ID_TEXTCTRL_GUID14 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID14->SetToolTip(_("GUID 14 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID13 = new wxTextCtrl;
  m_ctrlEditGUID13->Create( itemDialog1, ID_TEXTCTRL_GUID13 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID13->SetToolTip(_("GUID 13 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID12 = new wxTextCtrl;
  m_ctrlEditGUID12->Create( itemDialog1, ID_TEXTCTRL_GUID12 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID12->SetToolTip(_("GUID 12 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer6->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ctrlEditGUID11 = new wxTextCtrl;
  m_ctrlEditGUID11->Create( itemDialog1, ID_TEXTCTRL_GUID11 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID11->SetToolTip(_("GUID 11 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID10 = new wxTextCtrl;
  m_ctrlEditGUID10->Create( itemDialog1, ID_TEXTCTRL_GUID10 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID10->SetToolTip(_("GUID 10 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID9 = new wxTextCtrl;
  m_ctrlEditGUID9->Create( itemDialog1, ID_TEXTCTRL_GUID9 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID9->SetToolTip(_("GUID 9 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID8 = new wxTextCtrl;
  m_ctrlEditGUID8->Create( itemDialog1, ID_TEXTCTRL_GUID8 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID8->SetToolTip(_("GUID 8 (Nodes unique address )"));
  itemBoxSizer6->Add(m_ctrlEditGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlEditGUID7 = new wxTextCtrl;
  m_ctrlEditGUID7->Create( itemDialog1, ID_TEXTCTRL_GUID7 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID7->SetToolTip(_("GUID 7 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID6 = new wxTextCtrl;
  m_ctrlEditGUID6->Create( itemDialog1, ID_TEXTCTRL_GUID6 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID6->SetToolTip(_("GUID 6 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID5 = new wxTextCtrl;
  m_ctrlEditGUID5->Create( itemDialog1, ID_TEXTCTRL_GUID5 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID5->SetToolTip(_("GUID 5 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID4 = new wxTextCtrl;
  m_ctrlEditGUID4->Create( itemDialog1, ID_TEXTCTRL_GUID4, _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID4->SetToolTip(_("GUID 4  (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer16->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ctrlEditGUID3 = new wxTextCtrl;
  m_ctrlEditGUID3->Create( itemDialog1, ID_TEXTCTRL_GUID3 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID3->SetToolTip(_("GUID 3 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID2 = new wxTextCtrl;
  m_ctrlEditGUID2->Create( itemDialog1, ID_TEXTCTRL_GUID2 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID2->SetToolTip(_("GUID 2 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID1 = new wxTextCtrl;
  m_ctrlEditGUID1->Create( itemDialog1, ID_TEXTCTRL_GUID1 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID1->SetToolTip(_("GUID 1 (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditGUID0 = new wxTextCtrl;
  m_ctrlEditGUID0->Create( itemDialog1, ID_TEXTCTRL_GUID0 , _("0x00"), wxDefaultPosition, wxSize(40, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditGUID0->SetToolTip(_("GUID 0 LSB (Nodes unique address )"));
  itemBoxSizer16->Add(m_ctrlEditGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALL, 1);

  wxStaticText* itemStaticText28 = new wxStaticText;
  itemStaticText28->Create( itemDialog1, wxID_STATIC, _("Realname of node"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText28->SetForegroundColour(wxColour(165, 42, 42));
  itemStaticText28->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemBoxSizer27->Add(itemStaticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer29, 0, wxALIGN_LEFT|wxALL, 1);

  m_ctrlEditRealName = new wxTextCtrl;
  m_ctrlEditRealName->Create( itemDialog1, ID_TEXTCTRL_REALNAME, _("Unknown Node"), wxDefaultPosition, wxSize(350, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditRealName->SetToolTip(_("Relaname for the node"));
  itemBoxSizer29->Add(m_ctrlEditRealName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer31, 0, wxALIGN_LEFT|wxALL, 1);

  wxStaticText* itemStaticText32 = new wxStaticText;
  itemStaticText32->Create( itemDialog1, wxID_STATIC, _("IP address /hostname of node"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText32->SetForegroundColour(wxColour(165, 42, 42));
  itemStaticText32->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemBoxSizer31->Add(itemStaticText32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer33, 0, wxALIGN_LEFT|wxALL, 1);

  m_ctrlEditHostname = new wxTextCtrl;
  m_ctrlEditHostname->Create( itemDialog1, ID_TEXTCTRL_HOSTNAME, _("192.168.1.1"), wxDefaultPosition, wxSize(350, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditHostname->SetToolTip(_("hostname IP-address for node."));
  itemBoxSizer33->Add(m_ctrlEditHostname, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer35, 0, wxALIGN_LEFT|wxALL, 1);

  wxStaticText* itemStaticText36 = new wxStaticText;
  itemStaticText36->Create( itemDialog1, wxID_STATIC, _("Zone of node"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText36->SetForegroundColour(wxColour(165, 42, 42));
  itemStaticText36->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemBoxSizer35->Add(itemStaticText36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer37, 0, wxALIGN_LEFT|wxALL, 1);

  m_ctrlEditZone = new wxTextCtrl;
  m_ctrlEditZone->Create( itemDialog1, ID_TEXTCTRL_ZONE, _("0x00"), wxDefaultPosition, wxSize(50, -1), 0 );
  if (ShowToolTips())
    m_ctrlEditZone->SetToolTip(_("The Zone  for the node"));
  itemBoxSizer37->Add(m_ctrlEditZone, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(30, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer40 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer40, 0, wxALIGN_TOP|wxALL, 0);

  itemBoxSizer40->Add(5, 10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton42 = new wxButton;
  itemButton42->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer40->Add(itemButton42, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton43 = new wxButton;
  itemButton43->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer40->Add(itemButton43, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  itemBoxSizer2->Add(5, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end dlgNodeInfo content construction
}

/*!
 * Should we show tooltips?
 */

bool dlgNodeInfo::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgNodeInfo::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgNodeInfo bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgNodeInfo bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgNodeInfo::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgNodeInfo icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgNodeInfo icon retrieval
}


////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
//

void dlgNodeInfo::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in dlgNodeInfo.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in dlgNodeInfo. 
}

