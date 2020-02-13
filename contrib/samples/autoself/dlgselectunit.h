/////////////////////////////////////////////////////////////////////////////
// Name:        dlgselectunit.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 14:19:44 CEST
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

#ifndef _DLGSELECTUNIT_H_
#define _DLGSELECTUNIT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgselectunit.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgselectunit_symbols.h"
////@end includes

#include "autoself.h"


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DLGSELECTUNIT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGSELECTUNIT_TITLE _("Select unit")
#define SYMBOL_DLGSELECTUNIT_IDNAME ID_DIALOG_SELECT_UNIT
#define SYMBOL_DLGSELECTUNIT_SIZE wxSize(400, 300)
#define SYMBOL_DLGSELECTUNIT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgSelectUnit class declaration
 */

class dlgSelectUnit: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgSelectUnit )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgSelectUnit();
    dlgSelectUnit( wxWindow* parent, wxWindowID id = SYMBOL_DLGSELECTUNIT_IDNAME, const wxString& caption = SYMBOL_DLGSELECTUNIT_TITLE, const wxPoint& pos = SYMBOL_DLGSELECTUNIT_POSITION, const wxSize& size = SYMBOL_DLGSELECTUNIT_SIZE, long style = SYMBOL_DLGSELECTUNIT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGSELECTUNIT_IDNAME, const wxString& caption = SYMBOL_DLGSELECTUNIT_TITLE, const wxPoint& pos = SYMBOL_DLGSELECTUNIT_POSITION, const wxSize& size = SYMBOL_DLGSELECTUNIT_SIZE, long style = SYMBOL_DLGSELECTUNIT_STYLE );

    /// Destructor
    ~dlgSelectUnit();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
    /// Read value (decimal or heximal )
    unsigned char readStringValue( wxString& str );
    
    // Current selected index in global node array
    long m_selectedIndex;

////@begin dlgSelectUnit event handler declarations

  /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_UNITS
  void OnListboxUnitsSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_NEW
  void OnButtonNewClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
  void OnButtonEditClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_DELETE
  void OnButtonDeleteClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

////@end dlgSelectUnit event handler declarations

////@begin dlgSelectUnit member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgSelectUnit member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgSelectUnit member variables
  wxListBox* m_ctrlListBoxNodes;
  /// Control identifiers
  enum {
    ID_DIALOG_SELECT_UNIT = 10014,
    ID_LISTBOX_UNITS = 10415,
    ID_BUTTON_NEW = 13000,
    ID_BUTTON_EDIT = 10016,
    ID_BUTTON_DELETE = 10001
  };
////@end dlgSelectUnit member variables
};

#endif
    // _DLGSELECTUNIT_H_
