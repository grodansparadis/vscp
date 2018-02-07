/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpmsg.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:46:25
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
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGVSCPMSG_H_
#define _DLGVSCPMSG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgvscpmsg.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgvscpmsg_symbols.h"
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
#define SYMBOL_DLGVSCPMSG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGVSCPMSG_TITLE _("VSCP Event")
#define SYMBOL_DLGVSCPMSG_IDNAME ID_DLGVSCPMSG
#define SYMBOL_DLGVSCPMSG_SIZE wxSize(400, 300)
#define SYMBOL_DLGVSCPMSG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgVSCPMsg class declaration
 */

class dlgVSCPMsg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgVSCPMsg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgVSCPMsg();
    dlgVSCPMsg( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGVSCPMSG_IDNAME, 
                    const wxString& caption = SYMBOL_DLGVSCPMSG_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGVSCPMSG_POSITION, 
                    const wxSize& size = SYMBOL_DLGVSCPMSG_SIZE, 
                    long style = SYMBOL_DLGVSCPMSG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGVSCPMSG_IDNAME, 
                    const wxString& caption = SYMBOL_DLGVSCPMSG_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGVSCPMSG_POSITION, 
                    const wxSize& size = SYMBOL_DLGVSCPMSG_SIZE, 
                    long style = SYMBOL_DLGVSCPMSG_STYLE );

    /// Destructor
    ~dlgVSCPMsg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgVSCPMsg event handler declarations

  /// wxEVT_INIT_DIALOG event handler for ID_DLGVSCPMSG
  void OnInitDialog( wxInitDialogEvent& event );

  /// wxEVT_CLOSE_WINDOW event handler for ID_DLGVSCPMSG
  void OnCloseWindow( wxCloseEvent& event );

////@end dlgVSCPMsg event handler declarations

////@begin dlgVSCPMsg member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgVSCPMsg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgVSCPMsg member variables
  wxComboBox* m_choiceVscpClass;
  wxComboBox* m_wxChoiceVscpType;
  wxChoice* m_wxChoicePriority;
  wxTextCtrl* m_strGUID15;
  wxTextCtrl* m_wxStrGUID14;
  wxTextCtrl* m_wxStrGUID13;
  wxTextCtrl* m_wxStrGUID12;
  wxTextCtrl* m_wxStrGUID11;
  wxTextCtrl* m_wxStrGUID10;
  wxTextCtrl* m_wxStrGUID9;
  wxTextCtrl* m_wxStrGUID8;
  wxTextCtrl* m_wxStrGUID7;
  wxTextCtrl* m_wxStrGUID6;
  wxTextCtrl* m_wxStrGUID5;
  wxTextCtrl* m_wxStrGUID4;
  wxTextCtrl* m_wxStrGUID3;
  wxTextCtrl* m_wxStrGUID2;
  wxTextCtrl* m_wxStrGUID1;
  wxTextCtrl* m_wxStrGUID0;
  wxTextCtrl* m_strVscpData;
  wxTextCtrl* m_strNote;
  /// Control identifiers
  enum {
    ID_DLGVSCPMSG = 10000,
    ID_COMBOBOX2 = 10001,
    ID_COMBOBOX3 = 10002,
    ID_CHOICE = 10003,
    ID_TEXTCTRL22 = 10004,
    ID_TEXTCTRL23 = 10005,
    ID_TEXTCTRL24 = 10006,
    ID_TEXTCTRL25 = 10007,
    ID_TEXTCTRL26 = 10008,
    ID_TEXTCTRL27 = 10009,
    ID_TEXTCTRL28 = 10010,
    ID_TEXTCTRL29 = 10011,
    ID_TEXTCTRL30 = 10012,
    ID_TEXTCTRL31 = 10013,
    ID_TEXTCTRL32 = 10014,
    ID_TEXTCTRL33 = 10015,
    ID_TEXTCTRL34 = 10016,
    ID_TEXTCTRL35 = 10017,
    ID_TEXTCTRL36 = 10018,
    ID_TEXTCTRL37 = 10019,
    ID_TEXTCTRL38 = 10020,
    ID_TEXTCTRL39 = 10021
  };
////@end dlgVSCPMsg member variables
};

#endif
    // _DLGVSCPMSG_H_
