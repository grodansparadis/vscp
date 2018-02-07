/////////////////////////////////////////////////////////////////////////////
// Name:        frmsimpleuserinterfacegenerator.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/10/2012 14:35:37
// RCS-ID:      
// Copyright:   (C) 2012-2018 
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

#ifndef _FRMSIMPLEUSERINTERFACEGENERATOR_H_
#define _FRMSIMPLEUSERINTERFACEGENERATOR_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmsimpleuserinterfacegenerator.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/treectrl.h"
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxTreeCtrl;
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_SIMPLEUSERINTERFACEGENERATOR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_SIMPLEUSERINTERFACEGENERATOR_TITLE _("Simple User Interface Generator")
#define SYMBOL_SIMPLEUSERINTERFACEGENERATOR_IDNAME ID_SIMPLEUSERINTERFACEGENERATOR
#define SYMBOL_SIMPLEUSERINTERFACEGENERATOR_SIZE wxSize(500, 450)
#define SYMBOL_SIMPLEUSERINTERFACEGENERATOR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * SimpleUserInterfaceGenerator class declaration
 */

class SimpleUserInterfaceGenerator: public wxFrame
{    
  DECLARE_CLASS( SimpleUserInterfaceGenerator )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  SimpleUserInterfaceGenerator();
  SimpleUserInterfaceGenerator( wxWindow* parent, 
                                    wxWindowID id = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_IDNAME, 
                                    const wxString& caption = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_TITLE, 
                                    const wxPoint& pos = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_POSITION, 
                                    const wxSize& size = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_SIZE, 
                                    long style = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_STYLE );

  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_IDNAME, 
                    const wxString& caption = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_TITLE, 
                    const wxPoint& pos = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_POSITION, 
                    const wxSize& size = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_SIZE, 
                    long style = SYMBOL_SIMPLEUSERINTERFACEGENERATOR_STYLE );

  /// Destructor
  ~SimpleUserInterfaceGenerator();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin SimpleUserInterfaceGenerator event handler declarations

////@end SimpleUserInterfaceGenerator event handler declarations

////@begin SimpleUserInterfaceGenerator member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end SimpleUserInterfaceGenerator member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin SimpleUserInterfaceGenerator member variables
  wxTreeCtrl* m_mdfTree;
  wxHtmlWindow* m_htmlInfoWnd;
  /// Control identifiers
  enum {
    ID_SIMPLEUSERINTERFACEGENERATOR = 10148,
    ID_MENUITEM6 = 10000,
    ID_MENUITEM = 10001,
    ID_MENUITEM1 = 10002,
    ID_TOOLBAR1 = 10138,
    ID_PANEL = 10037,
    ID_TOOLBAR3 = 10034,
    ID_TOOL_NEW = 10035,
    ID_TOOL_LOAD = 10002,
    ID_TOOL_SAVE = 10001,
    ID_TOOL_UPLOAD = 10003,
    ID_TOOL_DOWNLOAD = 10000,
    ID_TOOL_ADD_ITEM = 10004,
    ID_TOOL_REMOVE_ITEM = 10005,
    ID_TREECTRL = 10032,
    ID_HTMLWINDOW2 = 10033
  };
////@end SimpleUserInterfaceGenerator member variables
};

#endif
  // _FRMSIMPLEUSERINTERFACEGENERATOR_H_
