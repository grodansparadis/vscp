/////////////////////////////////////////////////////////////////////////////
// Name:        dlgselectdaemoninterface.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     04/01/2009 11:55:25
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGSELECTDAEMONINTERFACE_H_
#define _DLGSELECTDAEMONINTERFACE_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgselectdaemoninterface.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
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
#define SYMBOL_DLGSELECTDAEMONINTERFACE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGSELECTDAEMONINTERFACE_TITLE _("Select session window to open")
#define SYMBOL_DLGSELECTDAEMONINTERFACE_IDNAME ID_DIALOG_SELECT_DAEMON_INTERFACE
#define SYMBOL_DLGSELECTDAEMONINTERFACE_SIZE wxSize(500, 300)
#define SYMBOL_DLGSELECTDAEMONINTERFACE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgSelectDaemonInterface class declaration
 */

class dlgSelectDaemonInterface: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgSelectDaemonInterface )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgSelectDaemonInterface();
  dlgSelectDaemonInterface( wxWindow* parent, 
                                wxWindowID id = SYMBOL_DLGSELECTDAEMONINTERFACE_IDNAME, 
                                const wxString& caption = SYMBOL_DLGSELECTDAEMONINTERFACE_TITLE, 
                                const wxPoint& pos = SYMBOL_DLGSELECTDAEMONINTERFACE_POSITION, 
                                const wxSize& size = SYMBOL_DLGSELECTDAEMONINTERFACE_SIZE, 
                                long style = SYMBOL_DLGSELECTDAEMONINTERFACE_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGSELECTDAEMONINTERFACE_IDNAME, 
                    const wxString& caption = SYMBOL_DLGSELECTDAEMONINTERFACE_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGSELECTDAEMONINTERFACE_POSITION, 
                    const wxSize& size = SYMBOL_DLGSELECTDAEMONINTERFACE_SIZE, 
                    long style = SYMBOL_DLGSELECTDAEMONINTERFACE_STYLE );

  /// Destructor
  ~dlgSelectDaemonInterface();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgSelectDaemonInterface event handler declarations

  /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_SELECT_DAEMON_INTERFACE
  void OnInitDialog( wxInitDialogEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_SELECT_DAEMON_INTERFACE
  void OnCloseWindow( wxCloseEvent& event );

////@end dlgSelectDaemonInterface event handler declarations

////@begin dlgSelectDaemonInterface member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgSelectDaemonInterface member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgSelectDaemonInterface member variables
  wxListBox* m_ctrlListInterfaces;
  /// Control identifiers
  enum {
    ID_DIALOG_SELECT_DAEMON_INTERFACE = 11000,
    ID_CtrlListInterfaces = 11001
  };
////@end dlgSelectDaemonInterface member variables
};

#endif
  // _DLGSELECTDAEMONINTERFACE_H_
