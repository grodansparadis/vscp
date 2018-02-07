/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnewcanalsession.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 12:10:45 CEST
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
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGNEWCANALSESSION_H_
#define _DLGNEWCANALSESSION_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgnewcanalsession.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgnewcanalsession_symbols.h"
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
#define SYMBOL_DLGNEWCANALSESSION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGNEWCANALSESSION_TITLE _("Select interface to use for session")
#define SYMBOL_DLGNEWCANALSESSION_IDNAME ID_DIALOG_NEW_CANAL_SESSION
#define SYMBOL_DLGNEWCANALSESSION_SIZE wxSize(400, 300)
#define SYMBOL_DLGNEWCANALSESSION_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgNewCanalSession class declaration
 */

class dlgNewCanalSession: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgNewCanalSession )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgNewCanalSession();
  dlgNewCanalSession( wxWindow* parent, 
                        wxWindowID id = SYMBOL_DLGNEWCANALSESSION_IDNAME, 
                        const wxString& caption = SYMBOL_DLGNEWCANALSESSION_TITLE, 
                        const wxPoint& pos = SYMBOL_DLGNEWCANALSESSION_POSITION, 
                        const wxSize& size = SYMBOL_DLGNEWCANALSESSION_SIZE, 
                        long style = SYMBOL_DLGNEWCANALSESSION_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGNEWCANALSESSION_IDNAME, 
                    const wxString& caption = SYMBOL_DLGNEWCANALSESSION_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGNEWCANALSESSION_POSITION, 
                    const wxSize& size = SYMBOL_DLGNEWCANALSESSION_SIZE, 
                    long style = SYMBOL_DLGNEWCANALSESSION_STYLE );

  /// Destructor
  ~dlgNewCanalSession();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgNewCanalSession event handler declarations

  /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_CANAL_SESSION
  void OnInitDialog( wxInitDialogEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_NEW_CANAL_SESSION
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOKClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCANCELClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD
  void OnButtonAddClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
  void OnButtonEditClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE
  void OnButtonRemoveClick( wxCommandEvent& event );

////@end dlgNewCanalSession event handler declarations

////@begin dlgNewCanalSession member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgNewCanalSession member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgNewCanalSession member variables
  wxListBox* m_ctrlListInterfaces;
  /// Control identifiers
  enum {
    ID_DIALOG_NEW_CANAL_SESSION = 14000,
    ID_LISTBOX_INTERFACES = 14001,
    ID_BUTTON_ADD = 14002,
    ID_BUTTON_EDIT = 14003,
    ID_BUTTON_REMOVE = 14004
  };
////@end dlgNewCanalSession member variables
};

#endif
  // _DLGNEWCANALSESSION_H_
