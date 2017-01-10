/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanalmsg.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 14:13:52 CEST
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

#ifndef _DLGCANALMSG_H_
#define _DLGCANALMSG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgcanalmsg.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgcanalmsg_symbols.h"
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
#define SYMBOL_DLGCANALMSG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_DLGCANALMSG_TITLE _("CANAL Message")
#define SYMBOL_DLGCANALMSG_IDNAME ID_DIALOG_CANAL_MSG
#define SYMBOL_DLGCANALMSG_SIZE wxSize(400, 300)
#define SYMBOL_DLGCANALMSG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgCanalMsg class declaration
 */

class dlgCanalMsg: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgCanalMsg )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgCanalMsg();
  dlgCanalMsg( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGCANALMSG_IDNAME, 
                    const wxString& caption = SYMBOL_DLGCANALMSG_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGCANALMSG_POSITION, 
                    const wxSize& size = SYMBOL_DLGCANALMSG_SIZE, 
                    long style = SYMBOL_DLGCANALMSG_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGCANALMSG_IDNAME, 
                    const wxString& caption = SYMBOL_DLGCANALMSG_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGCANALMSG_POSITION, 
                    const wxSize& size = SYMBOL_DLGCANALMSG_SIZE, 
                    long style = SYMBOL_DLGCANALMSG_STYLE );

  /// Destructor
  ~dlgCanalMsg();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgCanalMsg event handler declarations

////@end dlgCanalMsg event handler declarations

////@begin dlgCanalMsg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgCanalMsg member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin dlgCanalMsg member variables
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
  /// Control identifiers
  enum {
    ID_DIALOG_CANAL_MSG = 11000,
    ID_CtrlOptStandardId = 11001,
    ID_CtrlOptExtendedId = 11002,
    ID_CtrlEditId = 11003,
    ID_CtrlChkRtr = 11004,
    ID_CtrlEditData1 = 11005,
    ID_CtrlEditData2 = 11006,
    ID_CtrlEditData3 = 11007,
    ID_CtrlEditData4 = 11008,
    ID_CtrlEditData5 = 11009,
    ID_CtrlEditData6 = 11010,
    ID_CtrlEditData7 = 11011,
    ID_CtrlEditData8 = 11012
  };
////@end dlgCanalMsg member variables
};

#endif
  // _DLGCANALMSG_H_
