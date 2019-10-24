/////////////////////////////////////////////////////////////////////////////
// Name:        frmmain.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 22:03:27 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FRMMAIN_H_
#define _FRMMAIN_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmmain.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "frmmain_symbols.h"
#include "wx/mdi.h"
#include "wx/statusbr.h"
////@end includes


#include <wx/file.h>

DECLARE_EVENT_TYPE(wxMAINLOG_EVENT, wxID_ANY)



/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FRMMAIN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMMAIN_TITLE _("Auto-Self Control - Copyright 2006-2009 Ljusdals Energi AB")
#define SYMBOL_FRMMAIN_IDNAME ID_FRMMAIN
#define SYMBOL_FRMMAIN_SIZE wxSize(800, 600)
#define SYMBOL_FRMMAIN_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * frmMain class declaration
 */

class frmMain: public wxMDIParentFrame
{    
    DECLARE_CLASS( frmMain )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmMain();
    frmMain( wxWindow* parent, wxWindowID id = SYMBOL_FRMMAIN_IDNAME, const wxString& caption = SYMBOL_FRMMAIN_TITLE, const wxPoint& pos = SYMBOL_FRMMAIN_POSITION, const wxSize& size = SYMBOL_FRMMAIN_SIZE, long style = SYMBOL_FRMMAIN_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRMMAIN_IDNAME, const wxString& caption = SYMBOL_FRMMAIN_TITLE, const wxPoint& pos = SYMBOL_FRMMAIN_POSITION, const wxSize& size = SYMBOL_FRMMAIN_SIZE, long style = SYMBOL_FRMMAIN_STYLE );

    /// Destructor
    ~frmMain();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
    /// Main log message handler
    void OnMainLogMsg( wxCommandEvent &event );
    
    /// Logfile
    wxFile *m_plogfile;
    

////@begin frmMain event handler declarations

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_OPEN
  void OnMenuitemOpenClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_EXIT
  void OnMenuitemExitClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SETTINGS
  void OnMenuitemSettingsClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_ABOUT
  void OnMenuitemAboutClick( wxCommandEvent& event );

////@end frmMain event handler declarations

////@begin frmMain member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmMain member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmMain member variables
  /// Control identifiers
  enum {
    ID_FRMMAIN = 11111,
    ID_MENUITEM_OPEN = 10006,
    ID_MENUITEM_EXIT = 10004,
    ID_MENUITEM_SETTINGS = 10036,
    ID_MENUITEM_VSCP_SITE = 10018,
    ID_MENUITEM_ABOUT = 10017,
    ID_STATUSBAR_MAIN = 10005
  };
////@end frmMain member variables
};

#endif
    // _FRMMAIN_H_
