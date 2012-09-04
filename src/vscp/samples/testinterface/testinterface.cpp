/////////////////////////////////////////////////////////////////////////////
// Name:        testinterface.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     15/04/2009 10:35:08
// RCS-ID:      
// Copyright:   (c) 2009 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif




////@begin includes
////@end includes

#include "testinterface.h"

////@begin XPM images
////@end XPM images


/*!
 * TestInterface type definition
 */

IMPLEMENT_DYNAMIC_CLASS( TestInterface, wxDialog )


/*!
 * TestInterface event table definition
 */

BEGIN_EVENT_TABLE( TestInterface, wxDialog )

////@begin TestInterface event table entries
    EVT_BUTTON( ID_BUTTON_CONNECT, TestInterface::OnButtonConnectClick )

    EVT_BUTTON( ID_BUTTON_DISCONNECT, TestInterface::OnButtonDisconnectClick )

////@end TestInterface event table entries

END_EVENT_TABLE()


/*!
 * TestInterface constructors
 */

TestInterface::TestInterface()
{
    Init();
}

TestInterface::TestInterface( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * TestInterface creator
 */

bool TestInterface::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin TestInterface creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end TestInterface creation
    return true;
}


/*!
 * TestInterface destructor
 */

TestInterface::~TestInterface()
{
////@begin TestInterface destruction
////@end TestInterface destruction
}


/*!
 * Member initialisation
 */

void TestInterface::Init()
{
////@begin TestInterface member initialisation
    m_txtServer = NULL;
    m_txtUsername = NULL;
    m_txtPassword = NULL;
    m_chkUseCanalSuperWrapper = NULL;
    m_btnConnect = NULL;
    m_btnDisConnect = NULL;
    m_txtLog = NULL;
////@end TestInterface member initialisation
}


/*!
 * Control creation for TestInterface
 */

void TestInterface::CreateControls()
{    
////@begin TestInterface content construction
    TestInterface* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Server"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    m_txtServer = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_TEXT_SERVER, _("localhost"), wxDefaultPosition, wxSize(300, -1), 0 );
    m_txtServer->SetName(_T("txtServer"));
    itemBoxSizer2->Add(m_txtServer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    m_txtUsername = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_TEXT_USERNAME, _("admin"), wxDefaultPosition, wxSize(300, -1), 0 );
    m_txtUsername->SetName(_T("txtUserName"));
    itemBoxSizer2->Add(m_txtUsername, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

    m_txtPassword = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_TEXT_PASSWORD, _("secret"), wxDefaultPosition, wxSize(300, -1), 0 );
    m_txtPassword->SetName(_T("txtPassword"));
    itemBoxSizer2->Add(m_txtPassword, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_chkUseCanalSuperWrapper = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Use CanalSuperWrapper"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkUseCanalSuperWrapper->SetValue(false);
    itemBoxSizer9->Add(m_chkUseCanalSuperWrapper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_btnConnect = new wxButton( itemDialog1, ID_BUTTON_CONNECT, _("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_btnConnect, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_btnDisConnect = new wxButton( itemDialog1, ID_BUTTON_DISCONNECT, _("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_btnDisConnect, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtLog = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_LOG, _T(""), wxDefaultPosition, wxSize(-1, 200), wxTE_MULTILINE );
    m_txtLog->SetName(_T("txtLog"));
    itemBoxSizer2->Add(m_txtLog, 0, wxGROW|wxALL, 5);

////@end TestInterface content construction
}


/*!
 * Should we show tooltips?
 */

bool TestInterface::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap TestInterface::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin TestInterface bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end TestInterface bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon TestInterface::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin TestInterface icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end TestInterface icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CONNECT
 */

void TestInterface::OnButtonConnectClick( wxCommandEvent& event )
{
    if ( !m_chkUseCanalSuperWrapper->GetValue() ) {
        if ( 1922 == m_tcpif.doCmdOpen( m_txtServer->GetValue(), 
                                            9598, 
                                            m_txtUsername->GetValue(), 
                                            m_txtPassword->GetValue() ) ) {
            m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Connected with VscpTcpIf") );
            m_btnConnect->Enable( false );
        }
        else {
            m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Failed to connect woth VscpTcpIf") );
            m_btnConnect->Enable();
        }
    }
    else {
        m_csw.SetInterface( m_txtServer->GetValue(),
                        9598, 
                        m_txtUsername->GetValue(), 
                        m_txtPassword->GetValue() );
        if ( m_csw.doCmdOpen() ) {
            m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Connected  with CanalSuperWrapper") );
            m_btnConnect->Enable( false );
        }
        else {
            m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Failed to connect woth CanalSuperWrapper") );
            m_btnConnect->Enable();
        }
    }
    event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_DISCONNECT
 */

void TestInterface::OnButtonDisconnectClick( wxCommandEvent& event )
{
    if ( !m_chkUseCanalSuperWrapper->GetValue() ) {
        m_tcpif.doCmdClose();
        m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Disconnected") );
    }
    else {
        m_csw.doCmdClose();
        m_txtLog->SetValue( m_txtLog->GetValue() + _("\r\n") + _("Disconnected with CanalSuperWrapper") );
    }
	m_btnConnect->Enable();
    event.Skip();
}

