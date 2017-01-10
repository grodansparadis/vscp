/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanalinterfacesettings.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 20:45:11 CEST
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

#ifndef _DLGCANALINTERFACESETTINGS_H_
#define _DLGCANALINTERFACESETTINGS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgcanalinterfacesettings.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgcanalinterfacesettings_symbols.h"
#include "wx/propdlg.h"
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
#define SYMBOL_DLGCANALINTERFACESETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DLGCANALINTERFACESETTINGS_TITLE _("CANAL Interface Settings")
#define SYMBOL_DLGCANALINTERFACESETTINGS_IDNAME ID_DLGINTERFACESETTINGS
#define SYMBOL_DLGCANALINTERFACESETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_DLGCANALINTERFACESETTINGS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgCanalInterfaceSettings class declaration
 */

class dlgCanalInterfaceSettings: public wxPropertySheetDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgCanalInterfaceSettings )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgCanalInterfaceSettings();
  dlgCanalInterfaceSettings( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANALINTERFACESETTINGS_IDNAME, const wxString& caption = SYMBOL_DLGCANALINTERFACESETTINGS_TITLE, const wxPoint& pos = SYMBOL_DLGCANALINTERFACESETTINGS_POSITION, const wxSize& size = SYMBOL_DLGCANALINTERFACESETTINGS_SIZE, long style = SYMBOL_DLGCANALINTERFACESETTINGS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANALINTERFACESETTINGS_IDNAME, const wxString& caption = SYMBOL_DLGCANALINTERFACESETTINGS_TITLE, const wxPoint& pos = SYMBOL_DLGCANALINTERFACESETTINGS_POSITION, const wxSize& size = SYMBOL_DLGCANALINTERFACESETTINGS_SIZE, long style = SYMBOL_DLGCANALINTERFACESETTINGS_STYLE );

  /// Destructor
  ~dlgCanalInterfaceSettings();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgCanalInterfaceSettings event handler declarations

////@end dlgCanalInterfaceSettings event handler declarations

////@begin dlgCanalInterfaceSettings member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgCanalInterfaceSettings member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgCanalInterfaceSettings member variables
  wxTextCtrl* m_DirectFlags;
  wxTextCtrl* m_DriverName;
  wxTextCtrl* m_PathToDriver;
  wxTextCtrl* m_DeviceConfigurationString;
  wxTextCtrl* m_Flags;
  wxStaticText* m_idFilter;
  wxTextCtrl* m_C;
  wxTextCtrl* m_idMask;
  wxTextCtrl* m_RemoteServerURL;
  wxTextCtrl* m_RemoteServerPort;
  wxTextCtrl* m_RemoteServerUsername;
  wxTextCtrl* m_RemoteServerPassword;
  /// Control identifiers
  enum {
    ID_DLGINTERFACESETTINGS = 21004,
    ID_PANEL = 21000,
    ID_DirectFlags = 21001,
    ID_PANEL1 = 21002,
    ID_DriverName = 21003,
    ID_PathToDriver = 21008,
    ID_BUTTON_CANAL_DRIVER_PATH = 21005,
    ID_DeviceConfigurationString = 21009,
    ID_BUTTON_CANAL_CONFIGURATION = 21010,
    ID_Flags = 21011,
    ID_C = 21012,
    ID_IdMask = 21013,
    ID_BUTTON4 = 21014,
    ID_BUTTON_CANAL_DRIVER_WIZARD = 21018,
    ID_PANEL2 = 21019,
    ID_RemoteServerURL = 21020,
    ID_RemoteServerPort = 21021,
    ID_RemoteServerUsername = 21022,
    ID_RemoteServerPassword = 21023,
    ID_BUTTON1 = 21024
  };
////@end dlgCanalInterfaceSettings member variables
};

#endif
  // _DLGCANALINTERFACESETTINGS_H_
