/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpinterfacesettings.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Fri 26 Oct 2007 11:57:04 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
// 

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgvscpinterfacesettings.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/bookctrl.h"
////@end includes

#include <wx/tokenzr.h>
#include <wx/progdlg.h>

#include <vscpremotetcpif.h>
#include <dllwrapper.h>
#include <canalconfobj.h>
#include <vscpbase64.h>
#include "dlgvscpinterfacesettings.h"
#include "dlgselectdaemoninterface.h"
#include "dlgvscpfilter.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVscpInterfaceSettings type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgVscpInterfaceSettings, wxPropertySheetDialog )


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // dlgVscpInterfaceSettings event table definition
    //

    BEGIN_EVENT_TABLE( dlgVscpInterfaceSettings, wxPropertySheetDialog )

    ////@begin dlgVscpInterfaceSettings event table entries
  EVT_BUTTON( ID_BUTTON_VSCP_DRIVER_SET_PATH, dlgVscpInterfaceSettings::OnButtonVscpDriverSetPathClick )
  EVT_BUTTON( ID_BUTTON_VSCP_SET_CONFIGURATION, dlgVscpInterfaceSettings::OnButtonVscpSetConfigurationClick )
  EVT_BUTTON( ID_BUTTON_SET_FILTER, dlgVscpInterfaceSettings::OnButtonSetFilterClick )
  EVT_BUTTON( ID_BUTTON_TEST_INTERFACE, dlgVscpInterfaceSettings::OnButtonTestInterfaceClick )
  EVT_BUTTON( ID_BUTTON, dlgVscpInterfaceSettings::OnButtonGetInterfacesClick )
    ////@end dlgVscpInterfaceSettings event table entries

    END_EVENT_TABLE()


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // dlgVscpInterfaceSettings constructors
    //

    dlgVscpInterfaceSettings::dlgVscpInterfaceSettings()
{
    Init();
}

dlgVscpInterfaceSettings::dlgVscpInterfaceSettings( wxWindow* parent, 
                                                        wxWindowID id, 
                                                        const wxString& caption, 
                                                        const wxPoint& pos, 
                                                        const wxSize& size, 
                                                        long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVscpInterfaceSettings creator
//

bool dlgVscpInterfaceSettings::Create( wxWindow* parent, 
                                        wxWindowID id, 
                                        const wxString& caption, 
                                        const wxPoint& pos, 
                                        const wxSize& size, 
                                        long style )
{
    ////@begin dlgVscpInterfaceSettings creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  SetSheetStyle(wxPROPSHEET_DEFAULT);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  LayoutDialog();
  Centre();
    ////@end dlgVscpInterfaceSettings creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVscpInterfaceSettings destructor
//

dlgVscpInterfaceSettings::~dlgVscpInterfaceSettings()
{
    ////@begin dlgVscpInterfaceSettings destruction
    ////@end dlgVscpInterfaceSettings destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgVscpInterfaceSettings::Init()
{
    ////@begin dlgVscpInterfaceSettings member initialisation
  m_panelCanal = NULL;
  m_DriverDescription = NULL;
  m_PathToDriver = NULL;
  m_DriverConfigurationString = NULL;
  m_DriverFlags = NULL;
  m_panelServer = NULL;
  m_RemoteServerDescription = NULL;
  m_RemoteServerURL = NULL;
  m_RemoteServerUsername = NULL;
  m_RemoteServerPassword = NULL;
  m_fullLevel2 = NULL;
  m_RemoteInterfaceName = NULL;
  m_btnTestConnection = NULL;
  m_btnGetInterfaces = NULL;
    ////@end dlgVscpInterfaceSettings member initialisation

    // Init filter
    vscp_clearVSCPFilter( &m_vscpfilter );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgVscpInterfaceSettings
//

void dlgVscpInterfaceSettings::CreateControls()
{    
    ////@begin dlgVscpInterfaceSettings content construction
  dlgVscpInterfaceSettings* itemPropertySheetDialog1 = this;

  m_panelCanal = new wxPanel;
  m_panelCanal->Create( GetBookCtrl(), ID_PANEL_INTERFACE_DRIVER, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  m_panelCanal->SetHelpText(_("For a CANAL driver enter or edit the description below and press OK"));
  if (dlgVscpInterfaceSettings::ShowToolTips())
    m_panelCanal->SetToolTip(_("For a CANAL driver enter or edit the description below and press OK"));
  m_panelCanal->SetName(wxT("canal"));
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  m_panelCanal->SetSizer(itemBoxSizer3);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText6 = new wxStaticText;
  itemStaticText6->Create( m_panelCanal, wxID_STATIC, _("Description :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DriverDescription = new wxTextCtrl;
  m_DriverDescription->Create( m_panelCanal, ID_DriverDescription, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer5->Add(m_DriverDescription, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer5->Add(52, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer9, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( m_panelCanal, wxID_STATIC, _("Path to driver :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_PathToDriver = new wxTextCtrl;
  m_PathToDriver->Create( m_panelCanal, ID_PathToDriver, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer9->Add(m_PathToDriver, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton12 = new wxButton;
  itemButton12->Create( m_panelCanal, ID_BUTTON_VSCP_DRIVER_SET_PATH, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer9->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer9->Add(20, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer14, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText15 = new wxStaticText;
  itemStaticText15->Create( m_panelCanal, wxID_STATIC, _("Device configuration string :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer14->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DriverConfigurationString = new wxTextCtrl;
  m_DriverConfigurationString->Create( m_panelCanal, ID_DriverConfigurationString, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
  itemBoxSizer14->Add(m_DriverConfigurationString, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton17 = new wxButton;
  itemButton17->Create( m_panelCanal, ID_BUTTON_VSCP_SET_CONFIGURATION, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemBoxSizer14->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer14->Add(20, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer19, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText20 = new wxStaticText;
  itemStaticText20->Create( m_panelCanal, wxID_STATIC, _("Flags :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer19->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_DriverFlags = new wxTextCtrl;
  m_DriverFlags->Create( m_panelCanal, ID_DriverFlags, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
  itemBoxSizer19->Add(m_DriverFlags, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer19->Add(302, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  GetBookCtrl()->AddPage(m_panelCanal, _("CANAL Driver"));

  m_panelServer = new wxPanel;
  m_panelServer->Create( GetBookCtrl(), ID_PANEL_INTERFACE_REMOTE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  m_panelServer->SetName(wxT("server"));
  wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
  m_panelServer->SetSizer(itemBoxSizer24);

  wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer24->Add(itemBoxSizer25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer26, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText27 = new wxStaticText;
  itemStaticText27->Create( m_panelServer, wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer26->Add(itemStaticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerDescription = new wxTextCtrl;
  m_RemoteServerDescription->Create( m_panelServer, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(400, -1), 0 );
  itemBoxSizer26->Add(m_RemoteServerDescription, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer29, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText30 = new wxStaticText;
  itemStaticText30->Create( m_panelServer, wxID_STATIC, _("Server URL:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer29->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerURL = new wxTextCtrl;
  m_RemoteServerURL->Create( m_panelServer, ID_RemoteServerURL, _("127.0.0.1:9598"), wxDefaultPosition, wxSize(400, -1), 0 );
  itemBoxSizer29->Add(m_RemoteServerURL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer32, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText33 = new wxStaticText;
  itemStaticText33->Create( m_panelServer, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer32->Add(itemStaticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerUsername = new wxTextCtrl;
  m_RemoteServerUsername->Create( m_panelServer, ID_RemoteServerUsername, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer32->Add(m_RemoteServerUsername, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer32->Add(200, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer36, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText37 = new wxStaticText;
  itemStaticText37->Create( m_panelServer, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer36->Add(itemStaticText37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteServerPassword = new wxTextCtrl;
  m_RemoteServerPassword->Create( m_panelServer, ID_RemoteServerPassword, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD );
  itemBoxSizer36->Add(m_RemoteServerPassword, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer36->Add(200, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer40 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer40, 0, wxALIGN_RIGHT|wxALL, 1);
  m_fullLevel2 = new wxCheckBox;
  m_fullLevel2->Create( m_panelServer, ID_CHECKBOX2, _("Full Level II"), wxDefaultPosition, wxDefaultSize, 0 );
  m_fullLevel2->SetValue(false);
  itemBoxSizer40->Add(m_fullLevel2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer40->Add(320, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer43, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText44 = new wxStaticText;
  itemStaticText44->Create( m_panelServer, wxID_STATIC, _("Interface to use on server (leave blank to send events on all interfaces)"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText44->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemBoxSizer43->Add(itemStaticText44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemBoxSizer43->Add(160, 1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer46 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer46, 0, wxALIGN_RIGHT|wxALL, 1);
  wxStaticText* itemStaticText47 = new wxStaticText;
  itemStaticText47->Create( m_panelServer, wxID_STATIC, _("Interface name:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer46->Add(itemStaticText47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_RemoteInterfaceName = new wxTextCtrl;
  m_RemoteInterfaceName->Create( m_panelServer, ID_TEXTCTRL_INTERFACE_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
  itemBoxSizer46->Add(m_RemoteInterfaceName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer46->Add(200, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer24->Add(itemBoxSizer50, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 15);
  wxButton* itemButton51 = new wxButton;
  itemButton51->Create( m_panelServer, ID_BUTTON_SET_FILTER, _("Set Filter"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer50->Add(1, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_btnTestConnection = new wxButton;
  m_btnTestConnection->Create( m_panelServer, ID_BUTTON_TEST_INTERFACE, _("Test connection"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(m_btnTestConnection, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer50->Add(1, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_btnGetInterfaces = new wxButton;
  m_btnGetInterfaces->Create( m_panelServer, ID_BUTTON, _("Get interfaces"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(m_btnGetInterfaces, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer50->Add(30, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  GetBookCtrl()->AddPage(m_panelServer, _("Remote VSCP server"));

    ////@end dlgVscpInterfaceSettings content construction

  
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgVscpInterfaceSettings::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgVscpInterfaceSettings::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin dlgVscpInterfaceSettings bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end dlgVscpInterfaceSettings bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgVscpInterfaceSettings::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin dlgVscpInterfaceSettings icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end dlgVscpInterfaceSettings icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonVscpDriverSetPathClick
//

void dlgVscpInterfaceSettings::OnButtonVscpDriverSetPathClick( wxCommandEvent& event )
{
    wxString filename = wxFileSelector(_("Choose a CANAL driver"));
    if ( !filename.empty() ) {
        m_PathToDriver->SetValue( filename );
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonVscpSetConfigurationClick
//

void dlgVscpInterfaceSettings::OnButtonVscpSetConfigurationClick( wxCommandEvent& event )
{
    CDllWrapper dll;
    CCanalConfObj conf;
    foundMetods meth;
    wxString strDrvInfo;
    wxString path = m_PathToDriver->GetValue();

    if ( 0 == path.Length() ) {
        wxMessageBox( _( "Must have a valid path to the driver." ) );
        return;
    }

    // Check that the file exists
    if ( !( ::wxFileExists( path ) ) ) {
        wxMessageBox( _( "The path does not point to a valid file." ) );
        return;
    }

    if ( CANAL_ERROR_SUCCESS != dll.loadGetDriverInfo( path, strDrvInfo, &meth ) ) {
        wxMessageBox( _( "Sorry! This driver does not appears to have a stored description of how it should be configurated. Ask the driver maker to include one!" ) );
        return;
    }

    wxString wxstr = strDrvInfo;  
    vscp_base64_wxdecode( wxstr );

    // OK we have valid data - Parse it
    wxString driverinfo = wxstr;
    if ( !conf.parseDriverInfo( driverinfo ) ) {
        wxMessageBox( _( "Failed to parse the configuration data." ) );
        return;
    }

    // Start the wizard
    wxString resultConfString;
    wxString strDrvConf = m_DriverConfigurationString->GetValue();
    uint32_t resultFlags = 0;
    if ( !conf.runWizard( this,
                            strDrvConf,
                            vscp_readStringValue( m_DriverFlags->GetValue() ),
                            resultConfString, 
                            &resultFlags ) ) {
        wxMessageBox( _( "Failed to run configuration wizard." ) );
        return;
    }

    m_DriverConfigurationString->SetValue( resultConfString );
    wxString strWrk = wxString::Format( _( "%lu" ), resultFlags );
    m_DriverFlags->SetValue( strWrk );

    event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonTestInterfaceClick
//

void dlgVscpInterfaceSettings::OnButtonTestInterfaceClick( wxCommandEvent& event )
{
    VscpRemoteTcpIf tcpif;
    wxString wxstr;

    m_btnTestConnection->Enable( false );

    // If server username is given and no password is entered we ask for it.
    if ( m_RemoteServerPassword->GetValue().IsEmpty() && !m_RemoteServerUsername->GetValue().IsEmpty() ) {
        wxstr = ::wxGetTextFromUser( _("Please enter password"), 
            _("Connection Test") );
    }
    else {
        wxstr = m_RemoteServerPassword->GetValue();
    }

    wxBusyCursor busy;

    long rv = 
        tcpif.doCmdOpen( m_RemoteServerURL->GetValue().ToStdString(),
                            m_RemoteServerUsername->GetValue().ToStdString(),
                            wxstr.ToStdString() );
    if ( VSCP_ERROR_SUCCESS == rv ) {
        tcpif.doCmdClose();
        wxMessageBox(_("Successful connect to server."), _("Connection Test"), wxICON_INFORMATION );
        /*rv = tcpif.doCmdNOOP();
        if ( CANAL_ERROR_SUCCESS == rv ) {
            wxMessageBox(_("Successful connect to server."), _("Connection Test"), wxICON_INFORMATION );
        }
        else {
            wxMessageBox(_("Failed do command on server (connected OK)."), _("Connection Test"), wxICON_STOP );
        }*/
    }
    else {
        wxMessageBox(_("Failed to connect to server."), _("Connection Test"), wxICON_STOP );
        tcpif.doCmdClose();
    }

    m_btnTestConnection->Enable( true );

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonGetInterfacesClick
//

void dlgVscpInterfaceSettings::OnButtonGetInterfacesClick( wxCommandEvent& event )
{
    VscpRemoteTcpIf tcpif;
    wxString wxstr;

    m_btnGetInterfaces->Enable( false );

    
    // If server username is given and no password is entered we ask for it.
    if ( m_RemoteServerPassword->GetValue().IsEmpty() && 
        !m_RemoteServerUsername->GetValue().IsEmpty() ) {
            wxstr = ::wxGetTextFromUser( _("Please enter password"), 
                _("Password is needed") );
    }
    else {
        wxstr = m_RemoteServerPassword->GetValue();
    }

    wxBusyCursor busy;

    long rv = tcpif.doCmdOpen( m_RemoteServerURL->GetValue().ToStdString(),
                                m_RemoteServerUsername->GetValue().ToStdString(),
                                wxstr.ToStdString() );

    if ( VSCP_ERROR_SUCCESS == rv ) {

        // Get the interface list
        std::deque<std::string> array;
        tcpif.doCmdInterfaceList( array );

        // Close the channel
        tcpif.doCmdClose();

        if ( array.size() ) {

            dlgSelectDaemonInterface dlg( this );
            for ( unsigned int i=0; i<array.size(); i++ ) {

                wxStringTokenizer tkz( array[i], _(",") );
                wxString strOrdinal = tkz.GetNextToken();
                wxString strType = tkz.GetNextToken();
                wxString strGUID = tkz.GetNextToken();
                wxString strDescription = tkz.GetNextToken();
                wxString strLine = strGUID;
                strLine += _(" ");
                strLine += _(" Type = ");
                strLine +=  strType;
                strLine += _(" - ");
                strLine += strDescription;

                dlg.m_ctrlListInterfaces->Append( strLine );
            }

            if ( wxID_OK == dlg.ShowModal() ) {

                int selidx;
                if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

                    wxStringTokenizer tkz( array[selidx], _(",") );
                    wxString strOrdinal = tkz.GetNextToken();
                    wxString strType = tkz.GetNextToken();
                    wxString strGUID = tkz.GetNextToken();

                    // Name of interface
                    wxString strifName = tkz.GetNextToken();
                    int pos;
                    if ( wxNOT_FOUND != ( pos = strifName.Find(_("|") ) ) ) {
                        strifName = strifName.Left( pos );
                        strifName.Trim();
                    }

                    m_RemoteInterfaceName->ChangeValue( strifName );
                    
                }
            }

        }
        else {
            wxMessageBox(_("No interfaces found!"));
        }

    }
    else {
        wxMessageBox(_("Failed to connect to server."), _("Get daemon interfaces"), wxICON_STOP );
    }

    m_btnGetInterfaces->Enable( true );

    event.Skip( false );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_FILTER
//

void dlgVscpInterfaceSettings::OnButtonSetFilterClick( wxCommandEvent& event )
{
    dlgVSCPFilter dlg( this );

    dlg.setFilter( &m_vscpfilter );
    if ( wxID_OK == dlg.ShowModal() ) {
        dlg.getFilter( &m_vscpfilter );
    }

    event.Skip(); 
}




