/////////////////////////////////////////////////////////////////////////////
// Name:        fli4lcidtest.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 15 Nov 2007 15:27:16 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FLI4LCIDTEST_H_
#define _FLI4LCIDTEST_H_


/*!
 * Includes
 */

////@begin includes
#include "fli4lcidtest_symbols.h"
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
#define ID_FLI4LCIDTEST 10000
#define ID_TEXTCTRL1 10001
#define ID_BUTTON_FETCH 10002
#define SYMBOL_FLI4LCIDTEST_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_FLI4LCIDTEST_TITLE _("fli4l CID test")
#define SYMBOL_FLI4LCIDTEST_IDNAME ID_FLI4LCIDTEST
#define SYMBOL_FLI4LCIDTEST_SIZE wxSize(400, 300)
#define SYMBOL_FLI4LCIDTEST_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * fli4lCIDtest class declaration
 */

class fli4lCIDtest: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( fli4lCIDtest )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    fli4lCIDtest();
    fli4lCIDtest( wxWindow* parent, wxWindowID id = SYMBOL_FLI4LCIDTEST_IDNAME, const wxString& caption = SYMBOL_FLI4LCIDTEST_TITLE, const wxPoint& pos = SYMBOL_FLI4LCIDTEST_POSITION, const wxSize& size = SYMBOL_FLI4LCIDTEST_SIZE, long style = SYMBOL_FLI4LCIDTEST_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FLI4LCIDTEST_IDNAME, const wxString& caption = SYMBOL_FLI4LCIDTEST_TITLE, const wxPoint& pos = SYMBOL_FLI4LCIDTEST_POSITION, const wxSize& size = SYMBOL_FLI4LCIDTEST_SIZE, long style = SYMBOL_FLI4LCIDTEST_STYLE );

    /// Destructor
    ~fli4lCIDtest();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin fli4lCIDtest event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_FETCH
    void OnButtonFetchClick( wxCommandEvent& event );

////@end fli4lCIDtest event handler declarations

////@begin fli4lCIDtest member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end fli4lCIDtest member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin fli4lCIDtest member variables
    wxTextCtrl* m_ctrlEditValue;
    wxButton* m_btnFetch;
////@end fli4lCIDtest member variables
};

#endif
    // _FLI4LCIDTEST_H_
