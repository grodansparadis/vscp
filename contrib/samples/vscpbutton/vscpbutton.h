/////////////////////////////////////////////////////////////////////////////
// Name:        vscpbutton.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 15 Dec 2007 16:00:06 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007-2008 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _VSCPBUTTON_H_
#define _VSCPBUTTON_H_


/*!
 * Includes
 */

////@begin includes
#include "vscpbutton_symbols.h"
#include "wx/toolbar.h"
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
#define ID_VSCPBUTTON 10000
#define ID_TOOLBAR 10004
#define ID_TOOL 10005
#define ID_TOGGLEBUTTON1 10001
#define SYMBOL_VSCPBUTTON_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_VSCPBUTTON_TITLE _("VSCP Button")
#define SYMBOL_VSCPBUTTON_IDNAME ID_VSCPBUTTON
#define SYMBOL_VSCPBUTTON_SIZE wxSize(400, 300)
#define SYMBOL_VSCPBUTTON_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * VSCPButton class declaration
 */

class VSCPButton: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( VSCPButton )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    VSCPButton();
    VSCPButton( wxWindow* parent, wxWindowID id = SYMBOL_VSCPBUTTON_IDNAME, const wxString& caption = SYMBOL_VSCPBUTTON_TITLE, const wxPoint& pos = SYMBOL_VSCPBUTTON_POSITION, const wxSize& size = SYMBOL_VSCPBUTTON_SIZE, long style = SYMBOL_VSCPBUTTON_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_VSCPBUTTON_IDNAME, const wxString& caption = SYMBOL_VSCPBUTTON_TITLE, const wxPoint& pos = SYMBOL_VSCPBUTTON_POSITION, const wxSize& size = SYMBOL_VSCPBUTTON_SIZE, long style = SYMBOL_VSCPBUTTON_STYLE );

    /// Destructor
    ~VSCPButton();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin VSCPButton event handler declarations

////@end VSCPButton event handler declarations

////@begin VSCPButton member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end VSCPButton member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin VSCPButton member variables
////@end VSCPButton member variables
};

#endif
    // _VSCPBUTTON_H_
