/////////////////////////////////////////////////////////////////////////////
// Name:        frmmain.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 16 Apr 2007 18:19:49 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
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

#ifndef _FRMMAINNOMIDI_H_
#define _FRMMAINNOMIDI_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmmainnomidi.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/statusbr.h"
#include "wx/toolbar.h"
////@end includes

#include "../common/canal.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FRMMAIN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMMAIN_TITLE _("VSCP-Works")
#define SYMBOL_FRMMAIN_IDNAME ID_FRMMAIN
#define SYMBOL_FRMMAIN_SIZE wxSize(600, 310)
#define SYMBOL_FRMMAIN_POSITION wxPoint(1, 1)
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


/*!
 * frmMain class declaration
 */

class frmMain: public wxFrame
{    
    DECLARE_CLASS( frmMain )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmMain();
    frmMain( wxWindow* parent, wxWindowID id = SYMBOL_FRMMAIN_IDNAME, const wxString& caption = SYMBOL_FRMMAIN_TITLE, const wxPoint& pos = SYMBOL_FRMMAIN_POSITION, const wxSize& size = SYMBOL_FRMMAIN_SIZE, long style = SYMBOL_FRMMAIN_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRMMAIN_IDNAME, const wxString& caption = SYMBOL_FRMMAIN_TITLE, const wxPoint& pos = SYMBOL_FRMMAIN_POSITION, const wxSize& size = SYMBOL_FRMMAIN_SIZE, long style = SYMBOL_FRMMAIN_STYLE );

    /// Destructor
    ~frmMain();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin frmMain event handler declarations

  /// wxEVT_CLOSE_WINDOW event handler for ID_FRMMAIN
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_PAINT event handler for ID_FRMMAIN
  void OnPaint( wxPaintEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_OPEN_VSCP_SESSION
  void OnMenuitemOpenVscpSessionClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_DEVICE_CONFIGURATION
  void OnMenuitemOpenConfigSessionClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_MDF_EDITOR
  void OnMenuitemMdfEditorClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_DM_EDITOR
  void OnMenuitemOpenDaemonDMEditorClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VARIABLE_EDITOR
  void OnMenuitemOpenDaemonVariableEditorClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SCAN
  void OnMenuitemScanClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_BOOTLOADER_WIZARD
  void OnMenuitemBootloaderWizardClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SIMPLE_UI_DESIGNER
  void OnMenuitemOpenSimpleUIdesignerClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_MERLIN
  void OnMenuitemMerlinClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_VSCPWORKS_EXIT
  void OnMenuitemAppExitClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CONFIGURATION
  void OnMenuitemConfigurationClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM33
  void OnMenuitemHelpClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM34
  void OnMenuitemFaqClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM35
  void OnMenuitemShortcutsClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_THANKS
  void OnMenuitemThanksClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CREDITS
  void OnMenuitemCrediitsClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SITE
  void OnMenuitemVSCPSiteClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_ABOUT
  void OnMenuitemAboutClick( wxCommandEvent& event );

////@end frmMain event handler declarations

////@begin frmMain member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmMain member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmMain member variables
  /// Control identifiers
  enum {
    ID_FRMMAIN = 12000,
    ID_MENUITEM_OPEN_VSCP_SESSION = 12002,
    ID_MENUITEM_DEVICE_CONFIGURATION = 12003,
    ID_MENUITEM_MDF_EDITOR = 12006,
    ID_MENUITEM_DM_EDITOR = 10003,
    ID_MENUITEM_VARIABLE_EDITOR = 10004,
    ID_MENUITEM_SCAN = 12001,
    ID_MENUITEM_BOOTLOADER_WIZARD = 10000,
    ID_MENUITEM_SIMPLE_UI_DESIGNER = 10001,
    ID_MENUITEM_MERLIN = 10143,
    ID_MENU_VSCPWORKS_EXIT = 12005,
    ID_MENUITEM_CONFIGURATION = 12011,
    ID_MENUITEM33 = 12018,
    ID_MENUITEM34 = 12019,
    ID_MENUITEM35 = 12020,
    ID_MENUITEM_THANKS = 12021,
    ID_MENUITEM_CREDITS = 12022,
    ID_MENUITEM_VSCP_SITE = 12023,
    ID_MENUITEM_ABOUT = 12024,
    ID_STATUSBAR = 12025,
    ID_TOOLBAR = 12026,
    ID_TOOL = 12027,
    ID_PANEL1 = 10031
  };
////@end frmMain member variables
};

#endif
    // _FRMMAINNOMIDI_H_
