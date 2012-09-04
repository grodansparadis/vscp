// cw.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// cw.cpp
//
// This file is part of the CANAL (http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: cw.cpp,v $
// $Date: 2005/09/15 18:49:29 $
// $Author: akhe $
// $Revision: 1.20 $

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif


#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "wx/toolbar.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#include "mondrian.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/paste.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/help.xpm"

#include "cw.h"
#include "msglistwnd.h"
#include "vscpconfigwnd.h"
#include "msglistwnd_level2.h"
#include "selectifdlg.h"

// Configuration stuff
#include <wx/config.h>
#include "wx/wfstream.h"
#include "wx/fileconf.h"
#include <wx/stdpaths.h>

#define INFO_BETA_SOFTWARE  "This software is still beta software.\n"\
			     " please report bugs and suggest features to cw@vscp.org"


IMPLEMENT_APP( CanalWorksApp )

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

MyFrame *gframe = (MyFrame *) NULL;
wxList my_children;


int gs_nFrames = 0;

// ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
EVT_MENU( MDI_ABOUT, MyFrame::OnAbout)
EVT_MENU( MDI_NEW_CANAL_WINDOW, MyFrame::OnNewCANALWindow)
EVT_MENU( MDI_NEW_VSCP_WINDOW, MyFrame::OnNewVSCPWindow)
EVT_MENU( MDI_NEW_VSCP_CONFIG, MyFrame::OnNewVSCPConfigWindow)
EVT_MENU( MDI_QUIT, MyFrame::OnQuit)

EVT_CLOSE(MyFrame::OnClose)

EVT_SIZE(MyFrame::OnSize)
END_EVENT_TABLE()




// ===========================================================================
// implementation
// ===========================================================================

// ---------------------------------------------------------------------------
// MyApp
// ---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// OnInit
//
// Initialise this in OnInit, not statically
//

bool CanalWorksApp::OnInit()
{
	// Flags and switches
  m_bDisplayDeleteWarning = true;
	m_sizeWidth = 640;
	m_sizeHeight = 480;

  m_strUsername = _("admin");
  m_strPassword = _("secret");
  m_strHostname = _("localhost");
  m_port = VSCP_LEVEL2_TCP_PORT;
 
	if ( !loadRegistryData() ) {
		wxMessageBox( _("Unable to open configuration file! Aborting...") );
		return FALSE;
	}

	// Create the main frame window
  gframe = new MyFrame( (wxFrame *)NULL,
                              -1,
                              _T("CanalWorks"),
                              wxPoint( -1, -1 ),
                              wxSize( m_sizeWidth, m_sizeHeight ),
                              wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL );

    // Give it an icon
#ifdef __WXMSW__

    gframe->SetIcon(wxIcon(_T("mdi_icn")));
#else

    gframe->SetIcon(wxIcon( mondrian_xpm ));
#endif

    // Make a menubar
    wxMenu *file_menu = new wxMenu;

    file_menu->Append( MDI_NEW_CANAL_WINDOW,
                           _T("&New CANAL client window\tCtrl-N"),
                           _T("Create a new CANAL client window") );

    file_menu->Append( MDI_NEW_VSCP_WINDOW,
                           _T("&New VSCP client window\tCtrl-N"),
                           _T("Create a new VSCP client window") );

    file_menu->Append( MDI_NEW_VSCP_CONFIG,
                           _T("&VSCP device configuration\tCtrl-V"),
                           _T("Configure a VSCP device.") );

    file_menu->Append( MDI_NEW_VSCP_BOOTLOAD,
                           _T("&VSCP boot loader\tCtrl-V"),
                           _T("Load program data to a VSCP device.") );

    file_menu->Append( MDI_QUIT,
                           _T("&Exit\tAlt-X"),
                           _T("Quit CanalWorks") );

    wxMenu *help_menu = new wxMenu;
    help_menu->Append( MDI_ABOUT, _T("&About\tF1") );

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append( file_menu, _T("&File") );
    //menu_bar->Append( file_menu, _T("&View") );
    //menu_bar->Append( file_menu, _T("&Tools") );
    //menu_bar->Append( file_menu, _T("&Options") );
     menu_bar->Append( help_menu, _T("&Help") );

    // Associate the menu bar with the frame
    gframe->SetMenuBar(menu_bar);
    gframe->CreateStatusBar();
    gframe->Show(TRUE);
    SetTopWindow( gframe );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OnExit
//

int CanalWorksApp::OnExit()
{
	saveRegistryData();
	return 0;
}

#define WIN32_USE_FILE

/////////////////////////////////////////////////////////////////////////////
// loadRegistryData
//
// Read persistant data from the registry
//
//

bool CanalWorksApp::loadRegistryData( void )
{
	wxString str;
	long val;
  wxString strcfgfile;
  wxStandardPaths strpath;
  
  strcfgfile = strpath.GetUserDataDir();
  strcfgfile += _("/canalworks.conf");

  	// Check if file exists
	if ( !::wxFileExists( strcfgfile ) ) {
		return false;
	}

	wxFFileInputStream cfgstream( strcfgfile );

	if ( !cfgstream.Ok() ) {
		return false;
	}

	wxFileConfig *pconfig = new wxFileConfig( cfgstream );

/*
#ifdef WIN32
#ifdef WIN32_USE_FILE
	char szPathSystemDir[ MAX_PATH ];
	GetSystemDirectory( szPathSystemDir, sizeof( szPathSystemDir ) );
	strcat( szPathSystemDir, "\\canalworks.conf" );

	// Check if file exists
	if ( !::wxFileExists( szPathSystemDir ) ) {
		return FALSE;
	}

	wxFFileInputStream cfgstream( szPathSystemDir );

	if ( !cfgstream.Ok() ) {
		return false;
	}

	wxFileConfig *pconfig = new wxFileConfig( cfgstream );

#else
	wxConfig *pconfig = new wxConfig( _("canald"), _("VSCP") );
#endif
#else
        wxConfig *pconfig = new wxConfig( _("canald"), 
					                                _("VSCP"), 
					                                _("localcanalworks.conf"), 
					                                _("canalworks.conf"), 
					                                wxCONFIG_USE_GLOBAL_FILE );
#endif
*/                                          

	pconfig->SetPath( _("/OPTIONS") );

	// bDisplayDeleteWarning
	m_bDisplayDeleteWarning = true;
	if ( pconfig->Read( _("bDisplayDeleteWarning"), &val, 0 ) ) {
		m_bDisplayDeleteWarning = ( val ? true : false);
	}
	wxLogDebug( _("Read Configuration: bDisplayDeleteWarning=%d"), m_bDisplayDeleteWarning );

	// sizeWidth
	if ( pconfig->Read( _("sizeWidth"), &val, 640 ) ) {
		m_sizeWidth = val;
	}
	wxLogDebug(_("Read Configuration: sizeWidth=%d"), m_sizeWidth );
	
	// sizeHeight
	if ( pconfig->Read( _("sizeHeight"), &val, 480 ) ) {
		m_sizeHeight = val;
	}
	wxLogDebug(_("Read Configuration: sizeHeight=%d"), m_sizeHeight );

  m_strUsername = pconfig->Read( _("username"), _("admin" ) );
  m_strPassword = pconfig->Read( _("password"), _("secret" ) );
  m_strHostname = pconfig->Read( _("hostname"), _("localhost") );

  // port
	if ( pconfig->Read( _("port"), &val, VSCP_LEVEL2_TCP_PORT ) ) {
		m_port = val;
	}  

	if ( NULL!= pconfig ) delete pconfig;

	return true;
}



/////////////////////////////////////////////////////////////////////////////
// saveRegistryData
//
// Write persistant data to the registry
//

void CanalWorksApp::saveRegistryData( void )
{
	wxString str;
  wxString strcfgfile;
  wxStandardPaths strpath;
  
  strcfgfile = strpath.GetUserDataDir();
  strcfgfile += _("/canalworks.conf");
	
/*
#ifdef WIN32
  wxChar path[MAX_PATH];
	GetSystemDirectory( path, sizeof( path ) );
  wxString strPathSystemDir( path );
  strPathSystemDir += _("\\canalworks.conf");
#else
	wxString strPathSystemDir( _("/etc/canalworks.conf") );
#endif
*/

	wxFFileInputStream cfgstream( strcfgfile );

	if ( !cfgstream.Ok() ) {
		return;
	}

	wxFileConfig *pconfig = new wxFileConfig( cfgstream );

	pconfig->SetPath(_("/OPTIONS"));

	// bDisplayDeleteWarning
	pconfig->Write( _("bDisplayDeleteWarning"), ( m_bDisplayDeleteWarning ? 1 : 0 ) );
 
	// sizeWidth
	pconfig->Write( _("sizeWidth"), m_sizeWidth );
	
	// sizeHeight
	pconfig->Write( _("sizeHeight"), m_sizeHeight );

  pconfig->Write( _("username"), m_strUsername );
  pconfig->Write( _("password"), m_strPassword );
  pconfig->Write( _("hostname"), m_strHostname );
  pconfig->Write( _("port"), m_port );

	wxFFileOutputStream cfgOutstream( strcfgfile );

	if ( !cfgOutstream.Ok() ) {
		return;
	}

	// Save changes
	pconfig->Save( cfgOutstream );


	if ( NULL!= pconfig ) delete pconfig;
	
}








// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

MyFrame::MyFrame( wxWindow *parent,
                  const wxWindowID id,
                  const wxString& title,
                  const wxPoint& pos,
                  const wxSize& size,
                  const long style)
                : wxMDIParentFrame( parent, id,
                                    title,
                                    pos,
                                    size,
                                    style )
{
	
	m_pdebugWindow = new wxTextCtrl( this,
                                         -1,
                                         _T(INFO_BETA_SOFTWARE),
                                         wxDefaultPosition,
                                         wxDefaultSize,
                                         wxTE_MULTILINE | wxSUNKEN_BORDER );

    CreateToolBar( wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL );
    InitToolBar( GetToolBar() );

    // Accelerators
    wxAcceleratorEntry entries[ 5 ];
    entries[0].Set( wxACCEL_CTRL, (int) 'N', MDI_NEW_CANAL_WINDOW );
    entries[1].Set( wxACCEL_CTRL, (int) 'V', MDI_NEW_VSCP_WINDOW );
    entries[2].Set( wxACCEL_CTRL, (int) 'C', MDI_NEW_VSCP_CONFIG );
    entries[3].Set( wxACCEL_CTRL, (int) 'X', MDI_QUIT );
    entries[4].Set( wxACCEL_CTRL, (int) 'A', MDI_ABOUT );
    wxAcceleratorTable accel( 4, entries );
    SetAcceleratorTable( accel );

}


///////////////////////////////////////////////////////////////////////////////
// OnClose
//

void MyFrame::OnClose( wxCloseEvent& event )
{       
	
	/*
          if ( event.CanVeto() && (gs_nFrames > 0) ) {
          wxString msg;
          msg.Printf(_T("%d windows still open, close anyhow?"), gs_nFrames);
          if ( wxMessageBox(msg, _T("Please confirm"),
          wxICON_QUESTION | wxYES_NO) != wxYES ) {
          event.Veto();
          return;
          }
          }
	*/
    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnQuit
//

void MyFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
        Close();
}


///////////////////////////////////////////////////////////////////////////////
// OnAbout
//

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
  (void)wxMessageBox( _("CanalWorks\n Copyright (c) Ake Hedman, eurosource (c) 2004-2010\n"),
                        _("About CanalWorks"));
}


///////////////////////////////////////////////////////////////////////////////
// OnNewCANALWindow
//
// Create a new Canal List Window
//

void MyFrame::OnNewCANALWindow( wxCommandEvent& WXUNUSED( event ) )
{
        // Select Interface to connect to
        CSelectIfDlg dlg( this );

        if ( wxID_OK == dlg.ShowModal() ) {

                // Make another frame, containing a canvas
                CanalChild *subframe = new CanalChild( gframe,
                                                       _("CANAL Session"),
                                                       wxPoint( -1, -1 ),
                                                       wxSize(-1, -1),
                                                       wxDEFAULT_FRAME_STYLE,
                                                       dlg.getSelectedDevice() );

                gs_nFrames++;	// Another frame

                // Give it an icon
#ifdef __WXMSW__

                subframe->SetIcon(wxIcon(_("chrt_icn")));
#else

                subframe->SetIcon(wxIcon( mondrian_xpm ));
#endif

                // Make a menubar
                wxMenu *file_menu = new wxMenu;

                file_menu->Append( MDI_NEW_CANAL_WINDOW,
                                   _("&New CANAL client window\tCtrl-N"),
                                   _("Create a new CANAL client window") );

                file_menu->Append( MDI_NEW_VSCP_WINDOW,
                                   _("&New VSCP client window\tCtrl-V"),
                                   _("Create a new VSCP client window") );

                file_menu->Append( MDI_NEW_VSCP_CONFIG,
                                   _("&VSCP device configuration\tCtrl-C"),
                                   _("Configure a VSCP device.") );

                file_menu->Append( MDI_NEW_VSCP_BOOTLOAD,
                                   _("&VSCP boot loader\tCtrl-V"),
                                   _("Load program data to a VSCP device.") );

                file_menu->Append( MDI_CHILD_QUIT,
                                   _("&Close List Window"),
                                   _("Close this window") );

                file_menu->Append( MDI_QUIT, _("&Exit") );

                m_poption_menu = new wxMenu;

                m_poption_menu->AppendCheckItem( MDI_VIEW_SEND_TOOLS, _("&View Send tools\tCtrl-T"));
                m_poption_menu->Check( MDI_VIEW_SEND_TOOLS, TRUE );

                m_poption_menu->AppendCheckItem( MDI_ACTIVATE_INTERFACE,
                                                 _("&Activate Interface\tCtrl-A"));
                m_poption_menu->AppendCheckItem( MDI_ACTIVATE_FILTER, _("&Activate Filter\tCtrl-F"));

                m_poption_menu->AppendCheckItem( MDI_CHANGE_TITLE, _("Change &title\tCtrl-T"));
                m_poption_menu->AppendSeparator();
                m_poption_menu->Append( MDI_CHANGE_POSITION, _("Move frame\tCtrl-M"));
                m_poption_menu->Append( MDI_CHANGE_SIZE, _("Resize frame\tCtrl-S"));

                wxMenu *help_menu = new wxMenu;
                help_menu->Append(MDI_ABOUT, _("&About"));

                wxMenuBar *menu_bar = new wxMenuBar;

                menu_bar->Append( file_menu, _("&File") );
                menu_bar->Append( m_poption_menu, _("Canal &Interface") );
                menu_bar->Append( help_menu, _("&Help") );

                // Associate the menu bar with the frame
                subframe->SetMenuBar(menu_bar);

                subframe->CreateStatusBar();
                subframe->SetStatusText( _("Canal Works") );

                int width, height;
                subframe->GetClientSize( &width, &height );

                subframe->Show( TRUE );

        }
}


///////////////////////////////////////////////////////////////////////////////
// OnNewVSCPWindow
//
// Create a new VSCP Window
//

void MyFrame::OnNewVSCPWindow( wxCommandEvent& WXUNUSED( event ) )
{
        // Make another frame, containing a canvas
        Level2Child *subframe = new Level2Child( gframe,
                                _("VSCP Session"),
                                wxPoint( -1, -1 ),
                                wxSize(-1, -1),
                                wxDEFAULT_FRAME_STYLE,
                                NULL );

        gs_nFrames++;	// Another frame

        // Give it an icon
#ifdef __WXMSW__

        subframe->SetIcon(wxIcon(_("chrt_icn")));
#else

        subframe->SetIcon(wxIcon( mondrian_xpm ));
#endif

        // Make a menubar
        wxMenu *file_menu = new wxMenu;

        file_menu->Append( MDI_NEW_CANAL_WINDOW,
                           _("&New CANAL client window\tCtrl-N"),
                           _("Create a new CANAL client window") );

        file_menu->Append( MDI_NEW_VSCP_WINDOW,
                           _("&New VSCP client window\tCtrl-V"),
                           _("Create a new VSCP client window") );

        file_menu->Append( MDI_NEW_VSCP_CONFIG,
                           _("&VSCP device configuration\tCtrl-C"),
                           _("Configure a VSCP device.") );

        file_menu->Append( MDI_NEW_VSCP_BOOTLOAD,
                           _("&VSCP boot loader\tCtrl-V"),
                           _("Load program data to a VSCP device.") );

        file_menu->Append( MDI_CHILD_QUIT,
                           _("&Close List Window"),
                           _("Close this window") );

        file_menu->Append( MDI_QUIT, _("&Exit") );

        m_poption_menu = new wxMenu;

        m_poption_menu->AppendCheckItem( MDI_VIEW_SEND_TOOLS, _("&View Send tools\tCtrl-T"));
        m_poption_menu->Check( MDI_VIEW_SEND_TOOLS, TRUE );

        m_poption_menu->AppendCheckItem( MDI_ACTIVATE_INTERFACE,
                                         _("&Activate Interface\tCtrl-A"));
        m_poption_menu->AppendCheckItem( MDI_ACTIVATE_FILTER, _("&Activate Filter\tCtrl-F"));

        m_poption_menu->AppendCheckItem( MDI_CHANGE_TITLE, _("Change &title\tCtrl-T"));
        m_poption_menu->AppendSeparator();
        m_poption_menu->Append( MDI_CHANGE_POSITION, _("Move frame\tCtrl-M"));
        m_poption_menu->Append( MDI_CHANGE_SIZE, _("Resize frame\tCtrl-S"));

        wxMenu *help_menu = new wxMenu;
        help_menu->Append(MDI_ABOUT, _("&About"));

        wxMenuBar *menu_bar = new wxMenuBar;

        menu_bar->Append( file_menu, _("&File") );
        menu_bar->Append( m_poption_menu, _("Canal &Interface") );
        menu_bar->Append( help_menu, _("&Help") );

        // Associate the menu bar with the frame
        subframe->SetMenuBar(menu_bar);

        subframe->CreateStatusBar();
        subframe->SetStatusText( _("Canal Works") );

        int width, height;
        subframe->GetClientSize( &width, &height );

        subframe->Show( TRUE );

}



///////////////////////////////////////////////////////////////////////////////
// OnNewVSCPConfig
//
// Open up a new VSCP Configuration Window
//

void MyFrame::OnNewVSCPConfigWindow( wxCommandEvent& WXUNUSED( event ) )
{
        // Select Interface to connect to
        CSelectIfDlg dlg( this );

        if ( wxID_OK == dlg.ShowModal() ) {

                // Make another frame, containing a canvas
                VSCPConfigChild *subframe =
                        new VSCPConfigChild( gframe,
                                             _("VSCP Configuration"),
                                             wxPoint( -1, -1 ),
                                             wxSize(-1, -1),
                                             wxDEFAULT_FRAME_STYLE,
                                             dlg.getSelectedDevice() );

                gs_nFrames++;	// Another frame

                // Give it an icon
#ifdef __WXMSW__

                subframe->SetIcon(wxIcon(_("chrt_icn")));
#else

                subframe->SetIcon(wxIcon( mondrian_xpm ));
#endif

                // Make a menubar
                wxMenu *file_menu = new wxMenu;

                file_menu->Append( MDI_NEW_CANAL_WINDOW,
                                   _("&New CANAL client window\tCtrl-N"),
                                   _("Create a new client window") );

                file_menu->Append( MDI_NEW_VSCP_CONFIG,
                                   _("&VSCP device configuration\tCtrl-V"),
                                   _("Configure a VSCP device.") );

                file_menu->Append( MDI_NEW_VSCP_BOOTLOAD,
                                   _("&VSCP boot loader\tCtrl-V"),
                                   _("Load program data to a VSCP device.") );

                file_menu->Append( MDI_CHILD_QUIT,
                                   _("&Close List Window"),
                                   _("Close this window") );

                file_menu->Append( MDI_QUIT, _("&Exit") );

                m_poption_menu = new wxMenu;

                m_poption_menu->AppendCheckItem( MDI_VIEW_SEND_TOOLS, _("&View Commands\tCtrl-T") );
                m_poption_menu->Check( MDI_VIEW_SEND_TOOLS, TRUE );


                wxMenu *help_menu = new wxMenu;
                help_menu->Append(MDI_ABOUT, _("&About"));

                wxMenuBar *menu_bar = new wxMenuBar;

                menu_bar->Append( file_menu, _("&File") );
                menu_bar->Append( m_poption_menu, _("Canal &Interface") );
                menu_bar->Append( help_menu, _("&Help") );

                // Associate the menu bar with the frame
                subframe->SetMenuBar(menu_bar);

                subframe->CreateStatusBar();
                subframe->SetStatusText( _("Canal Works") );

                int width, height;
                subframe->GetClientSize( &width, &height );

                subframe->Show( TRUE );
        }
}


///////////////////////////////////////////////////////////////////////////////
// OnSize
//

void MyFrame::OnSize(wxSizeEvent& event)
{
	int w, h;
    GetClientSize( &w, &h );

    m_pdebugWindow->SetSize(0, h - 60, w, h - 60 );
    GetClientWindow()->SetSize(0, 0, w, h - 60);

	GetSize( &w, &h );
	wxGetApp().m_sizeWidth = w;
	wxGetApp().m_sizeHeight = h;

    // FIXME: On wxX11, we need the MDI frame to process this
    // event, but on other platforms this should not
    // be done.
#ifdef __WXUNIVERSAL__
    event.Skip();
#endif
}


///////////////////////////////////////////////////////////////////////////////
// InitToolBar
//

void MyFrame::InitToolBar( wxToolBar* toolBar )
{
	wxBitmap* bitmaps[8];

    bitmaps[0] = new wxBitmap( new_xpm );
    bitmaps[1] = new wxBitmap( open_xpm );
    bitmaps[2] = new wxBitmap( save_xpm );
    bitmaps[3] = new wxBitmap( copy_xpm );
    bitmaps[4] = new wxBitmap( cut_xpm );
    bitmaps[5] = new wxBitmap( paste_xpm );
    bitmaps[6] = new wxBitmap( print_xpm );
    bitmaps[7] = new wxBitmap( help_xpm );

    int width = 24;
    int currentX = 5;

    toolBar->AddTool( MDI_NEW_CANAL_WINDOW, 
							*(bitmaps[0]), 
							wxNullBitmap, 
							FALSE, 
							currentX, 
							-1, 
							(wxObject *) NULL,
							_("CAN Message Window") );
    currentX += width + 5;

	toolBar->AddTool( MDI_NEW_VSCP_WINDOW, 
							*(bitmaps[0]), 
							wxNullBitmap, 
							FALSE, 
							currentX, 
							-1, 
							(wxObject *) NULL,
							_("VSCP Message Window") );
    currentX += width + 5;

	toolBar->AddTool( MDI_NEW_VSCP_CONFIG, 
							*(bitmaps[0]), 
							wxNullBitmap, 
							FALSE, 
							currentX, 
							-1, 
							(wxObject *) NULL,
							_("VSCP Message Window") );
    currentX += width + 5;
    
	toolBar->AddTool(1, *bitmaps[1], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("CAN Message Window"));
	currentX += width + 5;
    
	toolBar->AddTool(3, *bitmaps[2], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("Save file"));
    currentX += width + 5;
    
	toolBar->AddSeparator();
    
	toolBar->AddTool(4, *bitmaps[3], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("Copy"));
    currentX += width + 5;
    
	toolBar->AddTool(5, *bitmaps[4], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("Cut"));
    currentX += width + 5;
    
	toolBar->AddTool(6, *bitmaps[5], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("Paste"));
    currentX += width + 5;
    
	toolBar->AddSeparator();
    
	toolBar->AddTool(7, *bitmaps[6], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, _("Print"));
    currentX += width + 5;
    
	toolBar->AddSeparator();
    
	toolBar->AddTool(8, *bitmaps[7], wxNullBitmap, TRUE, currentX, -1, (wxObject *) NULL, _("Help"));

    toolBar->Realize();

    for ( int i = 0; i < 8; i++ ) {
		delete bitmaps[ i ];
    }
}


