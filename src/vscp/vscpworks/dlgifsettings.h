/////////////////////////////////////////////////////////////////////////////
// Name:        dlgifsettings.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:48:03
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

#ifndef _DLGIFSETTINGS_H_
#define _DLGIFSETTINGS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgifsettings.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgifsettings_symbols.h"
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
#define SYMBOL_DLGINTERFACESETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGINTERFACESETTINGS_TITLE _("CANAL Interface settings")
#define SYMBOL_DLGINTERFACESETTINGS_IDNAME ID_DIALOG_INTERFACE_SETTINGS
#define SYMBOL_DLGINTERFACESETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_DLGINTERFACESETTINGS_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgInterfaceSettings class declaration
 */

class dlgInterfaceSettings: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgInterfaceSettings )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgInterfaceSettings();
    dlgInterfaceSettings( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DLGINTERFACESETTINGS_IDNAME, 
                            const wxString& caption = SYMBOL_DLGINTERFACESETTINGS_TITLE, 
                            const wxPoint& pos = SYMBOL_DLGINTERFACESETTINGS_POSITION, 
                            const wxSize& size = SYMBOL_DLGINTERFACESETTINGS_SIZE, 
                            long style = SYMBOL_DLGINTERFACESETTINGS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGINTERFACESETTINGS_IDNAME, 
                    const wxString& caption = SYMBOL_DLGINTERFACESETTINGS_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGINTERFACESETTINGS_POSITION, 
                    const wxSize& size = SYMBOL_DLGINTERFACESETTINGS_SIZE, 
                    long style = SYMBOL_DLGINTERFACESETTINGS_STYLE );

    /// Destructor
    ~dlgInterfaceSettings();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgInterfaceSettings event handler declarations

  /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_INTERFACE_SETTINGS
  void OnInitDialog( wxInitDialogEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_INTERFACE_SETTINGS
  void OnCloseWindow( wxCloseEvent& event );

////@end dlgInterfaceSettings event handler declarations

////@begin dlgInterfaceSettings member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgInterfaceSettings member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgInterfaceSettings member variables
  /// Control identifiers
  enum {
    ID_DIALOG_INTERFACE_SETTINGS = 15000,
    ID_TEXTCTRL19 = 15001,
    ID_TEXTCTRL20 = 15002,
    ID_BUTTON11 = 15003,
    ID_TEXTCTRL21 = 15004,
    ID_TEXTCTRL22 = 15005,
    ID_TEXTCTRL_FILTER = 15006,
    ID_BUTTON_DEVICE_FILTER = 15007,
    ID_TEXTCTRL_MASK = 15008,
    ID_BUTTON_DEVICE_MASK = 15009
  };
////@end dlgInterfaceSettings member variables
};

#endif
    // _DLGIFSETTINGS_H_
