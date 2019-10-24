/////////////////////////////////////////////////////////////////////////////
// Name:        frmmain.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 22:03:27 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmain.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "dlgselectunit.h"
#include "frmNode.h"
#include "threadvscpwork.h"


////@begin includes
////@end includes


#include "autoself.h"
#include "frmmain.h"
#include "dlgabout.h"
#include "dlgsettings.h"


extern _nodeinfo g_nodes[];
extern int g_cntNodes;
extern _appConfiguration g_Config;

DEFINE_EVENT_TYPE(wxMAINLOG_EVENT)


////@begin XPM images
/* XPM */
static char *wxwin32x32_xpm[] = {
"32 32 6 1",
"   c None",
".  c #000000",
"X  c #000084",
"o  c #FFFFFF",
"O  c #FFFF00",
"+  c #FF0000",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"        ..............          ",
"        .XXXXXXXXXXXX.          ",
"        .XXXXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXX..............",
"        .XooXXXXXX.OOOOOOOOOOOO.",
".........XooXXXXXX.OOOOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++...........OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".++++++++++++.    ..............",
".++++++++++++.                  ",
"..............                  ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "};

////@end XPM images

/*!
 * frmMain type definition
 */

IMPLEMENT_CLASS( frmMain, wxMDIParentFrame )

/*!
 * frmMain event table definition
 */

BEGIN_EVENT_TABLE( frmMain, wxMDIParentFrame )

////@begin frmMain event table entries
  EVT_MENU( ID_MENUITEM_OPEN, frmMain::OnMenuitemOpenClick )

  EVT_MENU( ID_MENUITEM_EXIT, frmMain::OnMenuitemExitClick )

  EVT_MENU( ID_MENUITEM_SETTINGS, frmMain::OnMenuitemSettingsClick )

  EVT_MENU( ID_MENUITEM_ABOUT, frmMain::OnMenuitemAboutClick )

////@end frmMain event table entries

  EVT_COMMAND( ID_FRMMAIN, wxMAINLOG_EVENT, frmMain::OnMainLogMsg )

END_EVENT_TABLE()

/*!
 * frmMain constructors
 */

frmMain::frmMain()
{
    Init();
}

frmMain::frmMain( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

/*!
 * frmMain creator
 */

bool frmMain::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin frmMain creation
  wxMDIParentFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("wxwin32x321.xpm")));
  Centre();
////@end frmMain creation

  return true;
}

/*!
 * frmMain destructor
 */

frmMain::~frmMain()
{
////@begin frmMain destruction
////@end frmMain destruction
}

/*!
 * Member initialisation 
 */

void frmMain::Init()
{
////@begin frmMain member initialisation
////@end frmMain member initialisation

  m_plogfile = NULL;  // No logfile yet
  
  // Open logfile (if it should be opend )
  if ( g_Config.m_bLogFile ) {

    m_plogfile = new wxFile( g_Config.m_strPathLogFile.fn_str(), wxFile::write_append );
        
    wxCommandEvent eventLog( wxMAINLOG_EVENT, frmMain::ID_FRMMAIN );
    eventLog.SetInt( 0 );
    wxString wxstr = _("Program started.");
    eventLog.SetString( wxstr );
    wxPostEvent( this, eventLog );
    
  }
  
}

/*!
 * Control creation for frmMain
 */

void frmMain::CreateControls()
{    
////@begin frmMain content constructionhttp://www.aftonbladet.se/
  frmMain* itemMDIParentFrame1 = this;

  wxMenuBar* menuBar = new wxMenuBar;
  wxMenu* itemMenu3 = new wxMenu;
  itemMenu3->Append(ID_MENUITEM_OPEN, _("&Open"), _T(""), wxITEM_NORMAL);
  itemMenu3->AppendSeparator();
  itemMenu3->Append(ID_MENUITEM_EXIT, _("E&xit"), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu3, _("&File"));
  wxMenu* itemMenu7 = new wxMenu;
  menuBar->Append(itemMenu7, _("Edit"));
  wxMenu* itemMenu8 = new wxMenu;
  menuBar->Append(itemMenu8, _("View"));
  wxMenu* itemMenu9 = new wxMenu;
  itemMenu9->Append(ID_MENUITEM_SETTINGS, _("General Settings..."), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu9, _("Tools"));
  wxMenu* itemMenu11 = new wxMenu;
  itemMenu11->Append(ID_MENUITEM_VSCP_SITE, _("VSCP Site"), _T(""), wxITEM_NORMAL);
  itemMenu11->AppendSeparator();
  itemMenu11->Append(ID_MENUITEM_ABOUT, _("About..."), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu11, _("Help"));
  itemMDIParentFrame1->SetMenuBar(menuBar);

  wxStatusBar* itemStatusBar15 = new wxStatusBar;
  itemStatusBar15->Create( itemMDIParentFrame1, ID_STATUSBAR_MAIN, wxST_SIZEGRIP|wxNO_BORDER );
  itemStatusBar15->SetFieldsCount(2);
  itemMDIParentFrame1->SetStatusBar(itemStatusBar15);

////@end frmMain content construction
}

/*!
 * Should we show tooltips?
 */

bool frmMain::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap frmMain::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin frmMain bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end frmMain bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon frmMain::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin frmMain icon retrieval
  wxUnusedVar(name);
  if (name == _T("wxwin32x321.xpm"))
  {
    wxIcon icon(wxwin32x32_xpm);
    return icon;
  }
  return wxNullIcon;
////@end frmMain icon retrieval
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_EXIT
 */

void frmMain::OnMenuitemExitClick( wxCommandEvent& event )
{
    Close();
    event.Skip(); 
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_OPEN
 */

void frmMain::OnMenuitemOpenClick( wxCommandEvent& event )
{
	dlgSelectUnit dlg( this );
    
  if ( wxID_OK == dlg.ShowModal() ) {
		
		frmNode *subframe = new frmNode( this, frmNode::ID_FRMNODE );
		if ( NULL != subframe ) { 	
		
			// Set table index for node info
			subframe->m_nodeIndex = dlg.m_selectedIndex;
	
			// Set the title
			subframe->SetTitle( g_nodes[ dlg.m_selectedIndex ].m_strRealname );
			
			subframe->Show( TRUE );
			
			// Start workerthread
			VscpWorkThread *pwrkThread = new VscpWorkThread();
			if ( NULL != pwrkThread ) {
					pwrkThread->m_pfrmNode = subframe;	
          pwrkThread->m_pfrmMain = this;
          pwrkThread->m_FrameId = dlg.m_selectedIndex;
          pwrkThread->m_strFrameName = g_nodes[ dlg.m_selectedIndex ].m_strRealname;
					pwrkThread->Create();
          pwrkThread->Run();
			}	

		}		
  }
	
	event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAboutClick
//

void frmMain::OnMenuitemAboutClick( wxCommandEvent& event )
{
	dlgAbout dlg( this );
  dlg.ShowModal();
	event.Skip(); 
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SETTINGS
 */

void frmMain::OnMenuitemSettingsClick( wxCommandEvent& event )
{	
	dlgSettings dlg( this );
	
	dlg.m_editVSCPServer->SetValue( g_Config.m_strServer );
  dlg.m_editVSCPPort->SetValue( g_Config.m_strPort );
  dlg.m_editVSCPUsername->SetValue( g_Config.m_strUsername );
  dlg.m_editVSCPPassword->SetValue( g_Config.m_strPassword );
  dlg.m_editPathLogFile->SetValue( g_Config.m_strPathLogFile );
  dlg.m_bCheckLogFile->SetValue( g_Config.m_bLogFile );
	
	if ( wxID_OK == dlg.ShowModal() ) {
		g_Config.m_strServer = dlg.m_editVSCPServer->GetValue();
		g_Config.m_strPort = dlg.m_editVSCPPort->GetValue();
		g_Config.m_strUsername = dlg.m_editVSCPUsername->GetValue();
		g_Config.m_strPassword = dlg.m_editVSCPPassword->GetValue();
    g_Config.m_strPathLogFile = dlg.m_editPathLogFile->GetValue();
    g_Config.m_bLogFile = dlg.m_bCheckLogFile->GetValue();
	}
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnMainLogMsg
//

void frmMain::OnMainLogMsg( wxCommandEvent &event )
{
  //::wxLogStatus( event.GetString() );
  //::wxLogMessage( event.GetString() );
  //m_labelLogMsg->SetLabel( event.GetString() );

  wxDateTime now = wxDateTime::Now();
  wxString wxstr;
  wxstr.Printf(_("[%d] %s\t"), event.GetInt(), now.Format("%c", wxDateTime::CET).c_str() );
  wxstr += event.GetString() ;
  wxstr += _("\n");
  if ( NULL != m_plogfile ) {
    m_plogfile->Write( wxstr );
  }

	event.Skip();
}
