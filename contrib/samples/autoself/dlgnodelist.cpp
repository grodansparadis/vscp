/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnodelist.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 13:50:12 CEST
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
#pragma implementation "dlgnodelist.h"
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

#include "dlgnodelist.h"

////@begin XPM images
////@end XPM images

/*!
 * dlgNodeList type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgNodeList, wxDialog )

/*!
 * dlgNodeList event table definition
 */

BEGIN_EVENT_TABLE( dlgNodeList, wxDialog )

////@begin dlgNodeList event table entries
////@end dlgNodeList event table entries

END_EVENT_TABLE()

/*!
 * dlgNodeList constructors
 */

dlgNodeList::dlgNodeList()
{
    Init();
}

dlgNodeList::dlgNodeList( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * dlgNodeList creator
 */

bool dlgNodeList::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgNodeList creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end dlgNodeList creation
    return true;
}

/*!
 * dlgNodeList destructor
 */

dlgNodeList::~dlgNodeList()
{
////@begin dlgNodeList destruction
////@end dlgNodeList destruction
}

/*!
 * Member initialisation 
 */

void dlgNodeList::Init()
{
////@begin dlgNodeList member initialisation
////@end dlgNodeList member initialisation
}

/*!
 * Control creation for dlgNodeList
 */

void dlgNodeList::CreateControls()
{    
////@begin dlgNodeList content construction
    dlgNodeList* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

    wxArrayString itemListBox4Strings;
    wxListBox* itemListBox4 = new wxListBox;
    itemListBox4->Create( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(320, 200), itemListBox4Strings, wxLB_SINGLE );
    itemBoxSizer3->Add(itemListBox4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemBoxSizer2->Add(5, 20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, 0, 0);

    wxButton* itemButton7 = new wxButton;
    itemButton7->Create( itemDialog1, ID_BUTTON, _("New..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton8 = new wxButton;
    itemButton8->Create( itemDialog1, ID_BUTTON1, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton9 = new wxButton;
    itemButton9->Create( itemDialog1, ID_BUTTON2, _("Delete..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemBoxSizer6->Add(5, 20, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton12 = new wxButton;
    itemButton12->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

////@end dlgNodeList content construction
}

/*!
 * Should we show tooltips?
 */

bool dlgNodeList::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgNodeList::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgNodeList bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end dlgNodeList bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgNodeList::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgNodeList icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end dlgNodeList icon retrieval
}
