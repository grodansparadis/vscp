/////////////////////////////////////////////////////////////////////////////
//  Name:        dlgeventfilter.h
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


#ifndef _DLGEVENTFILTER_H_
#define _DLGEVENTFILTER_H_

// Filter modes
#define FILTER_MODE_DISPLAY         0
#define FILTER_MODE_FILTER          1 



/*!
 * Control identifiers
 */


#define ID_EVENTFILTER                      10000
#define ID_LISTBOX_DISPLAY                  10001
#define ID_BUTTON_ADD_DISPLAY_EVENT         10011
#define ID_LISTBOX_FILTER                   10002
#define ID_BUTTON_ADD_FILTER_EVENT          10010
#define ID_RADIOBUTTON_DISPLAY              10003
#define ID_RADIOBUTTON_FILTER               10004
#define ID_CHECKBOX_ENABLE                  10006
#define ID_BUTTON_LOAD                      10008
#define ID_BUTTON_SAVE                      10009
#define Menu_Popup_Display_Remove           10010
#define Menu_Popup_Filter_Remove            10011
#define Menu_Popup_Display_Add              10012
#define Menu_Popup_Filter_Add               10013

#define SYMBOL_EVENTFILTER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_EVENTFILTER_TITLE _("Event Filter")
#define SYMBOL_EVENTFILTER_IDNAME ID_EVENTFILTER
#define SYMBOL_EVENTFILTER_SIZE wxSize(400, 300)
#define SYMBOL_EVENTFILTER_POSITION wxDefaultPosition


/*!
 * dlgEventFilter class declaration
 */

class dlgEventFilter: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgEventFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgEventFilter();
    dlgEventFilter( wxWindow* parent, 
                        wxWindowID id = SYMBOL_EVENTFILTER_IDNAME, 
                        const wxString& caption = SYMBOL_EVENTFILTER_TITLE, 
                        const wxPoint& pos = SYMBOL_EVENTFILTER_POSITION, 
                        const wxSize& size = SYMBOL_EVENTFILTER_SIZE, 
                        long style = SYMBOL_EVENTFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                        wxWindowID id = SYMBOL_EVENTFILTER_IDNAME, 
                        const wxString& caption = SYMBOL_EVENTFILTER_TITLE, 
                        const wxPoint& pos = SYMBOL_EVENTFILTER_POSITION, 
                        const wxSize& size = SYMBOL_EVENTFILTER_SIZE, 
                        long style = SYMBOL_EVENTFILTER_STYLE );

    /// Destructor
    ~dlgEventFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_DISPLAY
    void OnListboxDisplaySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_DISPLAY
    void OnListboxDisplayDoubleClicked( wxCommandEvent& event );

    /// wxEVT_RIGHT_DOWN event handler for ID_LISTBOX_DISPLAY
    void OnRightDown( wxMouseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5_ADD_DISPLAY_EVENT
    void OnButtonAddDisplayEventClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_FILTER
    void OnListboxFilterSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_FILTER
    void OnListboxFilterDoubleClicked( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_FILTER_EVENT
    void OnButtonAddFilterEventClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_DISPLAY
    void OnRadiobuttonDisplaySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON_FILTER
    void OnRadiobuttonFilterSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_ENABLE
    void OnCheckboxEnableClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
    void OnButtonOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
    void OnButtonCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD
    void OnButtonLoadClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE
    void OnButtonSaveClick( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Remove one or more display rows
    void OnRemoveDisplayRows( wxCommandEvent& event );

    /// Remove one or more filter rows
    void OnRemoveFilterRows( wxCommandEvent& event );

    bool m_bfilterActive;
    int m_nfilterMode;

    wxListBox* m_ctrlListDisplay;
    wxListBox* m_ctrlListFilter;
    wxRadioButton* m_ctrlRadioDisplay;
    wxRadioButton* m_ctrlRadioFilter;
    wxCheckBox* m_ctrlCheckBoxEnable;
};

#endif
    // _DLGEVENTFILTER_H_
