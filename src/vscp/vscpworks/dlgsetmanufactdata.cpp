/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsetmanufactdata.cpp
// Purpose:     
// Author:      Anthemion Software Ltd
// Modified by: 
// Created:     21/03/2015 14:17:23
// RCS-ID:      
// Copyright:   (c) Anthemion Software Ltd
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgsetmanufactdata.h"
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

#include "dlgsetmanufactdata.h"

////@begin XPM images
////@end XPM images


/*!
 * CsetManufactData type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CsetManufactData, wxDialog )


/*!
 * CsetManufactData event table definition
 */

BEGIN_EVENT_TABLE( CsetManufactData, wxDialog )

////@begin CsetManufactData event table entries
////@end CsetManufactData event table entries

END_EVENT_TABLE()


/*!
 * CsetManufactData constructors
 */

CsetManufactData::CsetManufactData()
{
    Init();
}

CsetManufactData::CsetManufactData( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CsetManufactData creator
 */

bool CsetManufactData::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CsetManufactData creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CsetManufactData creation
    return true;
}


/*!
 * CsetManufactData destructor
 */

CsetManufactData::~CsetManufactData()
{
////@begin CsetManufactData destruction
////@end CsetManufactData destruction
}


/*!
 * Member initialisation
 */

void CsetManufactData::Init()
{
////@begin CsetManufactData member initialisation
    m_pctrlGUID = NULL;
    m_pctrlManDevId = NULL;
    m_pctrlManDevSubId = NULL;
////@end CsetManufactData member initialisation
}


/*!
 * Control creation for CsetManufactData
 */

void CsetManufactData::CreateControls()
{    
////@begin CsetManufactData content construction
    CsetManufactData* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("GUID :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pctrlGUID = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_GUID, _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"), wxDefaultPosition, wxSize(300, -1), 0 );
    itemBoxSizer3->Add(m_pctrlGUID, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Manufacturer device ID :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pctrlManDevId = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_MANDEVID, _("0x00000000"), wxDefaultPosition, wxSize(300, -1), 0 );
    itemBoxSizer6->Add(m_pctrlManDevId, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_RIGHT|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Manufacturer sub device ID :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pctrlManDevSubId = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_MANDEVSUBID, _("0x00000000"), wxDefaultPosition, wxSize(300, -1), 0 );
    itemBoxSizer9->Add(m_pctrlManDevSubId, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton13->SetDefault();
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ID_BUTTON_LOAD, _("Load from file..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, ID_BUTTON_WRITE, _("Write to device"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CsetManufactData content construction
}


/*!
 * Should we show tooltips?
 */

bool CsetManufactData::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CsetManufactData::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CsetManufactData bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CsetManufactData bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CsetManufactData::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CsetManufactData icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CsetManufactData icon retrieval
}
