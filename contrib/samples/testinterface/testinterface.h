/////////////////////////////////////////////////////////////////////////////
// Name:        testinterface.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     15/04/2009 10:35:08
// RCS-ID:      
// Copyright:   (c) 2009 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _TESTINTERFACE_H_
#define _TESTINTERFACE_H_


/*!
 * Includes
 */

#include "../../common/vscptcpif.h"
#include "../../common/canalsuperwrapper.h"

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
#define ID_TESTINTERFACE 10000
#define ID_TEXTCTRL_TEXT_SERVER 10001
#define ID_TEXTCTRL_TEXT_USERNAME 10002
#define ID_TEXTCTRL_TEXT_PASSWORD 10003
#define ID_CHECKBOX 10008
#define ID_BUTTON_CONNECT 10004
#define ID_BUTTON_DISCONNECT 10005
#define ID_TEXTCTRL_LOG 10007
#define SYMBOL_TESTINTERFACE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_TESTINTERFACE_TITLE _("TestInterface")
#define SYMBOL_TESTINTERFACE_IDNAME ID_TESTINTERFACE
#define SYMBOL_TESTINTERFACE_SIZE wxSize(400, 300)
#define SYMBOL_TESTINTERFACE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * TestInterface class declaration
 */

class TestInterface: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( TestInterface )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    TestInterface();
    TestInterface( wxWindow* parent, wxWindowID id = SYMBOL_TESTINTERFACE_IDNAME, const wxString& caption = SYMBOL_TESTINTERFACE_TITLE, const wxPoint& pos = SYMBOL_TESTINTERFACE_POSITION, const wxSize& size = SYMBOL_TESTINTERFACE_SIZE, long style = SYMBOL_TESTINTERFACE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_TESTINTERFACE_IDNAME, const wxString& caption = SYMBOL_TESTINTERFACE_TITLE, const wxPoint& pos = SYMBOL_TESTINTERFACE_POSITION, const wxSize& size = SYMBOL_TESTINTERFACE_SIZE, long style = SYMBOL_TESTINTERFACE_STYLE );

    /// Destructor
    ~TestInterface();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


	
	VscpTcpIf m_tcpif;
    CCanalSuperWrapper m_csw;

////@begin TestInterface event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CONNECT
    void OnButtonConnectClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_DISCONNECT
    void OnButtonDisconnectClick( wxCommandEvent& event );

////@end TestInterface event handler declarations

////@begin TestInterface member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end TestInterface member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin TestInterface member variables
    wxTextCtrl* m_txtServer;
    wxTextCtrl* m_txtUsername;
    wxTextCtrl* m_txtPassword;
    wxCheckBox* m_chkUseCanalSuperWrapper;
    wxButton* m_btnConnect;
    wxButton* m_btnDisConnect;
    wxTextCtrl* m_txtLog;
////@end TestInterface member variables
};

#endif
    // _TESTINTERFACE_H_
