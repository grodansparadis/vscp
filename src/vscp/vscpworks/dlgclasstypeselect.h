/////////////////////////////////////////////////////////////////////////////
// Name:        dlgclasstypeselect.h
// Purpose:     
// Author:      Ake Hedman, Paradise of the Frog/Grodans Paradis AB
// Modified by: 
// Created:     13/01/2015 14:13:17
// RCS-ID:      
// Copyright:   Copyright (C) 2014 Grodans Paradis AB
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGCLASSTYPESELECT_H_
#define _DLGCLASSTYPESELECT_H_


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
#define ID_DLGCLASSTYPESELECT 10000
#define ID_COMBOBOX 10001
#define ID_COMBOBOX1 10002
#define ID_BUTTON 10003
#define ID_BUTTON1 10004
#define SYMBOL_DLGCLASSTYPESELECT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DLGCLASSTYPESELECT_TITLE _("Very Simple Control Protocol")
#define SYMBOL_DLGCLASSTYPESELECT_IDNAME ID_DLGCLASSTYPESELECT
#define SYMBOL_DLGCLASSTYPESELECT_SIZE wxSize(400, 300)
#define SYMBOL_DLGCLASSTYPESELECT_POSITION wxDefaultPosition
////@end control identifiers


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
    dlgClassTypeSelect( wxWindow* parent, wxWindowID id = SYMBOL_DLGCLASSTYPESELECT_IDNAME, const wxString& caption = SYMBOL_DLGCLASSTYPESELECT_TITLE, const wxPoint& pos = SYMBOL_DLGCLASSTYPESELECT_POSITION, const wxSize& size = SYMBOL_DLGCLASSTYPESELECT_SIZE, long style = SYMBOL_DLGCLASSTYPESELECT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGCLASSTYPESELECT_IDNAME, const wxString& caption = SYMBOL_DLGCLASSTYPESELECT_TITLE, const wxPoint& pos = SYMBOL_DLGCLASSTYPESELECT_POSITION, const wxSize& size = SYMBOL_DLGCLASSTYPESELECT_SIZE, long style = SYMBOL_DLGCLASSTYPESELECT_STYLE );

    /// Destructor
    ~dlgClassTypeSelect();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgClassTypeSelect event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
    void OnComboboxSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX1
    void OnCombobox1Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnButton1Click( wxCommandEvent& event );

////@end dlgClassTypeSelect event handler declarations

////@begin dlgClassTypeSelect member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end dlgClassTypeSelect member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgClassTypeSelect member variables
    wxComboBox* m_ctrlComboType;
////@end dlgClassTypeSelect member variables
};

#endif
    // _DLGCLASSTYPESELECT_H_
