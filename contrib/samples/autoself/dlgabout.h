/////////////////////////////////////////////////////////////////////////////
// Name:        dlgabout.h
// Purpose:     
// Author:       Ake Hedman
// Modified by: 
// Created:     Fri 27 Apr 2007 11:56:55 CEST
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
// Copyright (C) 2000-2025 Ake Hedman, eurosource, <ake@eurosource.se>
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

#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgabout.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgabout_symbols.h"
#include "wx/html/htmlwin.h"
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
#define SYMBOL_DLGABOUT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGABOUT_TITLE _("About this program")
#define SYMBOL_DLGABOUT_IDNAME ID_DLGABOUT
#define SYMBOL_DLGABOUT_SIZE wxSize(400, 300)
#define SYMBOL_DLGABOUT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgAbout class declaration
 */

class dlgAbout: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgAbout )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgAbout();
    dlgAbout( wxWindow* parent, wxWindowID id = SYMBOL_DLGABOUT_IDNAME, const wxString& caption = SYMBOL_DLGABOUT_TITLE, const wxPoint& pos = SYMBOL_DLGABOUT_POSITION, const wxSize& size = SYMBOL_DLGABOUT_SIZE, long style = SYMBOL_DLGABOUT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGABOUT_IDNAME, const wxString& caption = SYMBOL_DLGABOUT_TITLE, const wxPoint& pos = SYMBOL_DLGABOUT_POSITION, const wxSize& size = SYMBOL_DLGABOUT_SIZE, long style = SYMBOL_DLGABOUT_STYLE );

    /// Destructor
    ~dlgAbout();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgAbout event handler declarations

////@end dlgAbout event handler declarations

////@begin dlgAbout member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end dlgAbout member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgAbout member variables
    /// Control identifiers
    enum {
        ID_DLGABOUT = 10013,
        ID_HTMLWINDOW_ABOUT = 10019
    };
////@end dlgAbout member variables
};

#endif
    // _DLGABOUT_H_
