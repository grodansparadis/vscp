/////////////////////////////////////////////////////////////////////////////
// Name:        dlgCanalRowEdit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 14:14:50 CEST
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
#pragma implementation "dlgCanalRowEdit.h"
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

#include "dlgCanalRowEdit.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalRowEdit type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgCanalRowEdit, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalRowEdit event table definition
//

BEGIN_EVENT_TABLE( dlgCanalRowEdit, wxDialog )

////@begin dlgCanalRowEdit event table entries
////@end dlgCanalRowEdit event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalRowEdit constructors
//

dlgCanalRowEdit::dlgCanalRowEdit()
{
  Init();
}

dlgCanalRowEdit::dlgCanalRowEdit( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalRowEdit creator
//

bool dlgCanalRowEdit::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgCanalRowEdit creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgCanalRowEdit creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgCanalRowEdit destructor
//

dlgCanalRowEdit::~dlgCanalRowEdit()
{
////@begin dlgCanalRowEdit destruction
////@end dlgCanalRowEdit destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgCanalRowEdit::Init()
{
////@begin dlgCanalRowEdit member initialisation
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
  m_ctrlEditRoeDescription = NULL;
  m_ctrlEditRoeInterval = NULL;
////@end dlgCanalRowEdit member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgCanalRowEdit
//

void dlgCanalRowEdit::CreateControls()
{    
////@begin dlgCanalRowEdit content construction
  dlgCanalRowEdit* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxALL, 5);

  itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlOptStandardId = new wxRadioButton;
  m_ctrlOptStandardId->Create( itemDialog1, ID_RADIOBUTTON_STANDARD_ID, _("Standard"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlOptStandardId->SetValue(true);
  itemBoxSizer4->Add(m_ctrlOptStandardId, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_ctrlOptExtendedId = new wxRadioButton;
  m_ctrlOptExtendedId->Create( itemDialog1, ID_RADIOBUTTON_EXTENDED_ID, _("Extended"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlOptExtendedId->SetValue(true);
  itemBoxSizer4->Add(m_ctrlOptExtendedId, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer9, 0, wxALIGN_TOP|wxALL, 5);

  itemBoxSizer9->Add(5, 1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemDialog1, wxID_STATIC, _("Id"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  m_ctrlEditId = new wxTextCtrl;
  m_ctrlEditId->Create( itemDialog1, ID_TEXTCTRL_ID, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(m_ctrlEditId, 0, wxALIGN_LEFT|wxALL, 1);

  m_ctrlChkRtr = new wxCheckBox;
  m_ctrlChkRtr->Create( itemDialog1, ID_CHECKBOX_RTR, _("RTR"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlChkRtr->SetValue(false);
  itemBoxSizer9->Add(m_ctrlChkRtr, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer15, 0, wxALIGN_TOP|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemDialog1, wxID_STATIC, _("Data"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer15->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlEditData1 = new wxTextCtrl;
  m_ctrlEditData1->Create( itemDialog1, ID_TEXTCTRL_DATA1, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData2 = new wxTextCtrl;
  m_ctrlEditData2->Create( itemDialog1, ID_TEXTCTRL_DATA2, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData3 = new wxTextCtrl;
  m_ctrlEditData3->Create( itemDialog1, ID_TEXTCTRL_DATA3, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData4 = new wxTextCtrl;
  m_ctrlEditData4->Create( itemDialog1, ID_TEXTCTRL_DATA4, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData5 = new wxTextCtrl;
  m_ctrlEditData5->Create( itemDialog1, ID_TEXTCTRL_DATA5, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData6 = new wxTextCtrl;
  m_ctrlEditData6->Create( itemDialog1, ID_TEXTCTRL_DATA6, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData7 = new wxTextCtrl;
  m_ctrlEditData7->Create( itemDialog1, ID_TEXTCTRL_DATA7, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ctrlEditData8 = new wxTextCtrl;
  m_ctrlEditData8->Create( itemDialog1, ID_TEXTCTRL_DATA8, _T(""), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer17->Add(m_ctrlEditData8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton28 = new wxButton;
  itemButton28->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemButton28, 0, wxALIGN_RIGHT|wxALL, 1);

  wxButton* itemButton29 = new wxButton;
  itemButton29->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemButton29, 0, wxALIGN_RIGHT|wxALL, 1);

  wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer31, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText32 = new wxStaticText;
  itemStaticText32->Create( itemDialog1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer31->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  m_ctrlEditRoeDescription = new wxTextCtrl;
  m_ctrlEditRoeDescription->Create( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxSize(400, -1), 0 );
  itemBoxSizer31->Add(m_ctrlEditRoeDescription, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

  wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer34, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText35 = new wxStaticText;
  itemStaticText35->Create( itemDialog1, wxID_STATIC, _("Interval (ms)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer34->Add(itemStaticText35, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 1);

  m_ctrlEditRoeInterval = new wxTextCtrl;
  m_ctrlEditRoeInterval->Create( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer34->Add(m_ctrlEditRoeInterval, 0, wxALIGN_LEFT|wxALL, 3);

////@end dlgCanalRowEdit content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgCanalRowEdit::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgCanalRowEdit::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgCanalRowEdit bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgCanalRowEdit bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgCanalRowEdit::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgCanalRowEdit icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgCanalRowEdit icon retrieval
}
