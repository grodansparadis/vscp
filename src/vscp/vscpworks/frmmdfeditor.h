/////////////////////////////////////////////////////////////////////////////
// Name:        frmmdfeditor.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     09/04/2009 11:05:11
// RCS-ID:      
// Copyright:   (C) 2009-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
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

#ifndef _FRMMDFEDITOR_H_
#define _FRMMDFEDITOR_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmmdfeditor.h"
#endif

/*!
 * Includes
 */
#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/treectrl.h"
#include "wx/html/htmlwin.h"
#include <wx/imaglist.h>

#include <mdf.h>


 // Picture types for the tree control
enum {
    MDF_EDITOR_TOP_ITEM = 0,
    MDF_EDITOR_MAIN_ITEM,
    MDF_EDITOR_SUB_ITEM
};


/*!
 * Forward declarations
 */

class wxTreeCtrl;
class wxHtmlWindow;

/*!
 * Control identifiers
 */

#define SYMBOL_FRMMDFEDITOR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMMDFEDITOR_TITLE _("MDF Editor")
#define SYMBOL_FRMMDFEDITOR_IDNAME ID_FRMMDFEDITOR
#define SYMBOL_FRMMDFEDITOR_SIZE wxSize(500, 560)
#define SYMBOL_FRMMDFEDITOR_POSITION wxDefaultPosition


/*!
 * frmMDFEditor class declaration
 */

class frmMDFEditor : public wxFrame
{
    DECLARE_CLASS( frmMDFEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmMDFEditor();
    frmMDFEditor( wxWindow* parent, wxWindowID id = SYMBOL_FRMMDFEDITOR_IDNAME, const wxString& caption = SYMBOL_FRMMDFEDITOR_TITLE, const wxPoint& pos = SYMBOL_FRMMDFEDITOR_POSITION, const wxSize& size = SYMBOL_FRMMDFEDITOR_SIZE, long style = SYMBOL_FRMMDFEDITOR_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRMMDFEDITOR_IDNAME, const wxString& caption = SYMBOL_FRMMDFEDITOR_TITLE, const wxPoint& pos = SYMBOL_FRMMDFEDITOR_POSITION, const wxSize& size = SYMBOL_FRMMDFEDITOR_SIZE, long style = SYMBOL_FRMMDFEDITOR_STYLE );

    /// Destructor
    ~frmMDFEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /*!
        Add the default content to the dialog
    */
    void addDefaultContent( void );

      /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_EXIT
    void OnMenuitemExitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_NEW
    void OnToolNewClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_LOAD
    void OnToolLoadClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_SAVE
    void OnToolSaveClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_DOWNLOAD
    void OnToolDownloadClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_REMOVE_ITEM
    void OnToolRemoveItemClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
    void OnTreectrlSelChanged( wxTreeEvent& event );

    /// wxEVT_LEFT_DOWN event handler for ID_TREECTRL
    void OnLeftDown( wxMouseEvent& event );

    /// wxEVT_LEFT_DCLICK event handler for ID_TREECTRL
    void OnLeftDClick( wxMouseEvent& event );

    ////@end frmMDFEditor event handler declarations

    ////@begin frmMDFEditor member function declarations

      /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    ////@end frmMDFEditor member function declarations

      /// Should we show tooltips?
    static bool ShowToolTips();

    /// Root item of the list
    wxTreeItemId m_rootItem;

    /// Module item of the list
    wxTreeItemId m_moduleItem;

    /// Module description file functionality
    CMDF m_mdf;

    wxTreeCtrl* m_mdfTree;
    wxHtmlWindow* m_htmlInfoWnd;

    /// Control identifiers
    enum {
        ID_FRMMDFEDITOR = 10036,
        ID_MENUITEM_NEW = 10006,
        ID_MENUITEM_OPEN = 10039,
        ID_MENUITEM_SAVE = 10007,
        ID_MENUITEM_DOWNLOAD = 10008,
        ID_MENUITEM_UPLOAD = 10009,
        ID_MENUITEM_EXIT = 10010,
        ID_MENU_ITEMS = 10040,
        ID_MENUITEM_ADD = 10041,
        ID_MENUITEM_REMOVE = 10011,
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

};

#endif
// _FRMMDFEDITOR_H_
