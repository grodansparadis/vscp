/////////////////////////////////////////////////////////////////////////////
// Name:        dlgabout.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     18/05/2009 17:14:13
// RCS-ID:      
// Copyright:   (C) 2009-2018 
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

#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgabout.h"
#endif

/*!
 * Includes
 */

#include "dlgabout_symbols.h"
#include "wx/html/htmlwin.h"

/*!
 * Forward declarations
 */



/*!
 * Control identifiers
 */

#define SYMBOL_DLGABOUT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DLGABOUT_TITLE _("About VSCP Works...")
#define SYMBOL_DLGABOUT_IDNAME ID_DLGABOUT
#define SYMBOL_DLGABOUT_SIZE wxSize(400, 500)
#define SYMBOL_DLGABOUT_POSITION wxDefaultPosition


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
  dlgAbout( wxWindow* parent, 
                wxWindowID id = SYMBOL_DLGABOUT_IDNAME, 
                const wxString& caption = SYMBOL_DLGABOUT_TITLE, 
                const wxPoint& pos = SYMBOL_DLGABOUT_POSITION, 
                const wxSize& size = SYMBOL_DLGABOUT_SIZE, 
                long style = SYMBOL_DLGABOUT_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_DLGABOUT_IDNAME, 
                const wxString& caption = SYMBOL_DLGABOUT_TITLE, 
                const wxPoint& pos = SYMBOL_DLGABOUT_POSITION, 
                const wxSize& size = SYMBOL_DLGABOUT_SIZE, 
                long style = SYMBOL_DLGABOUT_STYLE );

  /// Destructor
  ~dlgAbout();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );

  /// Should we show tooltips?
  static bool ShowToolTips();

  /// Control identifiers
  enum {
    ID_DLGABOUT = 26000,
    ID_HTMLWINDOW = 26001
  };

};

#endif
  // _DLGABOUT_H_
