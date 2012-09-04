/////////////////////////////////////////////////////////////////////////////
// Name:        frmvscpcfg.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/02/2007 18:13:36
// RCS-ID:      
// Copyright:   (c) 2007 Ake Hedman, eurosource, <ake@eurosource.se>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FRMVSCPCFG_H_
#define _FRMVSCPCFG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmvscpcfg.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/grid.h"
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
#define ID_FRMVSCPCFG 10000
#define ID_GRID1 10001
#define SYMBOL_FRMVSCPCFG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMVSCPCFG_TITLE _("VSCP Configuration")
#define SYMBOL_FRMVSCPCFG_IDNAME ID_FRMVSCPCFG
#define SYMBOL_FRMVSCPCFG_SIZE wxSize(400, 300)
#define SYMBOL_FRMVSCPCFG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * frmVSCPCfg class declaration
 */

class frmVSCPCfg: public wxFrame
{    
    DECLARE_CLASS( frmVSCPCfg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmVSCPCfg();
    frmVSCPCfg( wxWindow* parent, wxWindowID id = SYMBOL_FRMVSCPCFG_IDNAME, const wxString& caption = SYMBOL_FRMVSCPCFG_TITLE, const wxPoint& pos = SYMBOL_FRMVSCPCFG_POSITION, const wxSize& size = SYMBOL_FRMVSCPCFG_SIZE, long style = SYMBOL_FRMVSCPCFG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRMVSCPCFG_IDNAME, const wxString& caption = SYMBOL_FRMVSCPCFG_TITLE, const wxPoint& pos = SYMBOL_FRMVSCPCFG_POSITION, const wxSize& size = SYMBOL_FRMVSCPCFG_SIZE, long style = SYMBOL_FRMVSCPCFG_STYLE );

    /// Destructor
    ~frmVSCPCfg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin frmVSCPCfg event handler declarations

////@end frmVSCPCfg event handler declarations

////@begin frmVSCPCfg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end frmVSCPCfg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmVSCPCfg member variables
////@end frmVSCPCfg member variables
};

#endif
    // _FRMVSCPCFG_H_
