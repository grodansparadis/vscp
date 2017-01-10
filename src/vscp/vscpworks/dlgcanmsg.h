/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanmsg.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:47:00
// RCS-ID:      
// Copyright:   (C) 2007-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGCANMSG_H_
#define _DLGCANMSG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgcanmsg.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgcanmsg_symbols.h"
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG_CAN_MESSAGE 10003
#define ID_GRID_LOG 10004
#define ID_BITMAPBUTTON_EDIT 10002
#define ID_BITMAPBUTTON_DELETE 10005
#define ID_BITMAPBUTTON_LOAD 10006
#define ID_BITMAPBUTTON_SAVE 10054
#define ID_GRID_SEND 10001
#define ID_BITMAPBUTTON_SEND 10007
#define ID_BITMAPBUTTON_ACTIVATE 10000
#define ID_BITMAPBUTTON_CLEAR 10008
#define SYMBOL_DLGCANMSG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGCANMSG_TITLE _("CAN Session")
#define SYMBOL_DLGCANMSG_IDNAME ID_DIALOG_CAN_MESSAGE
#define SYMBOL_DLGCANMSG_SIZE wxSize(400, 300)
#define SYMBOL_DLGCANMSG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * DlgCANMsg class declaration
 */

class DlgCANMsg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DlgCANMsg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DlgCANMsg();
    DlgCANMsg( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANMSG_IDNAME, const wxString& caption = SYMBOL_DLGCANMSG_TITLE, const wxPoint& pos = SYMBOL_DLGCANMSG_POSITION, const wxSize& size = SYMBOL_DLGCANMSG_SIZE, long style = SYMBOL_DLGCANMSG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGCANMSG_IDNAME, const wxString& caption = SYMBOL_DLGCANMSG_TITLE, const wxPoint& pos = SYMBOL_DLGCANMSG_POSITION, const wxSize& size = SYMBOL_DLGCANMSG_SIZE, long style = SYMBOL_DLGCANMSG_STYLE );

    /// Destructor
    ~DlgCANMsg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin DlgCANMsg event handler declarations

    /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_CAN_MESSAGE
    void OnInitDialog( wxInitDialogEvent& event );

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_CAN_MESSAGE
    void OnCloseWindow( wxCloseEvent& event );

////@end DlgCANMsg event handler declarations

////@begin DlgCANMsg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DlgCANMsg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DlgCANMsg member variables
    wxGrid* m_ctrlGrid;
    wxBitmapButton* m_btnEdit;
    wxBitmapButton* m_btnDelete;
    wxBitmapButton* m_btnLoad;
    wxBitmapButton* m_btnSave;
    wxGrid* m_ctrlGridSend;
    wxBitmapButton* m_btnSend;
    wxBitmapButton* m_btnActivate;
    wxBitmapButton* m_btnClear;
////@end DlgCANMsg member variables
};

#endif
    // _DLGCANMSG_H_
