/////////////////////////////////////////////////////////////////////////////
// Name:        dlgCanalRowEdit.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 14:14:50 CEST
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

#ifndef _DLGCANALROWEDIT_H_
#define _DLGCANALROWEDIT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgCanalRowEdit.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgCanalRowEdit_symbols.h"
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
#define SYMBOL_DLGCANALROWEDIT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DLGCANALROWEDIT_TITLE _("CANAL Message")
#define SYMBOL_DLGCANALROWEDIT_IDNAME ID_DIALOG_CANAL_ROW
#define SYMBOL_DLGCANALROWEDIT_SIZE wxSize(400, 300)
#define SYMBOL_DLGCANALROWEDIT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgCanalRowEdit class declaration
 */

class dlgCanalRowEdit: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgCanalRowEdit )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgCanalRowEdit();
  dlgCanalRowEdit( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANALROWEDIT_IDNAME, const wxString& caption = SYMBOL_DLGCANALROWEDIT_TITLE, const wxPoint& pos = SYMBOL_DLGCANALROWEDIT_POSITION, const wxSize& size = SYMBOL_DLGCANALROWEDIT_SIZE, long style = SYMBOL_DLGCANALROWEDIT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANALROWEDIT_IDNAME, const wxString& caption = SYMBOL_DLGCANALROWEDIT_TITLE, const wxPoint& pos = SYMBOL_DLGCANALROWEDIT_POSITION, const wxSize& size = SYMBOL_DLGCANALROWEDIT_SIZE, long style = SYMBOL_DLGCANALROWEDIT_STYLE );

  /// Destructor
  ~dlgCanalRowEdit();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgCanalRowEdit event handler declarations

////@end dlgCanalRowEdit event handler declarations

////@begin dlgCanalRowEdit member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgCanalRowEdit member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgCanalRowEdit member variables
  wxRadioButton* m_ctrlOptStandardId;
  wxRadioButton* m_ctrlOptExtendedId;
  wxTextCtrl* m_ctrlEditId;
  wxCheckBox* m_ctrlChkRtr;
  wxTextCtrl* m_ctrlEditData1;
  wxTextCtrl* m_ctrlEditData2;
  wxTextCtrl* m_ctrlEditData3;
  wxTextCtrl* m_ctrlEditData4;
  wxTextCtrl* m_ctrlEditData5;
  wxTextCtrl* m_ctrlEditData6;
  wxTextCtrl* m_ctrlEditData7;
  wxTextCtrl* m_ctrlEditData8;
  wxTextCtrl* m_ctrlEditRoeDescription;
  wxTextCtrl* m_ctrlEditRoeInterval;
  /// Control identifiers
  enum {
    ID_DIALOG_CANAL_ROW = 12000,
    ID_RADIOBUTTON_STANDARD_ID = 12001,
    ID_RADIOBUTTON_EXTENDED_ID = 12002,
    ID_TEXTCTRL_ID = 12003,
    ID_CHECKBOX_RTR = 12004,
    ID_TEXTCTRL_DATA1 = 12005,
    ID_TEXTCTRL_DATA2 = 12006,
    ID_TEXTCTRL_DATA3 = 12007,
    ID_TEXTCTRL_DATA4 = 12009,
    ID_TEXTCTRL_DATA5 = 12009,
    ID_TEXTCTRL_DATA6 = 12010,
    ID_TEXTCTRL_DATA7 = 12011,
    ID_TEXTCTRL_DATA8 = 12012,
    ID_TEXTCTRL1 = 12013,
    ID_TEXTCTRL = 12014
  };
////@end dlgCanalRowEdit member variables
};

#endif
  // _DLGCANALROWEDIT_H_
