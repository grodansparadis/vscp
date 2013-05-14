/////////////////////////////////////////////////////////////////////////////
// Name:        defaultsettings.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 23 Oct 2009 16:00:29 CEST
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DEFAULTSETTINGS_H_
#define _DEFAULTSETTINGS_H_


/*!
 * Includes
 */

////@begin includes
#include "defaultsettings_symbols.h"
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
#define ID_DEFAULTSETTINGS 10011
#define SYMBOL_DEFAULTSETTINGS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DEFAULTSETTINGS_TITLE _("Default Settings")
#define SYMBOL_DEFAULTSETTINGS_IDNAME ID_DEFAULTSETTINGS
#define SYMBOL_DEFAULTSETTINGS_SIZE wxSize(400, 300)
#define SYMBOL_DEFAULTSETTINGS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * DefaultSettings class declaration
 */

class DefaultSettings: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DefaultSettings )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DefaultSettings();
    DefaultSettings( wxWindow* parent, wxWindowID id = SYMBOL_DEFAULTSETTINGS_IDNAME, const wxString& caption = SYMBOL_DEFAULTSETTINGS_TITLE, const wxPoint& pos = SYMBOL_DEFAULTSETTINGS_POSITION, const wxSize& size = SYMBOL_DEFAULTSETTINGS_SIZE, long style = SYMBOL_DEFAULTSETTINGS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DEFAULTSETTINGS_IDNAME, const wxString& caption = SYMBOL_DEFAULTSETTINGS_TITLE, const wxPoint& pos = SYMBOL_DEFAULTSETTINGS_POSITION, const wxSize& size = SYMBOL_DEFAULTSETTINGS_SIZE, long style = SYMBOL_DEFAULTSETTINGS_STYLE );

    /// Destructor
    ~DefaultSettings();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin DefaultSettings event handler declarations
////@end DefaultSettings event handler declarations

////@begin DefaultSettings member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DefaultSettings member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DefaultSettings member variables
////@end DefaultSettings member variables
};

#endif
    // _DEFAULTSETTINGS_H_
