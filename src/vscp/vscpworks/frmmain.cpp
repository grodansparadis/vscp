/////////////////////////////////////////////////////////////////////////////
// Name:        frmmain.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 16 Apr 2007 18:19:49 CEST
//       
// Copyright:   (C) 2007-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
#pragma implementation "frmmainnomidi.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "merlin.h"

#include <wx/stdpaths.h>
#include <wx/dir.h>

#include "vscpworks.h"
#include "frmmain.h"
#include "dlgnewcanalsession.h"
#include "dlgnewvscpsession.h"
#include "frmcanalsession.h"
#include "frmvscpsession.h"
#include "frmdeviceconfig.h"
#include "frmscanfordevices.h"
#include "dlgconfiguration.h"
#include "mdf.h"
#include "dlgabout.h"
#include "frmmdfeditor.h"
#include "devicebootloaderwizard.h"
#include "frmdaemondecisionmartrixeditor.h"
#include "frmdaemonvariableeditor.h"
#include "frmsimpleuserinterfacegenerator.h"
#include "frmmain_images.h"

extern appConfiguration g_Config;

// frmMain type definition
IMPLEMENT_CLASS( frmMain, wxFrame )

// frmMain event table definition
BEGIN_EVENT_TABLE( frmMain, wxFrame )
	EVT_CLOSE( frmMain::OnCloseWindow )
	EVT_PAINT( frmMain::OnPaint )
	EVT_MENU( ID_MENUITEM_OPEN_VSCP_SESSION, frmMain::OnMenuitemOpenVscpSessionClick )
	EVT_MENU( ID_MENUITEM_DEVICE_CONFIGURATION, frmMain::OnMenuitemOpenConfigSessionClick )
	EVT_MENU( ID_MENUITEM_MDF_EDITOR, frmMain::OnMenuitemMdfEditorClick )
	EVT_MENU( ID_MENUITEM_DM_EDITOR, frmMain::OnMenuitemOpenDaemonDMEditorClick )
	EVT_MENU( ID_MENUITEM_VARIABLE_EDITOR, frmMain::OnMenuitemOpenDaemonVariableEditorClick )
	EVT_MENU( ID_MENUITEM_SCAN, frmMain::OnMenuitemScanClick )
	EVT_MENU( ID_MENUITEM_BOOTLOADER_WIZARD, frmMain::OnMenuitemBootloaderWizardClick )
	EVT_MENU( ID_MENUITEM_SIMPLE_UI_DESIGNER, frmMain::OnMenuitemOpenSimpleUIdesignerClick )
	EVT_MENU( ID_MENUITEM_MERLIN, frmMain::OnMenuitemMerlinClick )
	EVT_MENU( ID_MENU_VSCPWORKS_EXIT, frmMain::OnMenuitemAppExitClick )
	EVT_MENU( ID_MENUITEM_CONFIGURATION, frmMain::OnMenuitemConfigurationClick )
	EVT_MENU( ID_MENUITEM33, frmMain::OnMenuitemHelpClick )
	EVT_MENU( ID_MENUITEM34, frmMain::OnMenuitemFaqClick )
	EVT_MENU( ID_MENUITEM35, frmMain::OnMenuitemShortcutsClick )
	EVT_MENU( ID_MENUITEM_THANKS, frmMain::OnMenuitemThanksClick )
	EVT_MENU( ID_MENUITEM_CREDITS, frmMain::OnMenuitemCrediitsClick )
	EVT_MENU( ID_MENUITEM_VSCP_SITE, frmMain::OnMenuitemVSCPSiteClick )
	EVT_MENU( ID_MENUITEM_ABOUT, frmMain::OnMenuitemAboutClick )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////
// frmMain ctor
//

frmMain::frmMain()
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
// frmMain ctor
//

frmMain::frmMain( wxWindow* parent, 
                    wxWindowID id, 
                    const wxString& caption, 
                    const wxPoint& pos, 
                    const wxSize& size, 
                    long style )
{
	Init();
	Create( parent, id, caption, pos, size, style );
}

///////////////////////////////////////////////////////////////////////////////
// Create
//

bool frmMain::Create( wxWindow* parent, 
                            wxWindowID id, 
                            const wxString& caption, 
                            const wxPoint& pos, 
                            const wxSize& size, 
                            long style )
{
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// frmMain dtor
//

frmMain::~frmMain()
{

}

///////////////////////////////////////////////////////////////////////////////
// Init
//

void frmMain::Init()
{
    if ( !::wxDirExists( wxStandardPaths::Get().GetUserDataDir() ) ) {
        if ( !::wxMkdir( wxStandardPaths::Get().GetUserDataDir() ) ) {
            wxMessageBox(_("Failed to create directory. ") + wxStandardPaths::Get().GetUserDataDir() );
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// CreateControls
//

void frmMain::CreateControls()
{    
    frmMain* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;

    itemMenu3->Append( ID_MENUITEM_OPEN_VSCP_SESSION,
                        _("VSCP client window..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_DEVICE_CONFIGURATION, 
                        _("VSCP device configuration window..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_MDF_EDITOR, 
                        _("VSCP MDF Editor..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_DM_EDITOR, 
                        _("VSCP Daemon Decision Matrix Editor..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_VARIABLE_EDITOR, 
                        _("VSCP Daemon Variable Editor..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_SCAN, 
                        _("Scan for devices..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_BOOTLOADER_WIZARD, 
                        _("VSCP bootloader wizard..."), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_SIMPLE_UI_DESIGNER, 
                        _("VSCP Simple UI deigner..."), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENU_VSCPWORKS_EXIT, 
                        _("Exit"), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    
    // VSCP Menu
    menuBar->Append(itemMenu3, _("VSCP"));
    wxMenu* itemMenu17 = new wxMenu;
    itemMenu17->Append( ID_MENUITEM_CONFIGURATION, 
                            _("Configuration..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu17->AppendSeparator();
    itemMenu17->Append( ID_MENUITEM_MDF_EDITOR, 
                            _("MDF Editor"), 
                            wxEmptyString, 
                            wxITEM_NORMAL );
    
    // Tools menu
    menuBar->Append(itemMenu17, _("Tools"));
    wxMenu* itemMenu21 = new wxMenu;
    itemMenu21->Append( ID_MENUITEM33, 
                            _("VSCP-Works Help"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->Append( ID_MENUITEM34, 
                            _("Frequently Asked Questions"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->Append( ID_MENUITEM35, 
                            _("Keyboard shortcuts"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->AppendSeparator();
    itemMenu21->Append( ID_MENUITEM_THANKS, 
                            _("Thanks..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->Append( ID_MENUITEM_CREDITS, 
                            _("Credits..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->AppendSeparator();
    itemMenu21->Append( ID_MENUITEM_VSCP_SITE, 
                            _("Go to VSCP site"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu21->AppendSeparator();
    itemMenu21->Append( ID_MENUITEM_ABOUT, 
                            _("About"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    
    menuBar->Append (itemMenu21, _("Help") );

    itemFrame1->SetMenuBar( menuBar );

    // Statusbar
    wxStatusBar* itemStatusBar32 = new wxStatusBar;
    itemStatusBar32->Create( itemFrame1, 
                                ID_STATUSBAR, 
                                wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar32->SetFieldsCount(2);
    itemFrame1->SetStatusBar( itemStatusBar32 );

    // Toolbar
    wxToolBar* itemToolBar33 = CreateToolBar( wxTB_FLAT | wxTB_HORIZONTAL | wxTB_NODIVIDER, 
                                                ID_TOOLBAR );
    wxBitmap itemtool34Bitmap( itemFrame1->GetBitmapResource( wxT("open.xpm") ) );
    wxBitmap itemtool34BitmapDisabled;
    itemToolBar33->AddTool( ID_TOOL, 
                                wxEmptyString, 
                                itemtool34Bitmap, 
                                itemtool34BitmapDisabled, 
                                wxITEM_NORMAL, 
                                wxEmptyString, 
                                wxEmptyString );
    itemToolBar33->Realize();
    itemFrame1->SetToolBar( itemToolBar33 );

    wxPanel* itemPanel35 = new wxPanel;
    itemPanel35->Create( itemFrame1, 
                            ID_PANEL1, 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanel35->SetBackgroundColour( wxColour(255, 255, 255) );

    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer( wxVERTICAL );
    itemPanel35->SetSizer( itemBoxSizer36 );

    wxStaticBitmap* itemStaticBitmap37 = new wxStaticBitmap;
    itemStaticBitmap37->Create( itemPanel35, 
                                    wxID_STATIC, 
                                    itemFrame1->GetBitmapResource( wxT("../../../docs/vscp/logo/vscp_logo.jpg") ), 
                                    wxDefaultPosition, 
                                    wxSize(151, 212),
                                    0 );
    itemStaticBitmap37->SetBackgroundColour( wxColour(255, 255, 255) );
    itemBoxSizer36->Add( itemStaticBitmap37, 
                            0, 
                            wxALIGN_CENTER_HORIZONTAL | wxALL, 
                            0 );

}

///////////////////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool frmMain::ShowToolTips()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// GetBitmapResource
//

wxBitmap frmMain::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    if ( name == _T("open.xpm") ) {
        wxBitmap bitmap(open_xpm);
        return bitmap;
    }
    else if ( name == _T("../../../docs/vscp/logo/vscp_logo.jpg") ) {
        wxBitmap bitmap(vscp_logo_xpm);
        return bitmap;
    }
  
    return wxNullBitmap;
    
}

///////////////////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon frmMain::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == _T("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }

    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmMain::OnCloseWindow( wxCloseEvent& event )
{
    // Save frame size and position
    wxRect rc = GetRect();
    g_Config.m_xpos = rc.x;  
    g_Config.m_ypos = rc.y;
    g_Config.m_sizeWidth = rc.width;
    g_Config.m_sizeHeight = rc.height;

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnPaint
//

void frmMain::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);

	dc.DrawText(wxT("Version: "), 40, 60); 

	// draw a circle
	dc.SetBrush(*wxGREEN_BRUSH);                    // green filling
	dc.SetPen( wxPen( wxColor(255,0,0), 5 ) );      // 5-pixels-thick red outline
	dc.DrawCircle( wxPoint(200,100), 25 );

	// draw a rectangle
	dc.SetBrush(*wxBLUE_BRUSH); // blue filling
	dc.SetPen( wxPen( wxColor(255,175,175), 10 ) ); // 10-pixels-thick pink outline
	dc.DrawRectangle( 300, 100, 400, 200 );

	// draw a line
	dc.SetPen( wxPen( wxColor(0,0,0), 3 ) );	    // black line, 3 pixels thick
	dc.DrawLine( 300, 100, 700, 300 );			    // draw line across the rectangle

	event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenVscpSessionClick
//

void frmMain::OnMenuitemOpenVscpSessionClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );

    if ( wxID_OK == dlg.ShowModal() ) {

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            frmVSCPSession *subframe = new frmVSCPSession( this );

            if ( NULL != subframe ) {

                if ( 0 != selidx ) {

                    both_interface *pBoth = 
                        (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                    if ( NULL != pBoth ) {  

                        // set type so session window is aware of it
                        subframe->m_CtrlObject.m_interfaceType = pBoth->m_type;

                        if ( INTERFACE_CANAL == pBoth->m_type ) {
                            subframe->SetTitle(_("VSCP Session (CANAL) - ") + 
                                pBoth->m_pcanalif->m_strDescription );
                            subframe->m_CtrlObject.m_ifCANAL.m_strDescription = pBoth->m_pcanalif->m_strDescription;
                            subframe->m_CtrlObject.m_ifCANAL.m_strPath = pBoth->m_pcanalif->m_strPath;
                            subframe->m_CtrlObject.m_ifCANAL.m_strConfig = pBoth->m_pcanalif->m_strConfig;
                            subframe->m_CtrlObject.m_ifCANAL.m_flags= pBoth->m_pcanalif->m_flags;
                        }
                        else if ( INTERFACE_VSCP == pBoth->m_type ) {
                            subframe->SetTitle(_("VSCP Session (TCP/IP)- ") +  pBoth->m_pvscpif->m_strDescription );
                            subframe->m_CtrlObject.m_ifVSCP.m_strDescription = pBoth->m_pvscpif->m_strDescription;
                            subframe->m_CtrlObject.m_ifVSCP.m_strHost = pBoth->m_pvscpif->m_strHost;
                            subframe->m_CtrlObject.m_ifVSCP.m_strUser = pBoth->m_pvscpif->m_strUser;
                            subframe->m_CtrlObject.m_ifVSCP.m_strPassword = pBoth->m_pvscpif->m_strPassword;
                            subframe->m_CtrlObject.m_ifVSCP.m_port = pBoth->m_pvscpif->m_port;
                            memcpy( &subframe->m_CtrlObject.m_ifVSCP.m_vscpfilter, 
                                &pBoth->m_pvscpif->m_vscpfilter, 
                                sizeof( vscpEventFilter ) );
                        }

                        if ( INTERFACE_VSCP == subframe->m_CtrlObject.m_interfaceType ) {

                            // If server username is given and no password is entered we ask for it.
                            if ( subframe->m_CtrlObject.m_ifVSCP.m_strPassword.IsEmpty() && 
                                !subframe->m_CtrlObject.m_ifVSCP.m_strUser.IsEmpty() ) {
                                    subframe->m_CtrlObject.m_ifVSCP.m_strPassword = 
                                        ::wxGetTextFromUser( _("Please enter passeword"), 
                                        _("Connection Test") );
                            }

                        }

                        // Start the worker threads
                        subframe->startWorkerThreads( subframe );
                       
                        // Show the VSCP session windows
                        subframe->Show( true );
                        subframe->Raise();
                        subframe->SetFocus();

                    } // VSCP connection

                } // 0 == selindex
                else {
                    subframe->m_BtnActivateInterface->Show( false );
                    subframe->SetTitle(_("VSCP Session - Unconnected Mode"));
                    subframe->Show( true );
                }

            } // subframe
        }
        else {
            wxMessageBox(_("You have to select an interface to connect to!"),
                _("Open new VSCP session"), 
                wxICON_STOP );
        }

        // Clean up listbox
        dlg.cleanupListbox();

    } // dialog

    event.Skip( false );  
}

///////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenConfigSessionClick
//


void frmMain::OnMenuitemOpenConfigSessionClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );

    // Don't show the "unconnected mode" line
    dlg.m_bShowUnconnectedMode = false;

    if ( wxID_OK == dlg.ShowModal() ) {

        wxBusyCursor wait;

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            frmDeviceConfig *subframe = new frmDeviceConfig( this );
            if ( NULL != subframe ) {

                both_interface *pBoth = 
                    (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                if ( NULL != pBoth ) {  

                    // set type so session window is aware of it
                    subframe->m_interfaceType = pBoth->m_type;

                    if ( INTERFACE_CANAL == pBoth->m_type ) {

						// Hide the Level II checkbox
						subframe->m_bLevel2->Show( false );

                        // Init node id combo
                        wxRect rc = subframe->m_comboNodeID->GetRect();
#ifdef WIN32						
                        rc.SetWidth( 60 );	
#else
						rc.SetWidth( 80 );
#endif						
                        subframe->m_comboNodeID->SetSize( rc );
                        wxArrayString strings;
                        for ( int i=1; i<256; i++ ) {
                            strings.Add( wxString::Format(_("0x%02x"), i ) );
                        }
                        subframe->m_comboNodeID->Append( strings );

                        subframe->m_comboNodeID->SetValue(_("0x01"));
                        subframe->SetTitle(_("VSCP Registers (CANAL) - ") +  pBoth->m_pcanalif->m_strDescription );

                        subframe->m_csw.setInterface( pBoth->m_pcanalif->m_strDescription,
														pBoth->m_pcanalif->m_strPath,
														pBoth->m_pcanalif->m_strConfig,
														pBoth->m_pcanalif->m_flags, 0, 0 );

                        // Connect to device bus
                        subframe->enableInterface();

                        // Move window on top
                        subframe->Raise();

                        // Show the VSCP configuration windows
                        subframe->Show( true );

                    }
                    else if ( INTERFACE_VSCP == pBoth->m_type ) {

                        wxString str;

						// Mark as full Level II interface if it is requested
						if ( pBoth->m_pvscpif->m_bLevel2 ) {
							subframe->m_bLevel2->SetValue( true );
						}
						else {
							subframe->m_bLevel2->SetValue( false );
						}

                        subframe->SetTitle(_("VSCP Registers (TCP/IP)- ") +  
                            pBoth->m_pvscpif->m_strDescription );

                        // If server username is given and no password is entered we ask for it.
                        if ( pBoth->m_pvscpif->m_strPassword.IsEmpty() && 
                            !pBoth->m_pvscpif->m_strUser.IsEmpty() ) {
                                pBoth->m_pvscpif->m_strPassword = 
                                    ::wxGetTextFromUser( _("Please enter password"), 
                                    _("Connection Test") );
                        }
                        
                        // Save interface parameters
						subframe->m_vscpif.m_strDescription = pBoth->m_pvscpif->m_strDescription;
						subframe->m_vscpif.m_strHost = pBoth->m_pvscpif->m_strHost;
						subframe->m_vscpif.m_strUser = pBoth->m_pvscpif->m_strUser;
						subframe->m_vscpif.m_strPassword = pBoth->m_pvscpif->m_strPassword;
						subframe->m_vscpif.m_port = pBoth->m_pvscpif->m_port;
                        subframe->m_vscpif.m_strInterfaceName = pBoth->m_pvscpif->m_strInterfaceName;
						memcpy( subframe->m_vscpif.m_GUID, pBoth->m_pvscpif->m_GUID, 16 );
						memcpy( &subframe->m_vscpif.m_vscpfilter, 
									&pBoth->m_pvscpif->m_vscpfilter, 
									sizeof( vscpEventFilter ) );

                        subframe->m_csw.setInterface( pBoth->m_pvscpif->m_strHost,
                                                        pBoth->m_pvscpif->m_strUser,
                                                        pBoth->m_pvscpif->m_strPassword );

                        // Connect to host
                        if ( subframe->enableInterface() ) {

							// Show the VSCP configuration windows
							subframe->Show( true );

							// Move window on top
							subframe->Raise();
						
						}

                    }

                } // pBoth valid

            } // valid subframe pointer

        } // selection made


        // Clean up listbox
        dlg.cleanupListbox();

    } // dialog

    event.Skip( false );
}

///////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemScanClick
//


void frmMain::OnMenuitemScanClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );

    // Don't show the "unconnected mode" line
    dlg.m_bShowUnconnectedMode = false;

    if ( wxID_OK == dlg.ShowModal() ) {
       
        wxBusyCursor wait;

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            frmScanforDevices *subframe = new frmScanforDevices( this );
            
            if ( NULL != subframe ) {

                both_interface *pBoth = 
                    (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                if ( NULL != pBoth ) {  

					// set type so session window is aware of it
                    subframe->m_interfaceType = pBoth->m_type;

                    if ( INTERFACE_CANAL == pBoth->m_type ) {

                        // Init node id combo
                        //wxRect rc = subframe->m_comboNodeID->GetRect();
                        //rc.SetWidth( 60 );
                        //subframe->m_comboNodeID->SetSize( rc );
                        //for ( int i=1; i<256; i++ ) {
                        //    subframe->m_comboNodeID->Append( wxString::Format(_("0x%02x"), i));
                        //}
                        
						//subframe->m_comboNodeID->SetValue(_("0x01"));
                        subframe->SetTitle(_("VSCP Registers (CANAL) - ") +  pBoth->m_pcanalif->m_strDescription );

						// Transfer some parameters so the configuration window can be
						// opened from the scan window.
						subframe->m_canalif.m_strDescription = pBoth->m_pcanalif->m_strDescription;
						subframe->m_canalif.m_strPath =  pBoth->m_pcanalif->m_strPath;
						subframe->m_canalif.m_strConfig = pBoth->m_pcanalif->m_strConfig;
						subframe->m_canalif.m_flags = pBoth->m_pcanalif->m_flags;

                        subframe->m_csw.setInterface( pBoth->m_pcanalif->m_strDescription,
														pBoth->m_pcanalif->m_strPath,
														pBoth->m_pcanalif->m_strConfig,
														pBoth->m_pcanalif->m_flags, 0, 0 );

                        // Connect to device bus
                        subframe->enableInterface();

                        // Move window on top
                        subframe->Raise();

                        // Show the VSCP configuration windows
                        subframe->Show( true );

                    }

                    else if ( INTERFACE_VSCP == pBoth->m_type ) {

                        wxString str;
                        unsigned char GUID[16];
                        memcpy( GUID, pBoth->m_pvscpif->m_GUID, 16 );

                        // Fill the combo
                        //for ( int i=1; i<256; i++ ) {
                        //    GUID[0] = i;
                        //    writeGuidArrayToString( GUID, str );
                        //    subframe->m_comboNodeID->Append( str );
                        //}

                        //GUID[0] = 0x01;
                        //writeGuidArrayToString( GUID, str );
                        //subframe->m_comboNodeID->SetValue( str );

                        subframe->SetTitle(_("VSCP Registers (TCP/IP)- ") +  
                            pBoth->m_pvscpif->m_strDescription );

                        // If server username is given and no password is entered we ask for it.
                        if ( pBoth->m_pvscpif->m_strPassword.IsEmpty() && 
                            !pBoth->m_pvscpif->m_strUser.IsEmpty() ) {
                                pBoth->m_pvscpif->m_strPassword = 
                                    ::wxGetTextFromUser( _("Please enter password"), 
                                    _("Connection Test") );
                        }

                        // Save interface parameters
						subframe->m_vscpif.m_strDescription = pBoth->m_pvscpif->m_strDescription;
						subframe->m_vscpif.m_strHost = pBoth->m_pvscpif->m_strHost;
						subframe->m_vscpif.m_strUser = pBoth->m_pvscpif->m_strUser;
						subframe->m_vscpif.m_strPassword = pBoth->m_pvscpif->m_strPassword;
						subframe->m_vscpif.m_port = pBoth->m_pvscpif->m_port;
                        subframe->m_vscpif.m_strInterfaceName = pBoth->m_pvscpif->m_strInterfaceName;
						memcpy( subframe->m_vscpif.m_GUID, pBoth->m_pvscpif->m_GUID, 16 );
						memcpy( &subframe->m_vscpif.m_vscpfilter, 
									&pBoth->m_pvscpif->m_vscpfilter, 
									sizeof( vscpEventFilter ) );
                        
                        // Set information about interface we search on
                        str = _("Searching on interface: ");
                        str += pBoth->m_pvscpif->m_strInterfaceName;
                        subframe->m_labelInterface->SetLabel(str);

                        //subframe->m_csw = m_interfaceType;
                        subframe->m_csw.setInterface( pBoth->m_pvscpif->m_strHost,
														pBoth->m_pvscpif->m_strUser,
														pBoth->m_pvscpif->m_strPassword );

                        // Connect to host
                        subframe->enableInterface();

                        // Show the VSCP configuration windows
                        subframe->Show( true );

                        // Move window on top
                        subframe->Raise();

                    }

                } // pBoth valid

            } // valid subframe pointer

        } // selection made

        // Clean up listbox
        dlg.cleanupListbox();

    } // dialog

    event.Skip( false );
}

///////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAppExitClick
//


void frmMain::OnMenuitemAppExitClick( wxCommandEvent& event )
{
    Close();
    event.Skip( false );
}

///////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAboutClick
//

void frmMain::OnMenuitemAboutClick( wxCommandEvent& event )
{
    dlgAbout dlg(this);
    if ( wxID_OK == dlg.ShowModal() ) {

    }
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemConfigurationClick
//

void frmMain::OnMenuitemConfigurationClick( wxCommandEvent& event )
{
    dlgConfiguration dlg( this );

    if ( wxID_OK == dlg.ShowModal() ) {
        dlg.getDialogData();
    }
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpClick
// 

void frmMain::OnMenuitemHelpClick( wxCommandEvent& event )
{
    // vscp works main window / help / help [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemFaqClick
// 

void frmMain::OnMenuitemFaqClick( wxCommandEvent& event )
{
    // vscp works main window / help / faq [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemShortcutsClick
//

void frmMain::OnMenuitemShortcutsClick( wxCommandEvent& event )
{
    // vscp works main window / help / shortcuts [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemThanksClick
//

void frmMain::OnMenuitemThanksClick( wxCommandEvent& event )
{
    // vscp works main window / help / thanks [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW );
    event.Skip( false ); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemCrediitsClick
//

void frmMain::OnMenuitemCrediitsClick( wxCommandEvent& event )
{
    // vscp works main window / help / credits [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVSCPSiteClick
//

void frmMain::OnMenuitemVSCPSiteClick( wxCommandEvent& event )
{
    // vscp works main window / help / vscp site [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemBootloaderWizardClick
//

void frmMain::OnMenuitemBootloaderWizardClick( wxCommandEvent& event )
{
    DeviceBootloaderwizard *wizard = new DeviceBootloaderwizard( this );
    wizard->Run();

    event.Skip( false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemMdfEditorClick
//

void frmMain::OnMenuitemMdfEditorClick( wxCommandEvent& event )
{
    frmMDFEditor *subframe = new frmMDFEditor( this );
    if ( NULL != subframe ) {
        subframe->Show();
    }

    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemMerlinClick
//

void frmMain::OnMenuitemMerlinClick( wxCommandEvent& event )
{
	Merlin* window = new Merlin(this);
	window->Show(true);

	event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenSimpleUIdesignerClick
//

void frmMain::OnMenuitemOpenSimpleUIdesignerClick( wxCommandEvent& event )
{
	SimpleUserInterfaceGenerator* window = new SimpleUserInterfaceGenerator(this);
	window->Show(true);

	event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenDaemonDMEditorClick
//

void frmMain::OnMenuitemOpenDaemonDMEditorClick( wxCommandEvent& event )
{
	DaemonDecisionMartrixEditor* window = new DaemonDecisionMartrixEditor(this);
	window->Show( true );

	event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenDaemonVaraibleEditorClick
//

void frmMain::OnMenuitemOpenDaemonVariableEditorClick( wxCommandEvent& event )
{
	DaemonVariableEditor* window = new DaemonVariableEditor(this);
	window->Show(true);

	event.Skip( false );
}




