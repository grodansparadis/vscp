/////////////////////////////////////////////////////////////////////////////
// Name:        frmmessagelog.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Wed 25 Apr 2007 11:54:26 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FRMMESSAGELOG_H_
#define _FRMMESSAGELOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmmessagelog.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "frmmessagelog_symbols.h"
#include "wx/mdi.h"
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
#define SYMBOL_FRMMESSAGELOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMMESSAGELOG_TITLE _("VSCP Message Log")
#define SYMBOL_FRMMESSAGELOG_IDNAME ID_FRMMESSAGELOG
#define SYMBOL_FRMMESSAGELOG_SIZE wxSize(600, 320)
#define SYMBOL_FRMMESSAGELOG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * frmMessageLog class declaration
 */

class frmMessageLog: public wxMDIChildFrame
{    
    DECLARE_CLASS( frmMessageLog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmMessageLog();
    frmMessageLog( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMMESSAGELOG_IDNAME, const wxString& caption = SYMBOL_FRMMESSAGELOG_TITLE, const wxPoint& pos = SYMBOL_FRMMESSAGELOG_POSITION, const wxSize& size = SYMBOL_FRMMESSAGELOG_SIZE, long style = SYMBOL_FRMMESSAGELOG_STYLE );

    bool Create( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMMESSAGELOG_IDNAME, const wxString& caption = SYMBOL_FRMMESSAGELOG_TITLE, const wxPoint& pos = SYMBOL_FRMMESSAGELOG_POSITION, const wxSize& size = SYMBOL_FRMMESSAGELOG_SIZE, long style = SYMBOL_FRMMESSAGELOG_STYLE );

    /// Destructor
    ~frmMessageLog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin frmMessageLog event handler declarations

////@end frmMessageLog event handler declarations

////@begin frmMessageLog member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmMessageLog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmMessageLog member variables
  /// Control identifiers
  enum {
    ID_FRMMESSAGELOG = 10008,
    ID_GRID_LOG = 10009,
    ID_BITMAPBUTTON_CLEAR_LIST = 10201,
    ID_BITMAPBUTTON1 = 10302
  };
////@end frmMessageLog member variables
};

#endif
    // _FRMMESSAGELOG_H_
