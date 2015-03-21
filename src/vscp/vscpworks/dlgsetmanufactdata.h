/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsetmanufactdata.h
// Purpose:     
// Author:      Anthemion Software Ltd
// Modified by: 
// Created:     21/03/2015 14:17:23
// RCS-ID:      
// Copyright:   (c) Anthemion Software Ltd
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGSETMANUFACTDATA_H_
#define _DLGSETMANUFACTDATA_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgsetmanufactdata.h"
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
#define ID_CSETMANUFACTDATA 10000
#define ID_TEXTCTRL_GUID 10001
#define ID_TEXTCTRL_MANDEVID 10002
#define ID_TEXTCTRL_MANDEVSUBID 10003
#define ID_BUTTON_LOAD 10006
#define ID_BUTTON_WRITE 10004
#define SYMBOL_CSETMANUFACTDATA_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSETMANUFACTDATA_TITLE _("Set GUID")
#define SYMBOL_CSETMANUFACTDATA_IDNAME ID_CSETMANUFACTDATA
#define SYMBOL_CSETMANUFACTDATA_SIZE wxSize(400, 300)
#define SYMBOL_CSETMANUFACTDATA_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CsetManufactData class declaration
 */

class CsetManufactData: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CsetManufactData )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CsetManufactData();
    CsetManufactData( wxWindow* parent, wxWindowID id = SYMBOL_CSETMANUFACTDATA_IDNAME, const wxString& caption = SYMBOL_CSETMANUFACTDATA_TITLE, const wxPoint& pos = SYMBOL_CSETMANUFACTDATA_POSITION, const wxSize& size = SYMBOL_CSETMANUFACTDATA_SIZE, long style = SYMBOL_CSETMANUFACTDATA_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSETMANUFACTDATA_IDNAME, const wxString& caption = SYMBOL_CSETMANUFACTDATA_TITLE, const wxPoint& pos = SYMBOL_CSETMANUFACTDATA_POSITION, const wxSize& size = SYMBOL_CSETMANUFACTDATA_SIZE, long style = SYMBOL_CSETMANUFACTDATA_STYLE );

    /// Destructor
    ~CsetManufactData();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CsetManufactData event handler declarations

////@end CsetManufactData event handler declarations

////@begin CsetManufactData member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CsetManufactData member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CsetManufactData member variables
    wxTextCtrl* m_pctrlGUID;
    wxTextCtrl* m_pctrlManDevId;
    wxTextCtrl* m_pctrlManDevSubId;
////@end CsetManufactData member variables
};

#endif
    // _DLGSETMANUFACTDATA_H_
