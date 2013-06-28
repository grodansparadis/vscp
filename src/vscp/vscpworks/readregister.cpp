/////////////////////////////////////////////////////////////////////////////
// Name:        readregister.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Fri 28 Jun 2013 18:28:11 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
#pragma implementation "readregister.h"
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

#include "readregister.h"

////@begin XPM images

////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadRegister type definition
//

IMPLEMENT_DYNAMIC_CLASS( ReadRegister, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadRegister event table definition
//

BEGIN_EVENT_TABLE( ReadRegister, wxDialog )

////@begin ReadRegister event table entries
////@end ReadRegister event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadRegister constructors
//

ReadRegister::ReadRegister()
{
  Init();
}

ReadRegister::ReadRegister( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadRegister creator
//

bool ReadRegister::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ReadRegister creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  Centre();
////@end ReadRegister creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadRegister destructor
//

ReadRegister::~ReadRegister()
{
////@begin ReadRegister destruction
////@end ReadRegister destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void ReadRegister::Init()
{
////@begin ReadRegister member initialisation
////@end ReadRegister member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for ReadRegister
//

void ReadRegister::CreateControls()
{    
////@begin ReadRegister content construction
  ReadRegister* itemDialog1 = this;

  wxPanel* itemPanel2 = new wxPanel;
  itemPanel2->Create( itemDialog1, ID_PANEL37, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxStaticText* itemStaticText4 = new wxStaticText;
  itemStaticText4->Create( itemPanel2, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end ReadRegister content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool ReadRegister::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap ReadRegister::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin ReadRegister bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end ReadRegister bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon ReadRegister::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin ReadRegister icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
////@end ReadRegister icon retrieval
}
