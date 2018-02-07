/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpinterfacesettings.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Fri 26 Oct 2007 11:57:04 CEST
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
// 

#ifndef _DLGVSCPINTERFACESETTINGS_H_
#define _DLGVSCPINTERFACESETTINGS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgvscpinterfacesettings.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/propdlg.h"
////@end includes

#include <vscp.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DLGVSCPINTERFACESETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGVSCPINTERFACESETTINGS_TITLE _("VSCP/CANAL Interface Settings")
#define SYMBOL_DLGVSCPINTERFACESETTINGS_IDNAME ID_DLGVSCPINTERFACESETTINGS
#define SYMBOL_DLGVSCPINTERFACESETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_DLGVSCPINTERFACESETTINGS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgVscpInterfaceSettings class declaration
 */

class dlgVscpInterfaceSettings: public wxPropertySheetDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgVscpInterfaceSettings )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgVscpInterfaceSettings();
  dlgVscpInterfaceSettings( wxWindow* parent, 
                                wxWindowID id = SYMBOL_DLGVSCPINTERFACESETTINGS_IDNAME, 
                                const wxString& caption = SYMBOL_DLGVSCPINTERFACESETTINGS_TITLE, 
                                const wxPoint& pos = SYMBOL_DLGVSCPINTERFACESETTINGS_POSITION, 
                                const wxSize& size = SYMBOL_DLGVSCPINTERFACESETTINGS_SIZE, 
                                long style = SYMBOL_DLGVSCPINTERFACESETTINGS_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGVSCPINTERFACESETTINGS_IDNAME, 
                    const wxString& caption = SYMBOL_DLGVSCPINTERFACESETTINGS_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGVSCPINTERFACESETTINGS_POSITION, 
                    const wxSize& size = SYMBOL_DLGVSCPINTERFACESETTINGS_SIZE, 
                    long style = SYMBOL_DLGVSCPINTERFACESETTINGS_STYLE );

  /// Destructor
  ~dlgVscpInterfaceSettings();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgVscpInterfaceSettings event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_VSCP_DRIVER_SET_PATH
  void OnButtonVscpDriverSetPathClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_VSCP_SET_CONFIGURATION
  void OnButtonVscpSetConfigurationClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_FILTER
  void OnButtonSetFilterClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_TEST_INTERFACE
  void OnButtonTestInterfaceClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
  void OnButtonGetInterfacesClick( wxCommandEvent& event );

////@end dlgVscpInterfaceSettings event handler declarations

////@begin dlgVscpInterfaceSettings member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgVscpInterfaceSettings member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();
	
	/// VSCP Filter
	vscpEventFilter m_vscpfilter;

////@begin dlgVscpInterfaceSettings member variables
  wxPanel* m_panelCanal;
  wxTextCtrl* m_DriverDescription;
  wxTextCtrl* m_PathToDriver;
  wxTextCtrl* m_DriverConfigurationString;
  wxTextCtrl* m_DriverFlags;
  wxPanel* m_panelServer;
  wxTextCtrl* m_RemoteServerDescription;
  wxTextCtrl* m_RemoteServerURL;
  wxTextCtrl* m_RemoteServerUsername;
  wxTextCtrl* m_RemoteServerPassword;
  wxCheckBox* m_fullLevel2;
  wxTextCtrl* m_RemoteInterfaceName;
  wxButton* m_btnTestConnection;
  wxButton* m_btnGetInterfaces;
  /// Control identifiers
  enum {
    ID_DLGVSCPINTERFACESETTINGS = 16000,
    ID_PANEL_INTERFACE_DRIVER = 16001,
    ID_DriverDescription = 16002,
    ID_PathToDriver = 16003,
    ID_BUTTON_VSCP_DRIVER_SET_PATH = 16004,
    ID_DriverConfigurationString = 16005,
    ID_BUTTON_VSCP_SET_CONFIGURATION = 16006,
    ID_DriverFlags = 16007,
    ID_PANEL_INTERFACE_REMOTE = 16024,
    ID_TEXTCTRL = 16025,
    ID_RemoteServerURL = 16026,
    ID_RemoteServerUsername = 16028,
    ID_RemoteServerPassword = 16029,
    ID_CHECKBOX2 = 16009,
    ID_TEXTCTRL_INTERFACE_NAME = 16019,
    ID_BUTTON_SET_FILTER = 16017,
    ID_BUTTON_TEST_INTERFACE = 16016,
    ID_BUTTON = 16018
  };
////@end dlgVscpInterfaceSettings member variables
};

#endif
  // _DLGVSCPINTERFACESETTINGS_H_
