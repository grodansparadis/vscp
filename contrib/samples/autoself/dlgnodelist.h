/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnodelist.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 13:50:12 CEST
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

#ifndef _DLGNODELIST_H_
#define _DLGNODELIST_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgnodelist.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgnodelist_symbols.h"
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
#define SYMBOL_DLGNODELIST_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGNODELIST_TITLE _("dlgNodeList")
#define SYMBOL_DLGNODELIST_IDNAME ID_DLGNODELIST
#define SYMBOL_DLGNODELIST_SIZE wxSize(400, 300)
#define SYMBOL_DLGNODELIST_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgNodeList class declaration
 */

class dlgNodeList: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgNodeList )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgNodeList();
    dlgNodeList( wxWindow* parent, wxWindowID id = SYMBOL_DLGNODELIST_IDNAME, const wxString& caption = SYMBOL_DLGNODELIST_TITLE, const wxPoint& pos = SYMBOL_DLGNODELIST_POSITION, const wxSize& size = SYMBOL_DLGNODELIST_SIZE, long style = SYMBOL_DLGNODELIST_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGNODELIST_IDNAME, const wxString& caption = SYMBOL_DLGNODELIST_TITLE, const wxPoint& pos = SYMBOL_DLGNODELIST_POSITION, const wxSize& size = SYMBOL_DLGNODELIST_SIZE, long style = SYMBOL_DLGNODELIST_STYLE );

    /// Destructor
    ~dlgNodeList();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgNodeList event handler declarations

////@end dlgNodeList event handler declarations

////@begin dlgNodeList member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end dlgNodeList member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgNodeList member variables
    /// Control identifiers
    enum {
        ID_DLGNODELIST = 10021,
        ID_LISTBOX1 = 10022,
        ID_BUTTON = 10001,
        ID_BUTTON1 = 10000,
        ID_BUTTON2 = 10002
    };
////@end dlgNodeList member variables
};

#endif
    // _DLGNODELIST_H_
