/////////////////////////////////////////////////////////////////////////////
// Name:        frmmain.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 16 Apr 2007 18:19:49 CEST
//       
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

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/socket.h"

#ifdef WIN32
//#include <sys/types.h>   // for type definitions 
//#include <winsock2.h>
//#include <ws2tcpip.h>    // for win socket structs 
#endif

#include "merlin.h"

#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/splash.h>

#include <mdf.h>
#include <version.h>
#include "vscpworks.h"
#include "frmmain.h"
#include "dlgnewcanalsession.h"
#include "dlgnewvscpsession.h"
#include "frmcanalsession.h"
#include "frmvscpsession.h"
#include "frmdeviceconfig.h"
#include "frmscanfordevices.h"
#include "dlgconfiguration.h"
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
    EVT_TREE_SEL_CHANGED( ID_TREECTRL, frmMain::OnTreectrlSelChanged )
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
// RenderTimer CTor/DTor
//

RenderTimer::RenderTimer( frmMain *pwnd, worksMulticastThread *pThread ) : wxTimer()
{
    m_nLastKnownNodes = 0;
    m_nLastKnownServers = 0;

    RenderTimer::m_pwnd = pwnd;
    m_multicastThread  = pThread;
}

///////////////////////////////////////////////////////////////////////////////
// Notify
//

void RenderTimer::Notify()
{
    bool bRefresh = false;
    
    /* TODO
    m_multicastThread->m_knownNodes.m_mutexKnownNodes.Lock();
    
    if ( m_nLastKnownNodes < m_multicastThread->m_knownNodes.m_nodes.size() ) {

        bRefresh = true;    // We should update stuff

        VSCP_HASH_KNOWN_NODES::iterator it;
        for ( it = m_multicastThread->m_knownNodes.m_nodes.begin(); 
                it != m_multicastThread->m_knownNodes.m_nodes.end(); ++it ) {
            wxString key = it->first;
            CNodeInformation *pNodeInfo = it->second;
            
            if ( NULL != pNodeInfo ) {

                if ( !pNodeInfo->m_bUpdated ) {

                    nodeClientData *pNode = new nodeClientData( CLIENT_NODE  );
                    if ( NULL != pNode ) {

                        pNode->m_nodeInformation = *pNodeInfo;

                        if ( pNodeInfo->m_strNodeName.Length() ) {
                            m_pwnd->m_nodeTree->AppendItem( m_pwnd->m_moduleNodeItem,
                                                            pNodeInfo->m_strNodeName,
                                                            MDF_EDITOR_SUB_ITEM,
                                                            -1,
                                                            pNode );
                        }
                        else {
                            m_pwnd->m_nodeTree->AppendItem( m_pwnd->m_moduleNodeItem,
                                                            _( "Node without name." ),
                                                            MDF_EDITOR_SUB_ITEM,
                                                            -1,
                                                            pNode );
                        }

                        pNodeInfo->m_bUpdated = true;

                    }
                }
            }

        }
        
        // Update numbers
        m_nLastKnownNodes = m_multicastThread->m_knownNodes.m_nodes.size();    
        
    }

    if ( m_nLastKnownServers < m_multicastThread->m_knownNodes.m_servers.size() ) {

        bRefresh = true;    // We should update stuff

        VSCP_HASH_KNOWN_SERVERS::iterator it;
        for ( it = m_multicastThread->m_knownNodes.m_servers.begin(); 
                it != m_multicastThread->m_knownNodes.m_servers.end(); ++it ) {
            wxString key = it->first;
            CVSCPServerInformation *pNodeInfo = it->second;
 
            if ( NULL != pNodeInfo ) {

                if ( !pNodeInfo->m_bUpdated ) {

                    nodeClientData *pServer = new nodeClientData( CLIENT_SERVER  );
                    if ( NULL != pServer ) {

                        pServer->m_serverInformation = *pNodeInfo;

                        if ( pNodeInfo->m_nameOfServer.Length() ) {
                            m_pwnd->m_nodeTree->AppendItem( m_pwnd->m_moduleServerItem,
                                                            pNodeInfo->m_nameOfServer,
                                                            MDF_EDITOR_SUB_ITEM,
                                                            -1,
                                                            pServer );
                        }
                        else {
                            m_pwnd->m_nodeTree->AppendItem( m_pwnd->m_moduleServerItem,
                                                            _( "Server without name." ),
                                                            MDF_EDITOR_SUB_ITEM,
                                                            -1,
                                                            pServer );
                        }

                        pNodeInfo->m_bUpdated = true;

                    }
                }
            }

        }

        // Update numbers
        m_nLastKnownServers = m_multicastThread->m_knownNodes.m_servers.size();

    }

    m_multicastThread->m_knownNodes.m_mutexKnownNodes.Unlock();

    // Refresh the window if necessary
    if ( bRefresh ) {
        // Refresh the display
        m_pwnd->Refresh();
    }
   */
}

///////////////////////////////////////////////////////////////////////////////
// start
//

void RenderTimer::start()
{
    wxTimer::Start( 2000 );
}



///////////////////////////////////////////////////////////////////////////////
// frmMain ctor
//

frmMain::frmMain()
{
    m_timerDiscovery = new RenderTimer( this, wxGetApp().m_pmulticastWorkerThread );

    Init();
    m_timerDiscovery->start();
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
    // Create the timer object
    m_timerDiscovery = new RenderTimer( this, wxGetApp().m_pmulticastWorkerThread );
    
    Init();
    Create( parent, id, caption, pos, size, style );
    m_timerDiscovery->start();
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
    SetIcon( GetIconResource( wxT("../../../docs/vscp/logo/fatbee.ico") ) );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// frmMain dtor
//

frmMain::~frmMain()
{
    delete m_timerDiscovery;
}

///////////////////////////////////////////////////////////////////////////////
// Init
//

void frmMain::Init()
{
    if ( !::wxDirExists( wxStandardPaths::Get().GetUserDataDir() ) ) {
        if ( !::wxMkdir( wxStandardPaths::Get().GetUserDataDir() ) ) {
            wxMessageBox( _("Failed to create directory. ") + 
                            wxStandardPaths::Get().GetUserDataDir() );
        }
    }
    
}


///////////////////////////////////////////////////////////////////////////////
// CreateControls
//

void frmMain::CreateControls()
{    
    frmMain* itemMainFrame = this;

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
    /*itemMenu3->Append( ID_MENUITEM_DM_EDITOR, 
                        _("VSCP Daemon Decision Matrix Editor..."), 
                        wxEmptyString, wxITEM_NORMAL);*/
    /*itemMenu3->Append( ID_MENUITEM_VARIABLE_EDITOR, 
                        _("VSCP Daemon Variable Editor..."), 
                        wxEmptyString, wxITEM_NORMAL);*/
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_SCAN, 
                        _("Scan for devices..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append( ID_MENUITEM_BOOTLOADER_WIZARD, 
                        _("VSCP bootloader wizard..."), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    /*itemMenu3->Append( ID_MENUITEM_SIMPLE_UI_DESIGNER, 
                        _("VSCP Simple UI designer..."), 
                        wxEmptyString, 
                        wxITEM_NORMAL);*/
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENU_VSCPWORKS_EXIT, 
                        _("Exit"), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    
    // VSCP Menu
    menuBar->Append(itemMenu3, _("VSCP"));
    wxMenu* itemMenu17 = new wxMenu;
    itemMenu17->Append( ID_MENUITEM_CONFIGURATION, 
                            _("Settings..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    
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

    itemMainFrame->SetMenuBar( menuBar );

    // Statusbar
    m_pitemStatusBar = new wxStatusBar;
    m_pitemStatusBar->Create( itemMainFrame,
                                ID_STATUSBAR, 
                                wxST_SIZEGRIP|wxNO_BORDER );
    //m_pitemStatusBar->SetFieldsCount( 2 );
    itemMainFrame->SetStatusBar( m_pitemStatusBar );

    // Toolbar
    wxToolBar* itemToolBar = 
                CreateToolBar( wxTB_FLAT | 
                                    wxTB_HORIZONTAL | 
                                    wxTB_NODIVIDER, 
                                    ID_TOOLBAR );
    wxBitmap itemtool34Bitmap( itemMainFrame->GetBitmapResource( wxT("open.xpm") ) );
    wxBitmap itemtool34BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL,
                                wxEmptyString, 
                                itemtool34Bitmap, 
                                itemtool34BitmapDisabled, 
                                wxITEM_NORMAL, 
                                wxEmptyString, 
                                wxEmptyString );
    itemToolBar->Realize();
    itemMainFrame->SetToolBar( itemToolBar );

    
    wxPanel* itemPanel = new wxPanel;
    itemPanel->Create( itemMainFrame,
                            ID_PANEL1, 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanel->SetBackgroundColour( wxColour( 255, 255, 255 ) );

    
    wxBoxSizer* itemSizerVertical = new wxBoxSizer( wxVERTICAL );
    itemPanel->SetSizer( itemSizerVertical );

    
    // Header for servers
    wxStaticText* itemStaticTextTop = new wxStaticText;
    itemStaticTextTop->Create( itemPanel,
                               wxID_STATIC,
                               _( "Discovered servers and nodes" ),
                               wxDefaultPosition,
                               wxDefaultSize,0 );
    itemStaticTextTop->SetForegroundColour( wxColour( 0, 128, 0 ) );
#if  wxCHECK_VERSION(2, 9, 5)
    itemStaticTextTop->SetFont( wxFont( wxFontInfo(10).FaceName("Tahoma").Bold() ) );
#else    
    itemStaticTextTop->SetFont( wxFont( 10, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif    
    itemSizerVertical->Add( itemStaticTextTop, 0, wxALIGN_LEFT | wxALL, 5 );

    m_nodeTree = new wxTreeCtrl;
    m_nodeTree->Create( itemPanel, 
                            ID_TREECTRL, 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxTR_HAS_BUTTONS | 
                                wxTR_FULL_ROW_HIGHLIGHT | 
                                wxTR_LINES_AT_ROOT | 
                                wxTR_ROW_LINES | 
                                wxTR_SINGLE );
    itemSizerVertical->Add( m_nodeTree, 10, wxGROW | wxALL, 2 );

    m_htmlInfoWnd = new wxHtmlWindow;
    m_htmlInfoWnd->Create( itemPanel, 
                            ID_HTMLWINDOW2, 
                            wxDefaultPosition, 
                            wxSize( -1, 150 ), 
                            wxHW_SCROLLBAR_AUTO | 
                                wxSUNKEN_BORDER | 
                                wxHSCROLL | 
                                wxVSCROLL );
    m_htmlInfoWnd->SetPage( _( "<html><h4>Node information</h4>This area will contain extended information about known (discovered) nodes. This is work in progress so information is sparse (and may be wrong) at the moment. Click on a discovered node to display info about it in this area.</html>" ) );
    itemSizerVertical->Add( m_htmlInfoWnd, 0, wxGROW | wxALL, 5 );
    
    // Connect events and objects
    m_nodeTree->Connect( ID_TREECTRL, 
                            wxEVT_LEFT_DOWN, 
                            wxMouseEventHandler( frmMain::OnLeftDown ), 
                            NULL, 
                            this );
    m_nodeTree->Connect( ID_TREECTRL, 
                            wxEVT_LEFT_DCLICK, 
                            wxMouseEventHandler( frmMain::OnLeftDClick ), 
                            NULL, 
                            this );

    wxImageList* itemImageList = new wxImageList( 16, 16, true, 5 );
    {
        wxImage icon0( Home_xpm );
        icon0.Rescale( 16, 16 );
        itemImageList->Add( icon0 );
        
        wxImage icon1( Folder_Add_xpm );
        icon1.Rescale( 16, 16 );
        itemImageList->Add( icon1 );
        
        wxImage icon2( Info_xpm );
        icon2.Rescale( 16, 16 );
        itemImageList->Add( icon2 );
        
        wxImage icon3( copy_xpm );
        icon3.Rescale( 16, 16 );
        itemImageList->Add( icon3 );
        
        wxImage icon4( copy_xpm );
        icon4.Rescale( 16, 16 );
        itemImageList->Add( icon4 );
    }

    m_nodeTree->AssignImageList( itemImageList );

    addDefaultContent();
    
    // Create Logo
    /*
    m_pStaticBitmapLogo = new wxStaticBitmap;
    m_pStaticBitmapLogo->Create( itemPanel,
                                    wxID_STATIC, 
                                    itemMainFrame->GetBitmapResource( wxT("../../../docs/vscp/logo/vscp_logo.jpg") ),
                                    wxDefaultPosition, 
                                    wxSize(151, 212),
                                    0 );
    m_pStaticBitmapLogo->SetBackgroundColour( wxColour( 255, 255, 255 ) );
    itemSizerVertical->Add( m_pStaticBitmapLogo,
                            0, 
                            wxALIGN_CENTER_HORIZONTAL | wxALL, 
                            0 );
    */
    wxString strVersion = _( VSCPD_COPYRIGHT );
    strVersion += _( " - " );
    strVersion += _( VSCPD_DISPLAY_VERSION );
    m_pitemStatusBar->SetStatusText( strVersion );
    
    wxBitmap bitmap( itemMainFrame->GetBitmapResource( wxT("vscp_logo_xpm") ) );
    /*if ( bitmap.LoadFile("splash16.png", wxBITMAP_TYPE_PNG ) ) {
        wxSplashScreen* splash = new wxSplashScreen( GetBitmapResource( _("../../../docs/vscp/logo/vscp_logo.jpg") ),
                                                        wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
                                                        3000, 
                                                        NULL, 
                                                        -1, 
                                                        wxDefaultPosition, 
                                                        wxDefaultSize,
                                                        wxBORDER_SIMPLE | wxSTAY_ON_TOP );
    }
    wxYield(); 
    */    
}

////////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool frmMain::ShowToolTips()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon frmMain::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == _T("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_xpm);
        return icon;
    }

    return wxNullIcon;
}

////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmMain::OnCloseWindow( wxCloseEvent& event )
{
    m_timerDiscovery->Stop();

    // Save frame size and position
    wxRect rc = GetRect();
    g_Config.m_xposMainFrame = rc.x;  
    g_Config.m_yposMainFrame = rc.y;
    g_Config.m_sizeMainFrameWidth = rc.width;
    g_Config.m_sizeMainFrameHeight = rc.height;

    event.Skip();
}



////////////////////////////////////////////////////////////////////////////////
// OnTreectrlSelChanged
//

void frmMain::OnTreectrlSelChanged( wxTreeEvent& event )
{
    wxTreeItemId itemId = event.GetItem();
    nodeClientData *item = ( nodeClientData * )m_nodeTree->GetItemData( itemId );

    /* TODO
    if ( item != NULL ) {

        wxString str;
        wxString page;
        
        if ( CLIENT_SERVER == item->m_type ) {
            page = _( "<h1>Hi-end Node Information</h1>" );
        }
        else if ( CLIENT_NODE == item->m_type ) {
            page = _( "<h1>Node information</h1>" );

            if ( VSCP_LEVEL1 == item->m_nodeInformation.m_level ) {
                page += _( "<b>Level: </b> 1<br>" );
            }
            else if ( VSCP_LEVEL2 == item->m_nodeInformation.m_level ) {
                page += _( "<b>Level: </b> 2<br>" );
            }

            page += _( "<b>Last heartbeat: </b>" );
            page += item->m_nodeInformation.m_lastHeartBeat.FormatISODate();
            page += _( " " );
            page += item->m_nodeInformation.m_lastHeartBeat.FormatISOTime();
            page += _( "<br>" );

            page += _( "<b>Node name: </b>" );
            if ( item->m_nodeInformation.m_strNodeName.Length() ) {
                page += item->m_nodeInformation.m_strNodeName;
            }
            else {
                page += _("No name set.");
            }
            page += _( "<br>" );

            page += _( "<b>Device name: </b>" );
            if ( item->m_nodeInformation.m_deviceName.Length() ) {
                page += item->m_nodeInformation.m_deviceName;
            }
            else {
                page += _( "No name set." );
            }
            page += _( "<br>" );

            page += _( "<b>Received from address: </b>" );
            page += item->m_nodeInformation.m_address;
            page += _( "<br>" );

            item->m_nodeInformation.m_realguid.toString( str );
            page += _( "<b>GUID: </b>" );
            page += str;
            page += _( "<br>" );

            item->m_nodeInformation.m_interfaceguid.toString( str );
            page += _( "<b>Interface GUID: </b>" );
            page += str;
            page += _( "<br>" );

            page += _( "<b>Proxy:</b> " );
            if ( item->m_nodeInformation.m_bProxy ) {
                page += _( "Yes" );
            }
            else {
                page += _( "No" );
            }
            page += _( "<br>" );

        }

        m_htmlInfoWnd->SetPage( page ); 
    }
    */
    
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnLeftDown
//

void frmMain::OnLeftDown( wxMouseEvent& event )
{

    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnLeftDClick
//

void frmMain::OnLeftDClick( wxMouseEvent& event )
{

    event.Skip();
}

////////////////////////////////////////////////////////////////////////////////
// addDefaultContent
//

void frmMain::addDefaultContent( void )
{
    m_rootItem = m_nodeTree->AddRoot( _( "VSCP Network neighbourhood" ), MDF_EDITOR_TOP_ITEM );
    m_moduleServerItem = m_nodeTree->AppendItem( m_rootItem, _( "High end nodes" ), MDF_EDITOR_MAIN_ITEM );
    //m_nodeTree->AppendItem( m_moduleServerItem, _( "Node" ), MDF_EDITOR_SUB_ITEM );

    m_moduleNodeItem = m_nodeTree->AppendItem( m_rootItem, _( "Known nodes" ), MDF_EDITOR_MAIN_ITEM );
    //m_nodeTree->AppendItem( m_moduleNodeItem, _( "Node" ), MDF_EDITOR_SUB_ITEM );

    //m_moduleItem = m_mdfTree->AppendItem( m_rootItem, _( "WEB interface" ), MDF_EDITOR_MAIN_ITEM );
    //m_moduleItem = m_mdfTree->AppendItem( m_rootItem, _( "Level II node" ), MDF_EDITOR_MAIN_ITEM );
    //m_moduleItem = m_mdfTree->AppendItem( m_rootItem, _( "Websocket interface" ), MDF_EDITOR_MAIN_ITEM );
    //m_moduleItem = m_mdfTree->AppendItem( m_rootItem, _( "REST interface" ), MDF_EDITOR_MAIN_ITEM );

    /*
    m_mdfTree->AppendItem( m_moduleItem, _( "Model" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Version" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Description" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "URL for full module information" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Last date changed" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Buffersize" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Location of MDF file" ), MDF_EDITOR_SUB_ITEM );

    m_mdfTree->AppendItem( m_moduleItem, _( "Manufacturer" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Driver" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Abstractions" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Registers" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Decision Matrix" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Events" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Alarm" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Boot" ), MDF_EDITOR_MAIN_ITEM );
    */
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenVscpSessionClick
//

void frmMain::OnMenuitemOpenVscpSessionClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );

    if ( wxID_OK == dlg.ShowModal() ) {

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            frmVSCPSession *subframe = new frmVSCPSession( this,
                                                            frmVSCPSession::ID_FRMVSCPSESSION,
                                                            SYMBOL_FRMVSCPSESSION_TITLE,
                                                            VSCPWORKS_SESSION_POSITION,
                                                            wxSize( g_Config.m_sizeConfigurationFrameWidth, 
                                                                        g_Config.m_sizeConfigurationFrameHeight ) );

            if ( NULL != subframe ) {

                if ( 0 != selidx ) {

                    both_interface *pBoth = 
                        (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                    if ( NULL != pBoth ) {

                        // set type so session window is aware of it
                        subframe->m_CtrlObject.m_interfaceType = pBoth->m_type;

                        if ( INTERFACE_CANAL == pBoth->m_type ) {
                            subframe->SetTitle(_("VSCP Client Window Session (CANAL) - ") + 
                                pBoth->m_pcanalif->m_strDescription );
                            subframe->m_CtrlObject.m_ifCANAL.m_strDescription = pBoth->m_pcanalif->m_strDescription;
                            subframe->m_CtrlObject.m_ifCANAL.m_strPath = pBoth->m_pcanalif->m_strPath;
                            subframe->m_CtrlObject.m_ifCANAL.m_strConfig = pBoth->m_pcanalif->m_strConfig;
                            subframe->m_CtrlObject.m_ifCANAL.m_flags= pBoth->m_pcanalif->m_flags;
                        }
                        else if ( INTERFACE_VSCP == pBoth->m_type ) {
                            subframe->SetTitle(_("VSCP Client Window Session (TCP/IP)- ") +  pBoth->m_pvscpif->m_strDescription );
                            subframe->m_CtrlObject.m_ifVSCP.m_strDescription = pBoth->m_pvscpif->m_strDescription;
                            subframe->m_CtrlObject.m_ifVSCP.m_strHost = pBoth->m_pvscpif->m_strHost;
                            subframe->m_CtrlObject.m_ifVSCP.m_strUser = pBoth->m_pvscpif->m_strUser;
                            subframe->m_CtrlObject.m_ifVSCP.m_strPassword = pBoth->m_pvscpif->m_strPassword;
                            memcpy( &subframe->m_CtrlObject.m_ifVSCP.m_vscpfilter, 
                                &pBoth->m_pvscpif->m_vscpfilter, 
                                sizeof( vscpEventFilter ) );
                        }

                        if ( INTERFACE_VSCP == subframe->m_CtrlObject.m_interfaceType ) {

                            // If server username is given and no password is entered we ask for it.
                            if ( subframe->m_CtrlObject.m_ifVSCP.m_strPassword.IsEmpty() && 
                                !subframe->m_CtrlObject.m_ifVSCP.m_strUser.IsEmpty() ) {
                                    subframe->m_CtrlObject.m_ifVSCP.m_strPassword = 
                                        ::wxGetTextFromUser( _("Please enter password"), 
                                        _("Connection Test") );
                            }

                        }

                        // Start the worker threads
                        subframe->startWorkerThreads( subframe );
/*                        
                        subframe->SetSize(  -1, 
                                            -1, 
                                            g_Config.m_sizeSessionFrameWidth, 
                                            g_Config.m_sizeSessionFrameHeight );
*/                        
                       
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

////////////////////////////////////////////////////////////////////////////////
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

            frmDeviceConfig *subframe = new frmDeviceConfig( this,
                                                                frmDeviceConfig::ID_FRMDEVICECONFIG,
                                                                SYMBOL_FRMDEVICECONFIG_TITLE,
                                                                VSCPWORKS_DEVICECONFIG_POSITION,
                                                                wxSize( g_Config.m_sizeConfigurationFrameWidth, 
                                                                            g_Config.m_sizeConfigurationFrameHeight ) );
            if ( NULL != subframe ) {

                both_interface *pBoth = 
                    (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                if ( NULL != pBoth ) {  

                    // set type so session window is aware of it
                    subframe->m_interfaceType = pBoth->m_type;

                    if ( INTERFACE_CANAL == pBoth->m_type ) {

                        // Hide the Level II check box
                        subframe->m_bLevel2->Show( false );

                        // Init. node id combo
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
                        subframe->SetTitle(_("VSCP Configuration Window Session (CANAL) - ") +  
                                    pBoth->m_pcanalif->m_strDescription );

                        subframe->m_csw.setInterface( pBoth->m_pcanalif->m_strDescription,
                                                        pBoth->m_pcanalif->m_strPath,
                                                        pBoth->m_pcanalif->m_strConfig,
                                                        pBoth->m_pcanalif->m_flags, 0, 0 );

                        // DLL timings
                        subframe->m_csw.getDllInterface()->setMaxRetries( g_Config.m_CANALRegMaxRetries );
                        subframe->m_csw.getDllInterface()->setReadResendTimeout( g_Config.m_CANALRegResendTimeout );
                        subframe->m_csw.getDllInterface()->setReadTimeout( g_Config.m_CANALRegErrorTimeout );

                        // Connect to device bus
                        if ( subframe->enableInterface() ) {

                            // Move window on top
                            subframe->Raise();

                            // Show the VSCP configuration windows
                            subframe->Show( true );
                            
                        }
                        else {
                            // Failed to connect - terminate
                            dlg.cleanupListbox();
                            subframe->m_csw.doCmdClose();
                            subframe->Close( true );
                            return;
                        }

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

                        subframe->SetTitle(_("VSCP Configuration Window Session (TCP/IP)- ") +  
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
                        subframe->m_vscpif.m_strInterfaceName = pBoth->m_pvscpif->m_strInterfaceName;
                        memcpy( subframe->m_vscpif.m_GUID, pBoth->m_pvscpif->m_GUID, 16 );
                        memcpy( &subframe->m_vscpif.m_vscpfilter, 
                                    &pBoth->m_pvscpif->m_vscpfilter, 
                                    sizeof( vscpEventFilter ) );

                        // TCP/IP timings
                        subframe->m_csw.getTcpIpInterface()->setRegisterOperationTiming( g_Config.m_TCPIPRegMaxRetries,
                                                                                         g_Config.m_TCPIPRegResendTimeout,
                                                                                         g_Config.m_TCPIPRegErrorTimeout );
                        subframe->m_csw.getTcpIpInterface()->setResponseTimeout( g_Config.m_TCPIP_ResponseTimeout );
                        subframe->m_csw.getTcpIpInterface()->setAfterCommandSleep( g_Config.m_TCPIP_SleepAfterCommand );

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
                        else {
                            // Failed to connect - terminate
                            dlg.cleanupListbox();
                            subframe->m_csw.doCmdClose();
                            subframe->Close( true );
                            return;
                        }

                    }

                } // pBoth valid

            } // valid sub frame pointer

        } // selection made


        // Clean up list box
        dlg.cleanupListbox();

    } // dialog

    event.Skip( false );
}

////////////////////////////////////////////////////////////////////////////////
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

                    if ( INTERFACE_CANAL == pBoth->m_type ) {

                        //subframe->m_comboNodeID->SetValue(_("0x01"));
                        subframe->SetTitle(_("VSCP Registers (CANAL) - ") +  pBoth->m_pcanalif->m_strDescription );

                        // Transfer some parameters so the configuration window can be
                        // opened from the scan window.
                        subframe->m_interfaceType = INTERFACE_CANAL;
                        subframe->m_canalif.m_strDescription = pBoth->m_pcanalif->m_strDescription;
                        subframe->m_canalif.m_strPath =  pBoth->m_pcanalif->m_strPath;
                        subframe->m_canalif.m_strConfig = pBoth->m_pcanalif->m_strConfig;
                        subframe->m_canalif.m_flags = pBoth->m_pcanalif->m_flags;

                        subframe->m_csw.setInterface( pBoth->m_pcanalif->m_strDescription,
                                                        pBoth->m_pcanalif->m_strPath,
                                                        pBoth->m_pcanalif->m_strConfig,
                                                        pBoth->m_pcanalif->m_flags, 0, 0 );

                        // DLL timings
                        subframe->m_csw.getDllInterface()->setMaxRetries( g_Config.m_CANALRegMaxRetries );
                        subframe->m_csw.getDllInterface()->setReadResendTimeout( g_Config.m_CANALRegResendTimeout );
                        subframe->m_csw.getDllInterface()->setReadTimeout( g_Config.m_CANALRegErrorTimeout );

                        // Connect to device bus
                        if ( !subframe->enableInterface() ) {
                            // Failed to connect - terminate
                            dlg.cleanupListbox();
                            subframe->m_csw.doCmdClose();
                            subframe->Close( true );
                            return;
                        }

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
                        subframe->m_interfaceType = INTERFACE_VSCP;
                        subframe->m_vscpif.m_strDescription = pBoth->m_pvscpif->m_strDescription;
                        subframe->m_vscpif.m_strHost = pBoth->m_pvscpif->m_strHost;
                        subframe->m_vscpif.m_strUser = pBoth->m_pvscpif->m_strUser;
                        subframe->m_vscpif.m_strPassword = pBoth->m_pvscpif->m_strPassword;
                        subframe->m_vscpif.m_strInterfaceName = pBoth->m_pvscpif->m_strInterfaceName;
                        memcpy( subframe->m_vscpif.m_GUID, pBoth->m_pvscpif->m_GUID, 16 );
                        memcpy( &subframe->m_vscpif.m_vscpfilter, 
                                    &pBoth->m_pvscpif->m_vscpfilter, 
                                    sizeof( vscpEventFilter ) );

                        // TCP/IP timings
                        subframe->m_csw.getTcpIpInterface()->setRegisterOperationTiming( g_Config.m_TCPIPRegMaxRetries,
                                                                          g_Config.m_TCPIPRegResendTimeout,
                                                                          g_Config.m_TCPIPRegErrorTimeout );
                        subframe->m_csw.getTcpIpInterface()->setResponseTimeout( g_Config.m_TCPIP_ResponseTimeout );
                        subframe->m_csw.getTcpIpInterface()->setAfterCommandSleep( g_Config.m_TCPIP_SleepAfterCommand );
                        
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


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemBootloaderWizardClick
//

void frmMain::OnMenuitemBootloaderWizardClick( wxCommandEvent& event )
{
    DeviceBootloaderwizard *wizard = new DeviceBootloaderwizard( this );

    // DLL timings
    wizard->m_dll.setMaxRetries( g_Config.m_CANALRegMaxRetries );
    wizard->m_dll.setReadResendTimeout( g_Config.m_CANALRegResendTimeout );
    wizard->m_dll.setReadTimeout( g_Config.m_CANALRegErrorTimeout );
    
    // TCP/IP timings
    wizard->m_tcpip.setRegisterOperationTiming( g_Config.m_TCPIPRegMaxRetries,
                                                      g_Config.m_TCPIPRegResendTimeout,
                                                      g_Config.m_TCPIPRegErrorTimeout );
    wizard->m_tcpip.setResponseTimeout( g_Config.m_TCPIP_ResponseTimeout );
    wizard->m_tcpip.setAfterCommandSleep( g_Config.m_TCPIP_SleepAfterCommand );
    
    // Go
    wizard->Run();

    delete wizard;

    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAppExitClick
//


void frmMain::OnMenuitemAppExitClick( wxCommandEvent& event )
{
    Close();
    event.Skip( false );
}

////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAboutClick
//

void frmMain::OnMenuitemAboutClick( wxCommandEvent& event )
{
    dlgAbout dlg(this);
    if ( wxID_OK == dlg.ShowModal() ) {

    }
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpClick
// 

void frmMain::OnMenuitemHelpClick( wxCommandEvent& event )
{
    // vscp works main window / help / help [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemFaqClick
// 

void frmMain::OnMenuitemFaqClick( wxCommandEvent& event )
{
    // vscp works main window / help / faq [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemShortcutsClick
//

void frmMain::OnMenuitemShortcutsClick( wxCommandEvent& event )
{
    // vscp works main window / help / shortcuts [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemThanksClick
//

void frmMain::OnMenuitemThanksClick( wxCommandEvent& event )
{
    // vscp works main window / help / thanks [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW );
    event.Skip( false ); 
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemCrediitsClick
//

void frmMain::OnMenuitemCrediitsClick( wxCommandEvent& event )
{
    // vscp works main window / help / credits [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVSCPSiteClick
//

void frmMain::OnMenuitemVSCPSiteClick( wxCommandEvent& event )
{
    // vscp works main window / help / vscp site [wiki-url]
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org"), wxBROWSER_NEW_WINDOW );
    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemMerlinClick
//

void frmMain::OnMenuitemMerlinClick( wxCommandEvent& event )
{
    Merlin* window = new Merlin(this);
    window->Show(true);

    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenSimpleUIdesignerClick
//

void frmMain::OnMenuitemOpenSimpleUIdesignerClick( wxCommandEvent& event )
{
    SimpleUserInterfaceGenerator* window = new SimpleUserInterfaceGenerator(this);
    window->Show(true);

    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenDaemonDMEditorClick
//

void frmMain::OnMenuitemOpenDaemonDMEditorClick( wxCommandEvent& event )
{
    DaemonDecisionMartrixEditor* window = new DaemonDecisionMartrixEditor(this);
    window->Show( true );

    event.Skip( false );
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemOpenDaemonVaraibleEditorClick
//

void frmMain::OnMenuitemOpenDaemonVariableEditorClick( wxCommandEvent& event )
{
    DaemonVariableEditor* window = new DaemonVariableEditor(this);
    window->Show(true);

    event.Skip( false );
}


