/////////////////////////////////////////////////////////////////////////////
// Name:        dialogabstractionedit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     26/05/2009 20:48:56
// RCS-ID:      
// Copyright:   (C) 2007-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
//  D of Scandinavia at info@dofscandinavia.com, http://www.dofscandinavia.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dialogabstractionedit.h"
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

#include "dialogabstractionedit.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogAbstractionEdit type definition
//

IMPLEMENT_DYNAMIC_CLASS( DialogAbstractionEdit, wxDialog )


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DialogAbstractionEdit event table definition
	//

	BEGIN_EVENT_TABLE( DialogAbstractionEdit, wxDialog )

	////@begin DialogAbstractionEdit event table entries
  EVT_BUTTON( ID_BUTTON18, DialogAbstractionEdit::OnButtonDefaultClick )

	////@end DialogAbstractionEdit event table entries

	END_EVENT_TABLE()


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DialogAbstractionEdit constructors
	//

	DialogAbstractionEdit::DialogAbstractionEdit()
{
	Init();
}

DialogAbstractionEdit::DialogAbstractionEdit( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	Init();
	Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogAbstractionEdit creator
//

bool DialogAbstractionEdit::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	////@begin DialogAbstractionEdit creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
	////@end DialogAbstractionEdit creation
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogAbstractionEdit destructor
//

DialogAbstractionEdit::~DialogAbstractionEdit()
{
	////@begin DialogAbstractionEdit destruction
	////@end DialogAbstractionEdit destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DialogAbstractionEdit::Init()
{
	////@begin DialogAbstractionEdit member initialisation
  m_nType = 0;
  m_abstractionName = NULL;
  m_abstractionId = NULL;
  m_abstractionType = NULL;
  m_abstractionWidth = NULL;
  m_abstractionRegisterPage = NULL;
  m_abstractionRegisterOffset = NULL;
  m_abstractionDescription = NULL;
  m_abstractionHelp = NULL;
  m_abstractionAccessRights = NULL;
  m_abstractionDefaultValue = NULL;
  m_singleValueLabel = NULL;
  m_abstractionValue = NULL;
  m_multipleValueLabel = NULL;
  m_abstractionComboValue = NULL;
	////@end DialogAbstractionEdit member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DialogAbstractionEdit
//

void DialogAbstractionEdit::CreateControls()
{    
	////@begin DialogAbstractionEdit content construction
  DialogAbstractionEdit* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText4 = new wxStaticText;
  itemStaticText4->Create( itemDialog1, wxID_STATIC, _("Abstraction item"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText4->SetForegroundColour(wxColour(0, 128, 255));
  itemStaticText4->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText6 = new wxStaticText;
  itemStaticText6->Create( itemDialog1, wxID_STATIC, _("Name :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText6->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionName = new wxStaticText;
  m_abstractionName->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer5->Add(m_abstractionName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer5->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( itemDialog1, wxID_STATIC, _("Id :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText10->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionId = new wxStaticText;
  m_abstractionId->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer9->Add(m_abstractionId, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer9->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText14 = new wxStaticText;
  itemStaticText14->Create( itemDialog1, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText14->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionType = new wxStaticText;
  m_abstractionType->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer13->Add(m_abstractionType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer13->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer17, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Width :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText18->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionWidth = new wxStaticText;
  m_abstractionWidth->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer17->Add(m_abstractionWidth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer17->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer21, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText22 = new wxStaticText;
  itemStaticText22->Create( itemDialog1, wxID_STATIC, _("Register page :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText22->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer21->Add(itemStaticText22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionRegisterPage = new wxStaticText;
  m_abstractionRegisterPage->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer21->Add(m_abstractionRegisterPage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer21->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer25, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText26 = new wxStaticText;
  itemStaticText26->Create( itemDialog1, wxID_STATIC, _("Register Offset :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText26->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer25->Add(itemStaticText26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionRegisterOffset = new wxStaticText;
  m_abstractionRegisterOffset->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer25->Add(m_abstractionRegisterOffset, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer25->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer29, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText30 = new wxStaticText;
  itemStaticText30->Create( itemDialog1, wxID_STATIC, _("Description :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText30->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer29->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionDescription = new wxStaticText;
  m_abstractionDescription->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  m_abstractionDescription->Wrap(200);
  itemBoxSizer29->Add(m_abstractionDescription, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer29->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer33, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText34 = new wxStaticText;
  itemStaticText34->Create( itemDialog1, wxID_STATIC, _("Help :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText34->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer33->Add(itemStaticText34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionHelp = new wxStaticText;
  m_abstractionHelp->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer33->Add(m_abstractionHelp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer33->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer37, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText38 = new wxStaticText;
  itemStaticText38->Create( itemDialog1, wxID_STATIC, _("Access rights :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText38->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer37->Add(itemStaticText38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionAccessRights = new wxStaticText;
  m_abstractionAccessRights->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer37->Add(m_abstractionAccessRights, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer37->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer41, 0, wxALIGN_RIGHT|wxALL, 1);

  wxStaticText* itemStaticText42 = new wxStaticText;
  itemStaticText42->Create( itemDialog1, wxID_STATIC, _("Default value :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText42->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer41->Add(itemStaticText42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionDefaultValue = new wxStaticText;
  m_abstractionDefaultValue->Create( itemDialog1, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer41->Add(m_abstractionDefaultValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton44 = new wxButton;
  itemButton44->Create( itemDialog1, ID_BUTTON18, _("*"), wxDefaultPosition, wxSize(30, 15), 0 );
  itemBoxSizer41->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer45, 0, wxALIGN_RIGHT|wxALL, 1);

  m_singleValueLabel = new wxStaticText;
  m_singleValueLabel->Create( itemDialog1, wxID_STATIC, _("Value :"), wxDefaultPosition, wxDefaultSize, 0 );
  m_singleValueLabel->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer45->Add(m_singleValueLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_abstractionValue = new wxTextCtrl;
  m_abstractionValue->Create( itemDialog1, ID_TEXTCTRL_ABSTRACTION_VALUE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer45->Add(m_abstractionValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer45->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer49, 0, wxALIGN_RIGHT|wxALL, 1);

  m_multipleValueLabel = new wxStaticText;
  m_multipleValueLabel->Create( itemDialog1, wxID_STATIC, _("Value :"), wxDefaultPosition, wxDefaultSize, 0 );
  m_multipleValueLabel->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemBoxSizer49->Add(m_multipleValueLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_abstractionComboValueStrings;
  m_abstractionComboValue = new wxChoice;
  m_abstractionComboValue->Create( itemDialog1, ID_CHOICE1, wxDefaultPosition, wxSize(200, -1), m_abstractionComboValueStrings, 0 );
  itemBoxSizer49->Add(m_abstractionComboValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer49->Add(20, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer53, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton54 = new wxButton;
  itemButton54->Create( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer53->Add(itemButton54, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton55 = new wxButton;
  itemButton55->Create( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer53->Add(itemButton55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	////@end DialogAbstractionEdit content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool DialogAbstractionEdit::ShowToolTips()
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap DialogAbstractionEdit::GetBitmapResource( const wxString& name )
{
	// Bitmap retrieval
	////@begin DialogAbstractionEdit bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
	////@end DialogAbstractionEdit bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DialogAbstractionEdit::GetIconResource( const wxString& name )
{
	// Icon retrieval
	////@begin DialogAbstractionEdit icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
	////@end DialogAbstractionEdit icon retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transfers data to the window
//

bool DialogAbstractionEdit::TransferDataToWindow( CMDF_Abstraction *pAbstraction, wxString &strValue )
{
	wxString str;

	if ( NULL != pAbstraction) {

		// Save the abstraction
		m_pAbstraction = pAbstraction;

		// Fill in the data
		m_abstractionName->SetLabel( pAbstraction->m_strName );

		m_abstractionId->SetLabel( pAbstraction->m_strID );

		wxString strType;
		switch ( pAbstraction->m_nType ) {

		case type_string: 
			strType = _("String");
			break;

		case type_bitfield:
			strType = _("Bitfield");
			break;

		case type_boolval:
			strType = _("Boolean");
			break;

		case type_int8_t:
			strType = _("Signed 8-bit integer");
			break;

		case type_uint8_t:
			strType = _("Unsigned 8-bit integer");
			break;

		case type_int16_t:
			strType = _("Signed 16-bit integer");
			break;

		case type_uint16_t:
			strType = _("Unsigned 16-bit integer");
			break;

		case type_int32_t:
			strType = _("Signed 32-bit integer");
			break;

		case type_uint32_t:
			strType = _("Unsigned 32-bit integer");
			break;

		case type_int64_t:
			strType = _("Signed 64-bit integer");
			break;

		case type_uint64_t:
			strType = _("Unsigned 64-bit integer");
			break;

		case type_float:
			strType = _("Float");
			break;

		case type_double:
			strType = _("Double");
			break;

		case type_date:
			strType = _("Date");
			break;

		case type_time:
			strType = _("Time");
			break;

		case type_guid:
			strType = _("GUID");
			break;

		case type_unknown:

		default:
			strType = _("Unknown Type");
			break;
		}


		str.Printf(_("%d"), pAbstraction->m_nType );
		m_abstractionType->SetLabel( str + _(" - ") + strType );

		str.Printf(_("%d"), pAbstraction->m_nWidth );
		m_abstractionWidth->SetLabel( str );

		str.Printf(_("%d"), pAbstraction->m_nPage );
		m_abstractionRegisterPage->SetLabel( str );

		str.Printf(_("%d"), pAbstraction->m_nOffset );
		m_abstractionRegisterOffset->SetLabel( str );

		m_abstractionDescription->SetLabel( pAbstraction->m_strDescription );

		m_abstractionHelp->SetLabel( pAbstraction->m_strHelp );

		m_abstractionDefaultValue->SetLabel( pAbstraction->m_strDefault );

		// Access
		str = _("");
		if ( pAbstraction->m_nAccess & MDF_ACCESS_READ ){
			str = _("r");
		}
		if ( pAbstraction->m_nAccess & MDF_ACCESS_WRITE ){
			str += _("w");
		}
		m_abstractionAccessRights->SetLabel( str );

		// Set value to read only if value is not writable
		if ( wxNOT_FOUND == str.Find(_("w")) ) {
			m_abstractionValue->SetEditable( false );
			//m_abstractionComboValue->SetEditable( false );
		}
		else {
			m_abstractionValue->SetEditable( true );
		}

		if ( pAbstraction->m_list_value.GetCount() ) {

			// Hide the single value static
			m_abstractionValue->Show( false );
			m_singleValueLabel->Show( false );

			// Fill values into combo
			MDF_VALUE_LIST::iterator iter;
			for ( iter = pAbstraction->m_list_value.begin(); 
				iter != pAbstraction->m_list_value.end(); ++iter ) {
					CMDF_ValueListValue *pMdfValue = *iter;
					m_abstractionComboValue->Append( pMdfValue->m_strValue );
					m_abstractionComboValue->SetStringSelection( strValue );
			}

		}
		else {

			// Hide the multiple value combo
			m_abstractionComboValue->Show( false );
			m_multipleValueLabel->Show( false );

			m_abstractionValue->SetLabel( strValue );

		}

	}

	return wxDialog::TransferDataToWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transfers data from the window
//

bool DialogAbstractionEdit::TransferDataFromWindow( wxString &strValue )
{
	strValue = _("");

	if ( NULL != m_pAbstraction ) {

		if ( m_abstractionComboValue->GetCount() ) {
			strValue = m_abstractionComboValue->GetStringSelection();
		}
		else {
			strValue = m_abstractionValue->GetLabel();
		}

	} // Valid pointer

	return wxDialog::TransferDataFromWindow();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonDefaultClick
//

void DialogAbstractionEdit::OnButtonDefaultClick( wxCommandEvent& event )
{
	if ( m_abstractionComboValue->GetCount() ) {
		for ( unsigned int i=0; i<m_pAbstraction->m_list_value.GetCount(); i++ ) {
			if ( wxNOT_FOUND != m_abstractionComboValue->GetString( i )
				.Find( m_abstractionDefaultValue->GetLabel() ) ) {
					m_abstractionComboValue->SetSelection( i );
			}
		}
	}
	else {
		m_abstractionValue->SetLabel( m_abstractionDefaultValue->GetLabel() );
	}

	event.Skip();
}

