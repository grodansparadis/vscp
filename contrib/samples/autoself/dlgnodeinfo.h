/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnodeinfo.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 12:52:34 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource <ake@eurosource.se>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (https://www.vscp.org) 
//
// Copyright (C) 2000-2009 Ake Hedman, eurosource, <ake@eurosource.se>
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

#ifndef _DLGNODEINFO_H_
#define _DLGNODEINFO_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgnodeinfo.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgnodeinfo_symbols.h"
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
#define SYMBOL_DLGNODEINFO_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGNODEINFO_TITLE _("Node information\n")
#define SYMBOL_DLGNODEINFO_IDNAME ID_DLGNODEINFO
#define SYMBOL_DLGNODEINFO_SIZE wxSize(400, 300)
#define SYMBOL_DLGNODEINFO_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgNodeInfo class declaration
 */

class dlgNodeInfo: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgNodeInfo )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgNodeInfo();
    dlgNodeInfo( wxWindow* parent, wxWindowID id = SYMBOL_DLGNODEINFO_IDNAME, const wxString& caption = SYMBOL_DLGNODEINFO_TITLE, const wxPoint& pos = SYMBOL_DLGNODEINFO_POSITION, const wxSize& size = SYMBOL_DLGNODEINFO_SIZE, long style = SYMBOL_DLGNODEINFO_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGNODEINFO_IDNAME, const wxString& caption = SYMBOL_DLGNODEINFO_TITLE, const wxPoint& pos = SYMBOL_DLGNODEINFO_POSITION, const wxSize& size = SYMBOL_DLGNODEINFO_SIZE, long style = SYMBOL_DLGNODEINFO_STYLE );

    /// Destructor
    ~dlgNodeInfo();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgNodeInfo event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOkClick( wxCommandEvent& event );

////@end dlgNodeInfo event handler declarations

////@begin dlgNodeInfo member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgNodeInfo member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgNodeInfo member variables
  wxTextCtrl* m_ctrlEditGUID15;
  wxTextCtrl* m_ctrlEditGUID14;
  wxTextCtrl* m_ctrlEditGUID13;
  wxTextCtrl* m_ctrlEditGUID12;
  wxTextCtrl* m_ctrlEditGUID11;
  wxTextCtrl* m_ctrlEditGUID10;
  wxTextCtrl* m_ctrlEditGUID9;
  wxTextCtrl* m_ctrlEditGUID8;
  wxTextCtrl* m_ctrlEditGUID7;
  wxTextCtrl* m_ctrlEditGUID6;
  wxTextCtrl* m_ctrlEditGUID5;
  wxTextCtrl* m_ctrlEditGUID4;
  wxTextCtrl* m_ctrlEditGUID3;
  wxTextCtrl* m_ctrlEditGUID2;
  wxTextCtrl* m_ctrlEditGUID1;
  wxTextCtrl* m_ctrlEditGUID0;
  wxTextCtrl* m_ctrlEditRealName;
  wxTextCtrl* m_ctrlEditHostname;
  wxTextCtrl* m_ctrlEditZone;
  /// Control identifiers
  enum {
    ID_DLGNODEINFO = 10020,
    ID_TEXTCTRL_GUID15 = 10015,
    ID_TEXTCTRL_GUID14  = 10016,
    ID_TEXTCTRL_GUID13  = 10017,
    ID_TEXTCTRL_GUID12  = 10018,
    ID_TEXTCTRL_GUID11  = 10019,
    ID_TEXTCTRL_GUID10  = 10023,
    ID_TEXTCTRL_GUID9  = 10024,
    ID_TEXTCTRL_GUID8  = 10025,
    ID_TEXTCTRL_GUID7  = 10026,
    ID_TEXTCTRL_GUID6  = 10027,
    ID_TEXTCTRL_GUID5  = 10028,
    ID_TEXTCTRL_GUID4 = 10029,
    ID_TEXTCTRL_GUID3  = 10030,
    ID_TEXTCTRL_GUID2  = 10031,
    ID_TEXTCTRL_GUID1  = 10032,
    ID_TEXTCTRL_GUID0  = 10033,
    ID_TEXTCTRL_REALNAME = 10034,
    ID_TEXTCTRL_HOSTNAME = 10001,
    ID_TEXTCTRL_ZONE = 17000
  };
////@end dlgNodeInfo member variables
};

#endif
    // _DLGNODEINFO_H_
