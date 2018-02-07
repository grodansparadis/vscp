/////////////////////////////////////////////////////////////////////////////
// Name:        dialogeditlevelidmrow.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     18/03/2009 23:10:00
// RCS-ID:      
// Copyright:   (C) 2007-2018 
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
// 

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgeditlevel1dmrow.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "dlgsetfiltermask.h"
#include "dlgeditlevel1dmrow.h"
#include "dialogeditlevelidmrow_images.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogEditLevelIDMrow type definition
//

IMPLEMENT_DYNAMIC_CLASS( DialogEditLevelIDMrow, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogEditLevelIDMrow event table definition
//

BEGIN_EVENT_TABLE( DialogEditLevelIDMrow, wxDialog )

  EVT_BUTTON( ID_BUTTON_SELECT_EVENTS, DialogEditLevelIDMrow::OnButtonSelectEventsClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogEditLevelIDMrow constructors
//

DialogEditLevelIDMrow::DialogEditLevelIDMrow()
{
    Init();
}

DialogEditLevelIDMrow::DialogEditLevelIDMrow( wxWindow* parent, 
                                                wxWindowID id, 
                                                const wxString& caption, 
                                                const wxPoint& pos, 
                                                const wxSize& size, 
                                                long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogEditLevelIDMrow creator
//

bool DialogEditLevelIDMrow::Create( wxWindow* parent, 
                                        wxWindowID id, 
                                        const wxString& caption, 
                                        const wxPoint& pos, 
                                        const wxSize& size, 
                                        long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    if ( GetSizer() ) {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogEditLevelIDMrow destructor
//

DialogEditLevelIDMrow::~DialogEditLevelIDMrow()
{
    ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DialogEditLevelIDMrow::Init()
{
    m_oaddr = NULL;
    m_chkEnableDMRow = NULL;
    m_chkCheckOAddr = NULL;
    m_chkHardOAddr = NULL;
    m_chkMatchZone = NULL;
    m_chkMatchSubzone = NULL;
    m_classMask = NULL;
    m_classFilter = NULL;
    m_typeMask = NULL;
    m_typeFilter = NULL;
    m_comboAction = NULL;
    m_actionParam = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DialogEditLevelIDMrow
//

void DialogEditLevelIDMrow::CreateControls()
{    
    DialogEditLevelIDMrow* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemDialog1, wxID_STATIC, _("Decision Matrix row"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetForegroundColour(wxColour(0, 128, 255));
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText4->SetFont( wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );
#else
    itemStaticText4->SetFont( wxFont( 8, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif    
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemDialog1, wxID_STATIC, _("Originating nickname :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_oaddr = new wxTextCtrl;
    m_oaddr->Create( itemDialog1, ID_TEXTCTRL_ORIGINATING_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_oaddr, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemBoxSizer5->Add(75, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_RIGHT|wxALL, 1);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer9->Add(itemBoxSizer10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_chkEnableDMRow = new wxCheckBox;
    m_chkEnableDMRow->Create( itemDialog1, 
                                ID_CHECKBOX_ENABLE_ROW, 
                                _("Enable DM row"), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                wxALIGN_RIGHT );
    m_chkEnableDMRow->SetValue(false);
    itemBoxSizer10->Add(m_chkEnableDMRow, 0, wxALIGN_RIGHT|wxALL, 5);

    m_chkCheckOAddr = new wxCheckBox;
    m_chkCheckOAddr->Create( itemDialog1, 
                                ID_CHECKBOX_CHECK_OADDR, 
                                _("Check originating address"), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                wxALIGN_RIGHT );
    m_chkCheckOAddr->SetValue(false);
    itemBoxSizer10->Add(m_chkCheckOAddr, 0, wxALIGN_RIGHT|wxALL, 5);

    m_chkHardOAddr = new wxCheckBox;
    m_chkHardOAddr->Create( itemDialog1, 
                                ID_CHECKBOX_HARDCODED_OADDR, 
                                _("Originating address should be hardcoded"), 
                                wxDefaultPosition, wxDefaultSize, 
                                wxALIGN_RIGHT );
    m_chkHardOAddr->SetValue(false);
    itemBoxSizer10->Add(m_chkHardOAddr, 0, wxALIGN_RIGHT|wxALL, 5);

    m_chkMatchZone = new wxCheckBox;
    m_chkMatchZone->Create( itemDialog1, 
                                ID_CHECKBOX_MATCH_ZONE, 
                                _("Zone should match"), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                wxALIGN_RIGHT );
    m_chkMatchZone->SetValue(false);
    itemBoxSizer10->Add(m_chkMatchZone, 0, wxALIGN_RIGHT|wxALL, 5);

    m_chkMatchSubzone = new wxCheckBox;
    m_chkMatchSubzone->Create( itemDialog1, 
                                    ID_CHECKBOX_MATCH_SUBZONE, 
                                    _("Subzone should match"), 
                                    wxDefaultPosition, 
                                    wxDefaultSize, 
                                    wxALIGN_RIGHT );
    m_chkMatchSubzone->SetValue(false);
    itemBoxSizer10->Add(m_chkMatchSubzone, 0, wxALIGN_RIGHT|wxALL, 5);

    itemBoxSizer9->Add(80, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer17, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText;
    itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Class Mask: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_classMask = new wxTextCtrl;
    m_classMask->Create( itemDialog1, ID_TEXTCTRL_CLASS_MASK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_classMask->SetBackgroundColour(wxColour(255, 192, 203));
    itemBoxSizer17->Add(m_classMask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxButton* itemButton20 = new wxButton;
    itemButton20->Create( itemDialog1, ID_BUTTON_SELECT_EVENTS, _("..."), wxDefaultPosition, wxSize(20, -1), 0 );
    itemButton20->SetHelpText(_("Set filter/mask for events thatshould  trigger the DM row"));
    if (DialogEditLevelIDMrow::ShowToolTips())
        itemButton20->SetToolTip(_("Set filter/mask for events thatshould  trigger the DM row"));
    itemBoxSizer17->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer17->Add(50, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemDialog1, wxID_STATIC, _("Class Filter: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_classFilter = new wxTextCtrl;
    m_classFilter->Create( itemDialog1, ID_TEXTCTRL_CLASS_FILTER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_classFilter->SetBackgroundColour(wxColour(255, 192, 203));
    itemBoxSizer22->Add(m_classFilter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer22->Add(80, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer26, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText;
    itemStaticText27->Create( itemDialog1, wxID_STATIC, _("Type Mask: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(itemStaticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_typeMask = new wxTextCtrl;
    m_typeMask->Create( itemDialog1, ID_TEXTCTRL_TYPE_MASK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_typeMask->SetBackgroundColour(wxColour(144, 238, 144));
    itemBoxSizer26->Add(m_typeMask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer26->Add(80, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer30, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create( itemDialog1, wxID_STATIC, _("Type Filter: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(itemStaticText31, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_typeFilter = new wxTextCtrl;
    m_typeFilter->Create( itemDialog1, ID_TEXTCTRL_TYPE_FILTER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_typeFilter->SetBackgroundColour(wxColour(144, 238, 144));
    itemBoxSizer30->Add(m_typeFilter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer30->Add(80, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer34, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemDialog1, wxID_STATIC, _("Action :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer34->Add(itemStaticText35, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxArrayString m_comboActionStrings;
    m_comboAction = new wxComboBox;
    m_comboAction->Create( itemDialog1, 
                            ID_COMBOCTRL_ACTION, 
                            wxEmptyString, 
                            wxDefaultPosition, 
                            wxSize(250, -1), 
                            m_comboActionStrings, 
                            wxCB_READONLY );
    itemBoxSizer34->Add(m_comboAction, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer34->Add(76, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer38 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer38, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemDialog1, wxID_STATIC, _("Action parameter : "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer38->Add(itemStaticText39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_actionParam = new wxTextCtrl;
    m_actionParam->Create( itemDialog1, ID_TEXTCTRL_ACTION_PARAMETER, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer38->Add(m_actionParam, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer38->Add(82, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer42, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton43 = new wxButton;
    itemButton43->Create( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(itemButton43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton44 = new wxButton;
    itemButton44->Create( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool DialogEditLevelIDMrow::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap DialogEditLevelIDMrow::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DialogEditLevelIDMrow::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == _T("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }

    return wxNullIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SELECT_EVENTS
//

void DialogEditLevelIDMrow::OnButtonSelectEventsClick( wxCommandEvent& event )
{
    DialogSetfiltermask dlg( this );

    if ( wxID_OK == dlg.ShowModal() ) {
        m_classMask->SetValue( wxString::Format( _("0x%04x"), dlg.m_classmask ) );
        m_classFilter->SetValue( wxString::Format( _("0x%04x"), dlg.m_classfilter ) );
        m_typeMask->SetValue( wxString::Format( _("0x%04x"), dlg.m_typemask ) );
        m_typeFilter->SetValue( wxString::Format( _("0x%04x"), dlg.m_typefilter ) );
    }

    event.Skip(); 
}

