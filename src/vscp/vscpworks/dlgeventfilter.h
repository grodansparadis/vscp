/////////////////////////////////////////////////////////////////////////////
// Name:        dlgeventfilter.h
// Purpose:     
// Author:      Ake Hedman, Paradise of the Frog/Grodans Paradis AB
// Modified by: 
// Created:     13/01/2015 13:00:07
// RCS-ID:      
// Copyright:   Copyright (C) 2014 Grodans Paradis AB
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGEVENTFILTER_H_
#define _DLGEVENTFILTER_H_


/*!
 * Includes
 */

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
#define ID_EVENTFILTER 10000
#define ID_LISTBOX_DISPLAY 10001
#define ID_BUTTON5_ADD_DISPLAY_EVENT 10011
#define ID_LISTBOX_FILTER 10002
#define ID_BUTTON_ADD_FILTER_EVENT 10010
#define ID_RADIOBUTTON_DISPLAY 10003
#define ID_RADIOBUTTON_FILTER 10004
#define ID_CHECKBOX_ENABLE 10006
#define ID_BUTTON_OK 10005
#define ID_BUTTON_CANCEL 10007
#define ID_BUTTON_LOAD 10008
#define ID_BUTTON_SAVE 10009
#define SYMBOL_EVENTFILTER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_EVENTFILTER_TITLE _("Event Filter")
#define SYMBOL_EVENTFILTER_IDNAME ID_EVENTFILTER
#define SYMBOL_EVENTFILTER_SIZE wxSize(400, 300)
#define SYMBOL_EVENTFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * EventFilter class declaration
 */

class EventFilter: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( EventFilter )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    EventFilter();
    EventFilter( wxWindow* parent, wxWindowID id = SYMBOL_EVENTFILTER_IDNAME, const wxString& caption = SYMBOL_EVENTFILTER_TITLE, const wxPoint& pos = SYMBOL_EVENTFILTER_POSITION, const wxSize& size = SYMBOL_EVENTFILTER_SIZE, long style = SYMBOL_EVENTFILTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EVENTFILTER_IDNAME, const wxString& caption = SYMBOL_EVENTFILTER_TITLE, const wxPoint& pos = SYMBOL_EVENTFILTER_POSITION, const wxSize& size = SYMBOL_EVENTFILTER_SIZE, long style = SYMBOL_EVENTFILTER_STYLE );

    /// Destructor
    ~EventFilter();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin EventFilter event handler declarations

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_DISPLAY
    void OnListboxDisplaySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_DISPLAY
    void OnListboxDisplayDoubleClicked( wxCommandEvent& event );

    /// wxEVT_RIGHT_DOWN event handler for ID_LISTBOX_DISPLAY
    void OnRightDown( wxMouseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5_ADD_DISPLAY_EVENT
    void OnButton5AddDisplayEventClick( wxCommandEvent& event );

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

////@end EventFilter event handler declarations

////@begin EventFilter member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end EventFilter member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin EventFilter member variables
    wxListBox* m_ctrlListDisplay;
    wxListBox* m_ctrlListFilter;
    wxRadioButton* m_ctrlRadioDisplay;
    wxRadioButton* m_ctrlRadioFilter;
    wxCheckBox* m_ctrlCheckBoxEnable;
////@end EventFilter member variables
};

#endif
    // _DLGEVENTFILTER_H_
