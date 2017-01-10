/////////////////////////////////////////////////////////////////////////////
// Name:        decsionmatrixeditor.h
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

#ifndef _DECSIONMATRIXEDITOR_H_
#define _DECSIONMATRIXEDITOR_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "decsionmatrixeditor.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DECSIONMATRIXEDITOR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DECSIONMATRIXEDITOR_TITLE _("Decsion matrix editor")
#define SYMBOL_DECSIONMATRIXEDITOR_IDNAME ID_DECSIONMATRIXEDITOR
#define SYMBOL_DECSIONMATRIXEDITOR_SIZE wxSize(400, 300)
#define SYMBOL_DECSIONMATRIXEDITOR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * Decsionmatrixeditor class declaration
 */

class Decsionmatrixeditor: public wxFrame
{    
  DECLARE_CLASS( Decsionmatrixeditor )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  Decsionmatrixeditor();
  Decsionmatrixeditor( wxWindow* parent, wxWindowID id = SYMBOL_DECSIONMATRIXEDITOR_IDNAME, const wxString& caption = SYMBOL_DECSIONMATRIXEDITOR_TITLE, const wxPoint& pos = SYMBOL_DECSIONMATRIXEDITOR_POSITION, const wxSize& size = SYMBOL_DECSIONMATRIXEDITOR_SIZE, long style = SYMBOL_DECSIONMATRIXEDITOR_STYLE );

  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DECSIONMATRIXEDITOR_IDNAME, const wxString& caption = SYMBOL_DECSIONMATRIXEDITOR_TITLE, const wxPoint& pos = SYMBOL_DECSIONMATRIXEDITOR_POSITION, const wxSize& size = SYMBOL_DECSIONMATRIXEDITOR_SIZE, long style = SYMBOL_DECSIONMATRIXEDITOR_STYLE );

  /// Destructor
  ~Decsionmatrixeditor();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin Decsionmatrixeditor event handler declarations

////@end Decsionmatrixeditor event handler declarations

////@begin Decsionmatrixeditor member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end Decsionmatrixeditor member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin Decsionmatrixeditor member variables
  /// Control identifiers
  enum {
    ID_DECSIONMATRIXEDITOR = 10147
  };
////@end Decsionmatrixeditor member variables
};

#endif
  // _DECSIONMATRIXEDITOR_H_
