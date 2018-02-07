/////////////////////////////////////////////////////////////////////////////
//  Name:        dlgclasstypeselect.h
//  Purpose:     
//  Author:      Ake Hedman
//  Modified by: 
//  Created:     Sat 30 Jun 2007 14:08:14 CEST
//  RCS-ID:      
//  Copyright:   (C) 2007-2018                       
//  Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//  Licence:     
//  This program is free software; you can redistribute it and/or  
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version
//  2 of the License, or (at your option) any later version.
// 
//  This file is part of the VSCP (http://www.vscp.org) 
// 
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this file see the file COPYING.  If not, write to
//  the Free Software Foundation, 59 Temple Place - Suite 330,
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



#ifndef _DLGCLASSTYPESELECT_H_
#define _DLGCLASSTYPESELECT_H_





/*!
 * Control identifiers
 */

#define ID_DLGCLASSTYPESELECT 10000
#define ID_COMBOBOX 10001
#define ID_COMBOBOX1 10002
#define SYMBOL_DLGCLASSTYPESELECT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DLGCLASSTYPESELECT_TITLE _("Very Simple Control Protocol")
#define SYMBOL_DLGCLASSTYPESELECT_IDNAME ID_DLGCLASSTYPESELECT
#define SYMBOL_DLGCLASSTYPESELECT_SIZE wxSize(400, 300)
#define SYMBOL_DLGCLASSTYPESELECT_POSITION wxDefaultPosition


/*!
 * dlgClassTypeSelect class declaration
 */

class dlgClassTypeSelect: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgClassTypeSelect )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgClassTypeSelect();
    dlgClassTypeSelect( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DLGCLASSTYPESELECT_IDNAME, 
                            const wxString& caption = SYMBOL_DLGCLASSTYPESELECT_TITLE, 
                            const wxPoint& pos = SYMBOL_DLGCLASSTYPESELECT_POSITION, 
                            const wxSize& size = SYMBOL_DLGCLASSTYPESELECT_SIZE, 
                            long style = SYMBOL_DLGCLASSTYPESELECT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGCLASSTYPESELECT_IDNAME, 
                    const wxString& caption = SYMBOL_DLGCLASSTYPESELECT_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGCLASSTYPESELECT_POSITION, 
                    const wxSize& size = SYMBOL_DLGCLASSTYPESELECT_SIZE, 
                    long style = SYMBOL_DLGCLASSTYPESELECT_STYLE );

    /// Destructor
    ~dlgClassTypeSelect();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
    void OnComboboxClassSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX1
    void OnComboboxTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonOKClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnButtonCANCELClick( wxCommandEvent& event );

    /*!
        Fill in VSCP class info in combo
    */
    void fillClassCombo( void );

    /*!
        Fill in VSCP type info in combo
    */
    void fillTypeCombo( unsigned int vscp_class );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Combo box for VSCP class information
    wxComboBox* m_ctrlComboClass;

    /// Combo box for VSCP type information
    wxComboBox* m_ctrlComboType;
};

#endif
    // _DLGCLASSTYPESELECT_H_
