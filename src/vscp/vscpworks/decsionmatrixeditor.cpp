/////////////////////////////////////////////////////////////////////////////
// Name:        decsionmatrixeditor.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/10/2012 13:39:52
// RCS-ID:      
// Copyright:   (C) 2007-2017 
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
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "decsionmatrixeditor.h"
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

#include "decsionmatrixeditor.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decsionmatrixeditor type definition
//

IMPLEMENT_CLASS( Decsionmatrixeditor, wxFrame )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decsionmatrixeditor event table definition
//

BEGIN_EVENT_TABLE( Decsionmatrixeditor, wxFrame )

////@begin Decsionmatrixeditor event table entries
////@end Decsionmatrixeditor event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decsionmatrixeditor constructors
//

Decsionmatrixeditor::Decsionmatrixeditor()
{
  Init();
}

Decsionmatrixeditor::Decsionmatrixeditor( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create( parent, id, caption, pos, size, style );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decsionmatrixeditor creator
//

bool Decsionmatrixeditor::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin Decsionmatrixeditor creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  Centre();
////@end Decsionmatrixeditor creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decsionmatrixeditor destructor
//

Decsionmatrixeditor::~Decsionmatrixeditor()
{
////@begin Decsionmatrixeditor destruction
////@end Decsionmatrixeditor destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void Decsionmatrixeditor::Init()
{
////@begin Decsionmatrixeditor member initialisation
////@end Decsionmatrixeditor member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for Decsionmatrixeditor
//

void Decsionmatrixeditor::CreateControls()
{    
////@begin Decsionmatrixeditor content construction
  Decsionmatrixeditor* itemFrame1 = this;

////@end Decsionmatrixeditor content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool Decsionmatrixeditor::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap Decsionmatrixeditor::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin Decsionmatrixeditor bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end Decsionmatrixeditor bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon Decsionmatrixeditor::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin Decsionmatrixeditor icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end Decsionmatrixeditor icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_REGISTERS
//

void Decsionmatrixeditor::OnCellRightClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_REGISTERS in Decsionmatrixeditor.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_REGISTERS in Decsionmatrixeditor. 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_GRID_REGISTERS
//

void Decsionmatrixeditor::OnLeftDClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_GRID_REGISTERS in Decsionmatrixeditor.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_GRID_REGISTERS in Decsionmatrixeditor. 
}

