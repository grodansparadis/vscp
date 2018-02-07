/////////////////////////////////////////////////////////////////////////////
// Name:        dialogeditlevelidmrow.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     18/03/2009 23:10:00
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

#ifndef _DIALOGEDITLEVELIDMROW_H_
#define _DIALOGEDITLEVELIDMROW_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgeditlevel1dmrow.h"
#endif

/*!
 * Includes
 */

#include "dlgeditlevel1dmrow_symbols.h"

/*!
 * Control identifiers
 */

#define SYMBOL_DIALOGEDITLEVELIDMROW_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DIALOGEDITLEVELIDMROW_TITLE _("Dialog Edit Level I DM row")
#define SYMBOL_DIALOGEDITLEVELIDMROW_IDNAME ID_DIALOGEDITLEVELIDMROW
#define SYMBOL_DIALOGEDITLEVELIDMROW_SIZE wxSize(400, 300)
#define SYMBOL_DIALOGEDITLEVELIDMROW_POSITION wxDefaultPosition


/*!
 * DialogEditLevelIDMrow class declaration
 */

class DialogEditLevelIDMrow: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( DialogEditLevelIDMrow )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  DialogEditLevelIDMrow();
  DialogEditLevelIDMrow( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DIALOGEDITLEVELIDMROW_IDNAME, 
                            const wxString& caption = SYMBOL_DIALOGEDITLEVELIDMROW_TITLE, 
                            const wxPoint& pos = SYMBOL_DIALOGEDITLEVELIDMROW_POSITION, 
                            const wxSize& size = SYMBOL_DIALOGEDITLEVELIDMROW_SIZE, 
                            long style = SYMBOL_DIALOGEDITLEVELIDMROW_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DIALOGEDITLEVELIDMROW_IDNAME, 
                    const wxString& caption = SYMBOL_DIALOGEDITLEVELIDMROW_TITLE, 
                    const wxPoint& pos = SYMBOL_DIALOGEDITLEVELIDMROW_POSITION, 
                    const wxSize& size = SYMBOL_DIALOGEDITLEVELIDMROW_SIZE, 
                    long style = SYMBOL_DIALOGEDITLEVELIDMROW_STYLE );

  /// Destructor
  ~DialogEditLevelIDMrow();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SELECT_EVENTS
  void OnButtonSelectEventsClick( wxCommandEvent& event );

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );

  /// Should we show tooltips?
  static bool ShowToolTips();

  wxTextCtrl* m_oaddr;
  wxCheckBox* m_chkEnableDMRow;
  wxCheckBox* m_chkCheckOAddr;
  wxCheckBox* m_chkHardOAddr;
  wxCheckBox* m_chkMatchZone;
  wxCheckBox* m_chkMatchSubzone;
  wxTextCtrl* m_classMask;
  wxTextCtrl* m_classFilter;
  wxTextCtrl* m_typeMask;
  wxTextCtrl* m_typeFilter;
  wxComboBox* m_comboAction;
  wxTextCtrl* m_actionParam;
  /// Control identifiers
  enum {
    ID_DIALOGEDITLEVELIDMROW = 10022,
    ID_TEXTCTRL_ORIGINATING_ADDRESS = 10023,
    ID_CHECKBOX_ENABLE_ROW = 10024,
    ID_CHECKBOX_CHECK_OADDR = 10001,
    ID_CHECKBOX_HARDCODED_OADDR = 10002,
    ID_CHECKBOX_MATCH_ZONE = 10003,
    ID_CHECKBOX_MATCH_SUBZONE = 10004,
    ID_TEXTCTRL_CLASS_MASK = 10000,
    ID_BUTTON_SELECT_EVENTS = 10026,
    ID_TEXTCTRL_CLASS_FILTER = 10005,
    ID_TEXTCTRL_TYPE_MASK = 10006,
    ID_TEXTCTRL_TYPE_FILTER = 10007,
    ID_COMBOCTRL_ACTION = 10025,
    ID_TEXTCTRL_ACTION_PARAMETER = 10009
  };

};

#endif
  // _DIALOGEDITLEVELIDMROW_H_
