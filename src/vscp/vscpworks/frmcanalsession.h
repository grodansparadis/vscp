/////////////////////////////////////////////////////////////////////////////
// Name:        frmcanalsession.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 16 Apr 2007 19:23:48 CEST
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

#ifndef _FRMCANALSESSIONNOMIDI_H_
#define _FRMCANALSESSIONNOMIDI_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmcanalsessionnomidi.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "frmcanalsession_symbols.h"
#include "wx/frame.h"
#include "wx/grid.h"
#include "wx/toolbar.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FRMCANALSESSION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMCANALSESSION_TITLE _("CANAL Session")
#define SYMBOL_FRMCANALSESSION_IDNAME ID_FRMCANALSESSION
#define SYMBOL_FRMCANALSESSION_SIZE wxSize(850, 490)
#define SYMBOL_FRMCANALSESSION_POSITION wxPoint(100, 100)
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * frmCANALSession class declaration
 */

class frmCANALSession: public wxFrame
{    
    DECLARE_CLASS( frmCANALSession )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmCANALSession();
    frmCANALSession( wxWindow* parent, 
                        wxWindowID id = SYMBOL_FRMCANALSESSION_IDNAME, 
                        const wxString& caption = SYMBOL_FRMCANALSESSION_TITLE, 
                        const wxPoint& pos = SYMBOL_FRMCANALSESSION_POSITION, 
                        const wxSize& size = SYMBOL_FRMCANALSESSION_SIZE, 
                        long style = SYMBOL_FRMCANALSESSION_STYLE );

    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_FRMCANALSESSION_IDNAME, 
                    const wxString& caption = SYMBOL_FRMCANALSESSION_TITLE, 
                    const wxPoint& pos = SYMBOL_FRMCANALSESSION_POSITION, 
                    const wxSize& size = SYMBOL_FRMCANALSESSION_SIZE, 
                    long style = SYMBOL_FRMCANALSESSION_STYLE );

    /// Destructor
    ~frmCANALSession();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin frmCANALSession event handler declarations

  /// wxEVT_CLOSE_WINDOW event handler for ID_FRMCANALSESSION
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_RECEIVE
  void OnGridCellRightClick( wxGridEvent& event );

  /// wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID_RECEIVE
  void OnGridLabelLeftDClick( wxGridEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_MSG_LIST
  void OnMenuitemCanalLoadMsgListClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_MSG_LIST
  void OnMenuitemCanalSaveMsgListClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET
  void OnMenuitemCanalLoadTransmissionSetClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET
  void OnMenuitemCanalSaveTransmissionSetClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_EXIT
  void OnMenuitemCanalExittClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_ABOUT
  void OnMenuitemCanalAboutClick( wxCommandEvent& event );

////@end frmCANALSession event handler declarations

////@begin frmCANALSession member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmCANALSession member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    
    
  /// TRUE as long as worker thread should run
  bool m_bRun;




////@begin frmCANALSession member variables
  wxGrid* m_ctrlGridReceive;
  wxBitmapButton* m_btnEdit;
  wxBitmapButton* m_btnDelete;
  wxBitmapButton* m_btnLoadSet;
  wxBitmapButton* m_btnSaveSet;
  wxGrid* m_ctrlGridTransmission;
  wxBitmapButton* m_btnSend;
  wxBitmapButton* m_btnActivate;
  wxBitmapButton* m_btnClear;
  wxComboBox* m_comboSelectView;
  wxBitmapButton* m_ctrlBtnActivateInterface;
  /// Control identifiers
  enum {
    ID_FRMCANALSESSION = 17000,
    ID_GRID_RECEIVE = 17001,
    ID_BITMAPBUTTON_CANAL_EDIT = 17002,
    ID_BITMAPBUTTONID_MENUITEM_CANAL_DELETE = 17003,
    ID_BITMAPBUTTONID_MENUITEM_CANAL_LOAD = 17004,
    ID_BITMAPBUTTONID_MENUITEM_CANAL_SAVE = 17005,
    ID_GRID_TRANSMISSION = 17006,
    ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND = 17007,
    ID_BITMAPBUTTON12 = 17008,
    ID_BITMAPBUTTON13 = 17009,
    ID_TOOLBAR_CANAL = 17010,
    ID_TOOL_CANAL_OPEN = 17011,
    ID_TOOL_CANAL_SAVE = 17012,
    ID_TOOL_CANAL_CUT = 17013,
    ID_TOOL_CANAL_COPY = 17014,
    ID_TOOL_CANAL_PASTE = 17015,
    ID_TOOL_CANAL_PREVIEW = 17016,
    ID_TOOL_CANAL_PRINT = 17017,
    ID_COMBOBOX = 17018,
    ID_BITMAPBUTTON = 17019,
    ID_MENUITEM_CANAL_LOAD_MSG_LIST = 17021,
    ID_MENUITEM_CANAL_SAVE_MSG_LIST = 17022,
    ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET = 17023,
    ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET = 17024,
    ID_MENUITEM_CANAL_EXIT = 17025,
    ID_MENUITEM_CANAL_CUT = 17026,
    ID_MENUITEM_CANAL_COPY = 17027,
    ID_MENUITEM_CANAL_PASTE = 17028,
    ID_MENUITEM_CANAL_LOG = 17029,
    ID_MENUITEM_CANAL_COUNT = 17030,
    ID_MENUITEM_CANAL_AUTO_REPLY = 17031,
    ID_MENUITEM_CANAL_BURST_COUNT = 17032,
    ID_MENUITEM_CANAL_HELP = 17033,
    ID_MENUITEM_CANAL_FAQ = 17034,
    ID_MENUITEM_CANAL_SHORTCUTS = 17035,
    ID_MENUITEM_CANAL_THANKS = 17036,
    ID_MENUITEM_CANAL_CREDITS = 17037,
    ID_MENUITEM_CANAL_VSCP_SITE = 17038,
    ID_MENUITEM_CANAL_ABOUT = 17039
  };
////@end frmCANALSession member variables
};

#endif
    // _FRMCANALSESSIONNOMIDI_H_
