/////////////////////////////////////////////////////////////////////////////
// Name:        frmdaemonvariableeditor.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/10/2012 14:36:40
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
//  Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
// 

#ifndef _FRMDAEMONVARIABLEEDITOR_H_
#define _FRMDAEMONVARIABLEEDITOR_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmdaemonvariableeditor.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/toolbook.h"
#include "wx/grid.h"
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxToolbook;
class wxGrid;
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DAEMONVARIABLEEDITOR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DAEMONVARIABLEEDITOR_TITLE _("Form Daemon Variable Editor")
#define SYMBOL_DAEMONVARIABLEEDITOR_IDNAME ID_DAEMONVARIABLEEDITOR
#define SYMBOL_DAEMONVARIABLEEDITOR_SIZE wxSize(400, 300)
#define SYMBOL_DAEMONVARIABLEEDITOR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * DaemonVariableEditor class declaration
 */

class DaemonVariableEditor: public wxFrame
{    
  DECLARE_CLASS( DaemonVariableEditor )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  DaemonVariableEditor();
  DaemonVariableEditor( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DAEMONVARIABLEEDITOR_IDNAME, 
                            const wxString& caption = SYMBOL_DAEMONVARIABLEEDITOR_TITLE, 
                            const wxPoint& pos = SYMBOL_DAEMONVARIABLEEDITOR_POSITION, 
                            const wxSize& size = SYMBOL_DAEMONVARIABLEEDITOR_SIZE, 
                            long style = SYMBOL_DAEMONVARIABLEEDITOR_STYLE );

  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DAEMONVARIABLEEDITOR_IDNAME, 
                    const wxString& caption = SYMBOL_DAEMONVARIABLEEDITOR_TITLE, 
                    const wxPoint& pos = SYMBOL_DAEMONVARIABLEEDITOR_POSITION, 
                    const wxSize& size = SYMBOL_DAEMONVARIABLEEDITOR_SIZE, 
                    long style = SYMBOL_DAEMONVARIABLEEDITOR_STYLE );

  /// Destructor
  ~DaemonVariableEditor();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin DaemonVariableEditor event handler declarations

  /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_REGISTERS
  void OnCellRightClick( wxGridEvent& event );

  /// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_GRID_REGISTERS
  void OnLeftDClick( wxGridEvent& event );

////@end DaemonVariableEditor event handler declarations

////@begin DaemonVariableEditor member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end DaemonVariableEditor member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin DaemonVariableEditor member variables
  wxToolbook* m_choiceBook;
  wxPanel* m_panel0;
  wxGrid* m_gridRegisters;
  wxGrid* m_gridAbstractions;
  wxGrid* m_gridDM;
  wxHtmlWindow* m_StatusWnd;
  wxCheckBox* m_chkFullUppdate;
  wxCheckBox* m_chkMdfFromFile;
  wxButton* m_ctrlButtonLoadMDF;
  wxButton* m_ctrlButtonWizard;
  /// Control identifiers
  enum {
    ID_DAEMONVARIABLEEDITOR = 10149,
    ID_MENUITEM6 = 10000,
    ID_MENUITEM = 10001,
    ID_MENUITEM1 = 10002,
    ID_TOOLBAR1 = 10138,
    ID_PANEL2 = 10139,
    ID_CHOICEBOOK = 19040,
    ID_PANEL_REGISTERS = 19041,
    ID_GRID_REGISTERS = 19042,
    ID_PANEL_ABSTRACTIONS = 19107,
    ID_GRID_ABSTRACTIONS = 19108,
    ID_PANEL_DM = 19109,
    ID_GRID_DM = 19110,
    ID_HTMLWINDOW1 = 19111,
    ID_CHECKBOX_FULL_UPDATE = 19112,
    ID_CHECKBOX_MDF_FROM_FILE = 10000,
    ID_BUTTON16 = 19113,
    ID_BUTTON17 = 19114,
    ID_BUTTON19 = 19116
  };
////@end DaemonVariableEditor member variables
};

#endif
  // _FRMDAEMONVARIABLEEDITOR_H_
