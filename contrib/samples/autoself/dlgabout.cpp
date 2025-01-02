/////////////////////////////////////////////////////////////////////////////
// Name:        dlgabout.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Fri 27 Apr 2007 11:56:55 CEST
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
// Copyright (C) 2000-2025 Ake Hedman, eurosource, <ake@eurosource.se>
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
#pragma implementation "dlgabout.h"
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

#include "dlgabout.h"

////@begin XPM images
/* XPM */
static char *wxwin32x32_xpm[] = {
"32 32 6 1",
"     c None",
".    c #000000",
"X    c #000084",
"o    c #FFFFFF",
"O    c #FFFF00",
"+    c #FF0000",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"        ..............          ",
"        .XXXXXXXXXXXX.          ",
"        .XXXXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXX..............",
"        .XooXXXXXX.OOOOOOOOOOOO.",
".........XooXXXXXX.OOOOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++...........OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".++++++++++++.    ..............",
".++++++++++++.                  ",
"..............                  ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "};

////@end XPM images


//wxString gHTMLContent = _("<body><html><p align=\"center\"><h1>Auto-Self Control</h1></p><p align=\"center\">Programmet kodat for Ljusdals Energi AB av eurosource, Los, http://www.eurosource.se</p><b><p align=\"center\">Copyright (C) 2006,2007 Ljusdals Energi AB</p></b></html><body>");




/*!
 * dlgAbout type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgAbout, wxDialog )

/*!
 * dlgAbout event table definition
 */

BEGIN_EVENT_TABLE( dlgAbout, wxDialog )

////@begin dlgAbout event table entries
////@end dlgAbout event table entries

END_EVENT_TABLE()

/*!
 * dlgAbout constructors
 */

dlgAbout::dlgAbout()
{
    Init();
}

dlgAbout::dlgAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * dlgAbout creator
 */

bool dlgAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgAbout creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("wxwin32x321.xpm")));
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end dlgAbout creation

    return true;
}

/*!
 * dlgAbout destructor
 */

dlgAbout::~dlgAbout()
{
////@begin dlgAbout destruction
////@end dlgAbout destruction
}

/*!
 * Member initialisation 
 */

void dlgAbout::Init()
{
////@begin dlgAbout member initialisation
////@end dlgAbout member initialisation
}

/*!
 * Control creation for dlgAbout
 */

void dlgAbout::CreateControls()
{    
////@begin dlgAbout content construction
    dlgAbout* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxHtmlWindow* itemHtmlWindow3 = new wxHtmlWindow;
    itemHtmlWindow3->Create( itemDialog1, ID_HTMLWINDOW_ABOUT, wxDefaultPosition, wxSize(400, 150), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemHtmlWindow3->SetPage(_("<body><html><p align=\"center\"><h1>Auto-Self Control</h1></p><p align=\"center\">Program created for Ljusdals Energi AB by eurosource, Sweden, http://www.eurosource.se</p><b><p align=\"center\">Copyright (C) 2006,2007 Ljusdals Energi AB</p></b></html><body>"));
    itemBoxSizer2->Add(itemHtmlWindow3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton4 = new wxButton;
    itemButton4->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end dlgAbout content construction
}

/*!
 * Should we show tooltips?
 */

bool dlgAbout::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgAbout::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgAbout bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end dlgAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgAbout::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgAbout icon retrieval
    wxUnusedVar(name);
    if (name == _T("wxwin32x321.xpm"))
    {
        wxIcon icon(wxwin32x32_xpm);
        return icon;
    }
    return wxNullIcon;
////@end dlgAbout icon retrieval
}
