/////////////////////////////////////////////////////////////////////////////
// Name:        dialogabstractionedit.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     26/05/2009 20:48:56
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
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//

#ifndef _DIALOGABSTRACTIONEDIT_H_
#define _DIALOGABSTRACTIONEDIT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgabstractionedit.h"
#endif

/*!
 * Includes
 */

#include "mdf.h"

////@begin includes
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
#define SYMBOL_DIALOGABSTRACTIONEDIT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DIALOGABSTRACTIONEDIT_TITLE _("Dialog Abstraction Item Edit")
#define SYMBOL_DIALOGABSTRACTIONEDIT_IDNAME ID_DIALOGABSTRACTIONEDIT
#define SYMBOL_DIALOGABSTRACTIONEDIT_SIZE wxSize( -1, -1 )
#define SYMBOL_DIALOGABSTRACTIONEDIT_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * DialogAbstractionEdit class declaration
 */

class DialogAbstractionEdit : public wxDialog {
    DECLARE_DYNAMIC_CLASS(DialogAbstractionEdit)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DialogAbstractionEdit();
    DialogAbstractionEdit(wxWindow* parent, wxWindowID id = SYMBOL_DIALOGABSTRACTIONEDIT_IDNAME, const wxString& caption = SYMBOL_DIALOGABSTRACTIONEDIT_TITLE, const wxPoint& pos = SYMBOL_DIALOGABSTRACTIONEDIT_POSITION, const wxSize& size = SYMBOL_DIALOGABSTRACTIONEDIT_SIZE, long style = SYMBOL_DIALOGABSTRACTIONEDIT_STYLE);

    /// Creation
    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_DIALOGABSTRACTIONEDIT_IDNAME, const wxString& caption = SYMBOL_DIALOGABSTRACTIONEDIT_TITLE, const wxPoint& pos = SYMBOL_DIALOGABSTRACTIONEDIT_POSITION, const wxSize& size = SYMBOL_DIALOGABSTRACTIONEDIT_SIZE, long style = SYMBOL_DIALOGABSTRACTIONEDIT_STYLE);

    /// Destructor
    ~DialogAbstractionEdit();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfers data to the window
    bool TransferDataToWindow(CMDF_Abstraction *pAbstraction, wxString &strValue);

    /// Transfers data from the window
    bool TransferDataFromWindow(wxString &strValue);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON18
    void OnButtonDefaultClick(wxCommandEvent& event);

    int GetTest() const
    {
        return m_nType;
    }

    void SetTest(int value)
    {
        m_nType = value;
    }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Pointer to the abstraction we are working with
    CMDF_Abstraction *m_pAbstraction;

    wxStaticText* m_abstractionName;
    wxStaticText* m_abstractionId;
    wxStaticText* m_abstractionType;
    wxStaticText* m_abstractionWidth;
    wxStaticText* m_abstractionRegisterPage;
    wxStaticText* m_abstractionRegisterOffset;
    wxStaticText* m_abstractionDescription;
    wxStaticText* m_abstractionHelp;
    wxStaticText* m_abstractionAccessRights;
    wxStaticText* m_abstractionDefaultValue;
    wxStaticText* m_singleValueLabel;
    wxTextCtrl* m_abstractionValue;
    wxStaticText* m_multipleValueLabel;
    wxChoice* m_abstractionComboValue;
    int m_nType; // Abstraction type
    /// Control identifiers

    enum {
        ID_DIALOGABSTRACTIONEDIT = 10042,
        ID_BUTTON18 = 10145,
        ID_TEXTCTRL_ABSTRACTION_VALUE = 10002,
        ID_CHOICE1 = 10043
    };
};

#endif
// _DIALOGABSTRACTIONEDIT_H_
