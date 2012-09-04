/////////////////////////////////////////////////////////////////////////////
// Name:        defaultsettings.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 23 Oct 2009 16:00:29 CEST
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
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

#include "defaultsettings.h"

////@begin XPM images

////@end XPM images


/*!
 * DefaultSettings type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DefaultSettings, wxDialog )


/*!
 * DefaultSettings event table definition
 */

BEGIN_EVENT_TABLE( DefaultSettings, wxDialog )

////@begin DefaultSettings event table entries
////@end DefaultSettings event table entries

END_EVENT_TABLE()


/*!
 * DefaultSettings constructors
 */

DefaultSettings::DefaultSettings()
{
    Init();
}

DefaultSettings::DefaultSettings( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * DefaultSettings creator
 */

bool DefaultSettings::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DefaultSettings creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end DefaultSettings creation
    return true;
}


/*!
 * DefaultSettings destructor
 */

DefaultSettings::~DefaultSettings()
{
////@begin DefaultSettings destruction
////@end DefaultSettings destruction
}


/*!
 * Member initialisation
 */

void DefaultSettings::Init()
{
////@begin DefaultSettings member initialisation
////@end DefaultSettings member initialisation
}


/*!
 * Control creation for DefaultSettings
 */

void DefaultSettings::CreateControls()
{    
////@begin DefaultSettings content construction
    DefaultSettings* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

////@end DefaultSettings content construction
}


/*!
 * Should we show tooltips?
 */

bool DefaultSettings::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap DefaultSettings::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DefaultSettings bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end DefaultSettings bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DefaultSettings::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DefaultSettings icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end DefaultSettings icon retrieval
}
