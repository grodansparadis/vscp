/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanalmsg.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 14:13:52 CEST
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
#pragma implementation "dlgcanalmsg.h"
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

#include "dlgcanalmsg.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalMsg type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgCanalMsg, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalMsg event table definition
//

BEGIN_EVENT_TABLE( dlgCanalMsg, wxDialog )

////@begin dlgCanalMsg event table entries
////@end dlgCanalMsg event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalMsg constructors
//

dlgCanalMsg::dlgCanalMsg()
{
  Init();
}

dlgCanalMsg::dlgCanalMsg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalMsg creator
//

bool dlgCanalMsg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgCanalMsg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgCanalMsg creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalMsg destructor
//

dlgCanalMsg::~dlgCanalMsg()
{
////@begin dlgCanalMsg destruction
////@end dlgCanalMsg destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgCanalMsg::Init()
{
////@begin dlgCanalMsg member initialisation
  m_ctrlOptStandardId = NULL;
  m_ctrlOptExtendedId = NULL;
  m_ctrlEditId = NULL;
  m_ctrlChkRtr = NULL;
  m_ctrlEditData1 = NULL;
  m_ctrlEditData2 = NULL;
  m_ctrlEditData3 = NULL;
  m_ctrlEditData4 = NULL;
  m_ctrlEditData5 = NULL;
  m_ctrlEditData6 = NULL;
  m_ctrlEditData7 = NULL;
  m_ctrlEditData8 = NULL;
////@end dlgCanalMsg member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgCanalMsg
//

void dlgCanalMsg::CreateControls()
{    
////@begin dlgCanalMsg content construction
  dlgCanalMsg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_TOP|wxALL, 1);

  itemBoxSizer3->Add(5, 8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  m_ctrlOptStandardId = new wxRadioButton;
  m_ctrlOptStandardId->Create( itemDialog1, ID_CtrlOptStandardId, _("Standard"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlOptStandardId->SetValue(true);
  itemBoxSizer3->Add(m_ctrlOptStandardId, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_ctrlOptExtendedId = new wxRadioButton;
  m_ctrlOptExtendedId->Create( itemDialog1, ID_CtrlOptExtendedId, _("Extended"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlOptExtendedId->SetValue(true);
  itemBoxSizer3->Add(m_ctrlOptExtendedId, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  itemBoxSizer2->Add(10, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText9 = new wxStaticText;
  itemStaticText9->Create( itemDialog1, wxID_STATIC, _("Id"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  m_ctrlEditId = new wxTextCtrl;
  m_ctrlEditId->Create( itemDialog1, ID_CtrlEditId, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT );
  itemBoxSizer8->Add(m_ctrlEditId, 0, wxALIGN_LEFT|wxALL, 1);

  m_ctrlChkRtr = new wxCheckBox;
  m_ctrlChkRtr->Create( itemDialog1, ID_CtrlChkRtr, _("RTR"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlChkRtr->SetValue(false);
  itemBoxSizer8->Add(m_ctrlChkRtr, 0, wxALIGN_LEFT|wxALL, 1);

  itemBoxSizer2->Add(10, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_TOP|wxALL, 1);

  wxStaticText* itemStaticText14 = new wxStaticText;
  itemStaticText14->Create( itemDialog1, wxID_STATIC, _("Data (0..7)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer13->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  m_ctrlEditData1 = new wxTextCtrl;
  m_ctrlEditData1->Create( itemDialog1, ID_CtrlEditData1, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData2 = new wxTextCtrl;
  m_ctrlEditData2->Create( itemDialog1, ID_CtrlEditData2, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData3 = new wxTextCtrl;
  m_ctrlEditData3->Create( itemDialog1, ID_CtrlEditData3, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData4 = new wxTextCtrl;
  m_ctrlEditData4->Create( itemDialog1, ID_CtrlEditData4, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData5 = new wxTextCtrl;
  m_ctrlEditData5->Create( itemDialog1, ID_CtrlEditData5, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData6 = new wxTextCtrl;
  m_ctrlEditData6->Create( itemDialog1, ID_CtrlEditData6, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData7 = new wxTextCtrl;
  m_ctrlEditData7->Create( itemDialog1, ID_CtrlEditData7, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData8 = new wxTextCtrl;
  m_ctrlEditData8->Create( itemDialog1, ID_CtrlEditData8, _T(""), wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT );
  itemBoxSizer15->Add(m_ctrlEditData8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer25, 0, wxALIGN_TOP|wxALL, 5);

  wxButton* itemButton26 = new wxButton;
  itemButton26->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(itemButton26, 0, wxALIGN_RIGHT|wxALL, 1);

  wxButton* itemButton27 = new wxButton;
  itemButton27->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer25->Add(itemButton27, 0, wxALIGN_RIGHT|wxALL, 1);

////@end dlgCanalMsg content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgCanalMsg::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgCanalMsg::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgCanalMsg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgCanalMsg bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgCanalMsg::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgCanalMsg icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgCanalMsg icon retrieval
}
