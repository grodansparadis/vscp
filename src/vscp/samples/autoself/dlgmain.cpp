/////////////////////////////////////////////////////////////////////////////
// Name:        dlgmain.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:43:18 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgmain.h"
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

#include "dlgmain.h"

////@begin XPM images
////@end XPM images

/*!
 * dlgMain type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgMain, wxDialog )

/*!
 * dlgMain event table definition
 */

BEGIN_EVENT_TABLE( dlgMain, wxDialog )

////@begin dlgMain event table entries
////@end dlgMain event table entries

END_EVENT_TABLE()

/*!
 * dlgMain constructors
 */

dlgMain::dlgMain()
{
    Init();
}

dlgMain::dlgMain( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * dlgMain creator
 */

bool dlgMain::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgMain creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end dlgMain creation
    return true;
}

/*!
 * dlgMain destructor
 */

dlgMain::~dlgMain()
{
////@begin dlgMain destruction
////@end dlgMain destruction
}

/*!
 * Member initialisation 
 */

void dlgMain::Init()
{
////@begin dlgMain member initialisation
////@end dlgMain member initialisation
}

/*!
 * Control creation for dlgMain
 */

void dlgMain::CreateControls()
{    
////@begin dlgMain content construction
    dlgMain* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxButton* itemButton3 = new wxButton( itemDialog1, ID_BUTTON1, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer4);

    wxBitmapButton* itemBitmapButton5 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemBoxSizer4->Add(itemBitmapButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton6 = new wxBitmapButton( itemDialog1, ID_BITMAPBUTTON, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemBoxSizer4->Add(itemBitmapButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end dlgMain content construction
}

/*!
 * Should we show tooltips?
 */

bool dlgMain::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgMain::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgMain bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end dlgMain bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgMain::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgMain icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end dlgMain icon retrieval
}
