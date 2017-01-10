/////////////////////////////////////////////////////////////////////////////
// Name:        frmcansession.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Tue 17 Apr 2007 13:38:09 CEST
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

#ifndef _FRMCANSESSION_H_
#define _FRMCANSESSION_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmcansession.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "frmcansession_symbols.h"
#include "wx/mdi.h"
#include "wx/grid.h"
#include "wx/toolbar.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_FRMCANSESSION 10055
#define ID_GRID 10003
#define ID_BITMAPBUTTON 10009
#define ID_BITMAPBUTTON1 10010
#define ID_BITMAPBUTTON2 10011
#define ID_BITMAPBUTTON3 10012
#define ID_GRID1 10013
#define ID_BITMAPBUTTON4 10014
#define ID_BITMAPBUTTON5 10015
#define ID_BITMAPBUTTON6 10016
#define ID_TOOLBAR 10000
#define ID_TOOL 10001
#define ID_TOOL1 10002
#define ID_TOOL2 10004
#define ID_TOOL3 10005
#define ID_TOOL4 10006
#define ID_TOOL5 10007
#define ID_TOOL6 10008
#define ID_COMBOBOX 10017
#define ID_BITMAPBUTTON7 10018
#define SYMBOL_FRMCANSESSION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMCANSESSION_TITLE _("frmCANSession")
#define SYMBOL_FRMCANSESSION_IDNAME ID_FRMCANSESSION
#define SYMBOL_FRMCANSESSION_SIZE wxSize(850, 500)
#define SYMBOL_FRMCANSESSION_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * frmCANSession class declaration
 */

class frmCANSession: public wxMDIChildFrame
{    
    DECLARE_CLASS( frmCANSession )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmCANSession();
    frmCANSession( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMCANSESSION_IDNAME, const wxString& caption = SYMBOL_FRMCANSESSION_TITLE, const wxPoint& pos = SYMBOL_FRMCANSESSION_POSITION, const wxSize& size = SYMBOL_FRMCANSESSION_SIZE, long style = SYMBOL_FRMCANSESSION_STYLE );

    bool Create( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMCANSESSION_IDNAME, const wxString& caption = SYMBOL_FRMCANSESSION_TITLE, const wxPoint& pos = SYMBOL_FRMCANSESSION_POSITION, const wxSize& size = SYMBOL_FRMCANSESSION_SIZE, long style = SYMBOL_FRMCANSESSION_STYLE );

    /// Destructor
    ~frmCANSession();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin frmCANSession event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_FRMCANSESSION
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID
    void OnGridLogCellRightClick( wxGridEvent& event );

    /// wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID
    void OnGridLogLabelLeftDClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID1
    void OnGridSendCellRightClick( wxGridEvent& event );

    /// wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID1
    void OnGridSendLabelLeftDClick( wxGridEvent& event );

////@end frmCANSession event handler declarations

////@begin frmCANSession member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end frmCANSession member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmCANSession member variables
    wxGrid* m_ctrlGrid;
    wxBitmapButton* m_btnEdit;
    wxBitmapButton* m_btnDelete;
    wxBitmapButton* m_btnLoad;
    wxBitmapButton* m_btnSave;
    wxGrid* m_ctrlGridSend;
    wxBitmapButton* m_btnSend;
    wxBitmapButton* m_btnActivate;
    wxBitmapButton* m_btnClear;
#if defined(__WXMSW__)
    wxComboBox* m_comboSelectView;
#endif
#if defined(__WXMSW__)
    wxBitmapButton* m_ctrlBtnActivateInterface;
#endif
////@end frmCANSession member variables
};

#endif
    // _FRMCANSESSION_H_
