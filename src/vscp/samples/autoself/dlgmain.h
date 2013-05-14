/////////////////////////////////////////////////////////////////////////////
// Name:        dlgmain.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:43:18 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGMAIN_H_
#define _DLGMAIN_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgmain.h"
#endif

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
#define ID_DLGMAIN 10000
#define ID_BUTTON1 10003
#define ID_BITMAPBUTTON1 10001
#define ID_BITMAPBUTTON 10002
#define SYMBOL_DLGMAIN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGMAIN_TITLE _("Autoself control")
#define SYMBOL_DLGMAIN_IDNAME ID_DLGMAIN
#define SYMBOL_DLGMAIN_SIZE wxSize(800, 600)
#define SYMBOL_DLGMAIN_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * dlgMain class declaration
 */

class dlgMain: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgMain )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgMain();
    dlgMain( wxWindow* parent, wxWindowID id = SYMBOL_DLGMAIN_IDNAME, const wxString& caption = SYMBOL_DLGMAIN_TITLE, const wxPoint& pos = SYMBOL_DLGMAIN_POSITION, const wxSize& size = SYMBOL_DLGMAIN_SIZE, long style = SYMBOL_DLGMAIN_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGMAIN_IDNAME, const wxString& caption = SYMBOL_DLGMAIN_TITLE, const wxPoint& pos = SYMBOL_DLGMAIN_POSITION, const wxSize& size = SYMBOL_DLGMAIN_SIZE, long style = SYMBOL_DLGMAIN_STYLE );

    /// Destructor
    ~dlgMain();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin dlgMain event handler declarations

////@end dlgMain event handler declarations

////@begin dlgMain member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end dlgMain member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin dlgMain member variables
////@end dlgMain member variables
};

#endif
    // _DLGMAIN_H_
