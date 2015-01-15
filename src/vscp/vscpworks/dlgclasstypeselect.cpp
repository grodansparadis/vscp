/////////////////////////////////////////////////////////////////////////////
// Name:        dlgclasstypeselect.cpp
// Purpose:     
// Author:      Ake Hedman, Paradise of the Frog/Grodans Paradis AB
// Modified by: 
// Created:     13/01/2015 14:13:17
// RCS-ID:      
// Copyright:   Copyright (C) 2014 Grodans Paradis AB
// Licence:     
/////////////////////////////////////////////////////////////////////////////

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

#include "dlgclasstypeselect.h"

////@begin XPM images
////@end XPM images


/*
 * dlgClassTypeSelect type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgClassTypeSelect, wxDialog )


/*
 * dlgClassTypeSelect event table definition
 */

BEGIN_EVENT_TABLE( dlgClassTypeSelect, wxDialog )

////@begin dlgClassTypeSelect event table entries
    EVT_COMBOBOX( ID_COMBOBOX, dlgClassTypeSelect::OnComboboxSelected )
    EVT_COMBOBOX( ID_COMBOBOX1, dlgClassTypeSelect::OnCombobox1Selected )
    EVT_BUTTON( ID_BUTTON, dlgClassTypeSelect::OnButtonClick )
    EVT_BUTTON( ID_BUTTON1, dlgClassTypeSelect::OnButton1Click )
////@end dlgClassTypeSelect event table entries

END_EVENT_TABLE()


/*
 * dlgClassTypeSelect constructors
 */

dlgClassTypeSelect::dlgClassTypeSelect()
{
    Init();
}

dlgClassTypeSelect::dlgClassTypeSelect( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * dlgClassTypeSelect creator
 */

bool dlgClassTypeSelect::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgClassTypeSelect creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end dlgClassTypeSelect creation
    return true;
}


/*
 * dlgClassTypeSelect destructor
 */

dlgClassTypeSelect::~dlgClassTypeSelect()
{
////@begin dlgClassTypeSelect destruction
////@end dlgClassTypeSelect destruction
}


/*
 * Member initialisation
 */

void dlgClassTypeSelect::Init()
{
////@begin dlgClassTypeSelect member initialisation
    m_ctrlComboType = NULL;
////@end dlgClassTypeSelect member initialisation
}


/*
 * Control creation for dlgClassTypeSelect
 */

void dlgClassTypeSelect::CreateControls()
{    
////@begin dlgClassTypeSelect content construction
    dlgClassTypeSelect* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("VSCP Class"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemComboBox4Strings;
    wxComboBox* itemComboBox4 = new wxComboBox( itemDialog1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox4Strings, wxCB_DROPDOWN );
    itemBoxSizer2->Add(itemComboBox4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("VSCP Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText5->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlComboTypeStrings;
    m_ctrlComboType = new wxComboBox( itemDialog1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ctrlComboTypeStrings, wxCB_DROPDOWN );
    itemBoxSizer2->Add(m_ctrlComboType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_BUTTON, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_BUTTON1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end dlgClassTypeSelect content construction
}


/*
 * Should we show tooltips?
 */

bool dlgClassTypeSelect::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap dlgClassTypeSelect::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgClassTypeSelect bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end dlgClassTypeSelect bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon dlgClassTypeSelect::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgClassTypeSelect icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end dlgClassTypeSelect icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void dlgClassTypeSelect::OnButtonClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in dlgClassTypeSelect.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in dlgClassTypeSelect. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void dlgClassTypeSelect::OnButton1Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in dlgClassTypeSelect.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in dlgClassTypeSelect. 
}


/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
 */

void dlgClassTypeSelect::OnComboboxSelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX in dlgClassTypeSelect.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX in dlgClassTypeSelect. 
}


/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX1
 */

void dlgClassTypeSelect::OnCombobox1Selected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX1 in dlgClassTypeSelect.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX1 in dlgClassTypeSelect. 
}

