/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsetmanufactdata.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 25 Oct 2007 22:17:45 CEST
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

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

#include "dlgsetmanufactdata.h"


/*!
 * CsetManufactData type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CsetManufactData, wxDialog )


/*!
 * CsetManufactData event table definition
 */

BEGIN_EVENT_TABLE( CsetManufactData, wxDialog )


END_EVENT_TABLE()


/*!
 * CsetManufactData constructors
 */

CsetManufactData::CsetManufactData()
{
    Init();
}

CsetManufactData::CsetManufactData( wxWindow* parent, 
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
 * CsetManufactData creator
 */

bool CsetManufactData::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


/*!
 * CsetManufactData destructor
 */

CsetManufactData::~CsetManufactData()
{
    
}


/*!
 * Member initialisation
 */

void CsetManufactData::Init()
{
    m_pctrlGUID = NULL;
    m_pctrlManDevId = NULL;
    m_pctrlManDevSubId = NULL;
}


/*!
 * Control creation for CsetManufactData
 */

void CsetManufactData::CreateControls()
{    
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
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CsetManufactData::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}
