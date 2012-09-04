/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsettings.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Mon Jun 18 14:52:05 2007
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource <ake@eurosource.se>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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

#ifndef _DLGSETTINGS_H_
#define _DLGSETTINGS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgsettings.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgsettings_symbols.h"
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
#define SYMBOL_DLGSETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGSETTINGS_TITLE _("General Settings")
#define SYMBOL_DLGSETTINGS_IDNAME ID_DIALOG_SETTINGS
#define SYMBOL_DLGSETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_DLGSETTINGS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgSettings class declaration
 */

class dlgSettings: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgSettings )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgSettings();
  dlgSettings( wxWindow* parent, wxWindowID id = SYMBOL_DLGSETTINGS_IDNAME, const wxString& caption = SYMBOL_DLGSETTINGS_TITLE, const wxPoint& pos = SYMBOL_DLGSETTINGS_POSITION, const wxSize& size = SYMBOL_DLGSETTINGS_SIZE, long style = SYMBOL_DLGSETTINGS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGSETTINGS_IDNAME, const wxString& caption = SYMBOL_DLGSETTINGS_TITLE, const wxPoint& pos = SYMBOL_DLGSETTINGS_POSITION, const wxSize& size = SYMBOL_DLGSETTINGS_SIZE, long style = SYMBOL_DLGSETTINGS_STYLE );

  /// Destructor
  ~dlgSettings();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgSettings event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_TEST
  void OnButtonTestClick( wxCommandEvent& event );

////@end dlgSettings event handler declarations

////@begin dlgSettings member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgSettings member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgSettings member variables
  wxTextCtrl* m_editVSCPServer;
  wxTextCtrl* m_editVSCPPort;
  wxTextCtrl* m_editVSCPUsername;
  wxTextCtrl* m_editVSCPPassword;
  wxCheckBox* m_bCheckLogFile;
  wxTextCtrl* m_editPathLogFile;
  /// Control identifiers
  enum {
    ID_DIALOG_SETTINGS = 10010,
    ID_TEXTCTRL_SETTINGS_VSCP_SERVER = 10035,
    ID_TEXTCTRL_SETTINGS_PORT = 10000,
    ID_TEXTCTRL_SETTINGS_USERNAME = 10001,
    ID_TEXTCTRL_PASSWORD_SETTINGS = 10002,
    ID_BCheckLogFile = 10004,
    ID_EditPathLogFile = 10003,
    ID_BUTTON_TEST = 10037
  };
////@end dlgSettings member variables
};

#endif
  // _DLGSETTINGS_H_
