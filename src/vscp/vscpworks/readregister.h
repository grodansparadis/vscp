/////////////////////////////////////////////////////////////////////////////
// Name:        readregister.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Fri 28 Jun 2013 18:28:11 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
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
//  Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
// 

#ifndef _READREGISTER_H_
#define _READREGISTER_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "readregister.h"
#endif

/*!
 * Includes
 */

////@begin includes
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
#define SYMBOL_READREGISTER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_READREGISTER_TITLE _("Read Register")
#define SYMBOL_READREGISTER_IDNAME ID_READREGISTER
#define SYMBOL_READREGISTER_SIZE wxSize(400, 300)
#define SYMBOL_READREGISTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ReadRegister class declaration
 */

class ReadRegister: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( ReadRegister )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  ReadRegister();
  ReadRegister( wxWindow* parent, 
                    wxWindowID id = SYMBOL_READREGISTER_IDNAME, 
                    const wxString& caption = SYMBOL_READREGISTER_TITLE, 
                    const wxPoint& pos = SYMBOL_READREGISTER_POSITION, 
                    const wxSize& size = SYMBOL_READREGISTER_SIZE, 
                    long style = SYMBOL_READREGISTER_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_READREGISTER_IDNAME, 
                    const wxString& caption = SYMBOL_READREGISTER_TITLE, 
                    const wxPoint& pos = SYMBOL_READREGISTER_POSITION, 
                    const wxSize& size = SYMBOL_READREGISTER_SIZE, 
                    long style = SYMBOL_READREGISTER_STYLE );

  /// Destructor
  ~ReadRegister();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin ReadRegister event handler declarations

////@end ReadRegister event handler declarations

////@begin ReadRegister member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end ReadRegister member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin ReadRegister member variables
  /// Control identifiers
  enum {
    ID_READREGISTER = 20000,
    ID_PANEL37 = 20001
  };
////@end ReadRegister member variables
};

#endif
  // _READREGISTER_H_
