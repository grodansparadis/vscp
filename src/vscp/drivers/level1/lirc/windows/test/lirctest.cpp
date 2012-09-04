///////////////////////////////////////////////////////////////////////////////
// Name:        lirctest.cpp
// Purpose:     Test of CANAL LIRC driver
// Author:      Ake Hedman
// Modified by:
// Created:     2005-02-25
// RCS-ID:      $Id: lirctest.cpp,v 1.5 2005/09/27 12:31:46 akhe Exp $
// Copyright:   (c) 2004-2011 Ake Hedman <akhe@eurosource.se>
// Version 0.0.3
// 
//
 

#define VERSION  "0.0.3"

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include "wx/log.h"
#include "wx/file.h"
#include "wx/sizer.h"
#include "wx/menuitem.h"
#include "wx/checklst.h"
#include "wx/url.h"
#include "wx/html/htmlpars.h"
#include "wx/stream.h"
#include "wx/datetime.h"
#include "wx/msw/ole/automtn.h"
#include "wx/tokenzr.h"

#include "../../common/lircobj.h"
#include "lirctest.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// * * * Event handling * * *

BEGIN_EVENT_TABLE(appFrame, wxFrame)
    
	EVT_MENU( MenuAbout, appFrame::OnAbout)
    EVT_MENU( MenuQuit, appFrame::OnQuit)


    EVT_BUTTON( BtnRefresh, appFrame::OnButtonRefresh )
    EVT_BUTTON( BtnExit, appFrame::OnQuit )

	EVT_TIMER( RefreshTimer, appFrame::OnTimerRefresh )
 

END_EVENT_TABLE()

IMPLEMENT_APP( CheckListBoxApp );



///////////////////////////////////////////////////////////////////////////////
//  OnInit
//
// init our app: create windows
//

bool CheckListBoxApp::OnInit( void )
{
    appFrame *pFrame = new appFrame( NULL,
										wxT("Simple LIRC test program --  eurosource (http://www.eurosource.se)"),
										50, 50, 680, 520 );
    SetTopWindow( pFrame );

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// appFrame
//
// main frame constructor
//

appFrame::appFrame( wxFrame *frame,
					const wxChar *title,
					int x, int y, int w, int h)
                 : wxFrame( frame, -1, title, wxPoint( x, y ), wxSize( w, h ) )
{
		
	m_secondCounter = 0;
	m_pStatusArea = NULL;

	// create the status line
	const int widths[] = { -1, 60, 60 };
	CreateStatusBar( 3 );
	SetStatusWidths( 3, widths );
	wxLogStatus( this, wxT("Simple LIRC CANAL driver test program -  eurosource (http://www.eurosource.se)") );

	///////////////////////////////////////////////////////////////////////////
	//            Persistent storage first time initialization
	///////////////////////////////////////////////////////////////////////////

    // make a panel with some controls
    m_panel = new wxPanel( this, -1, wxPoint( 0, 0 ), wxSize(400, 200), wxTAB_TRAVERSAL);


	m_pStatusArea = new wxStaticText( m_panel, 
										BtnStatusText, 
										wxT("Status Area"),
										wxPoint( 20, 50 ),
										wxSize(800, 30) );

	m_pLogArea = new wxStaticText( m_panel, 
										BtnLogText, 
										wxT("------------------------------------"),
										wxPoint( 20, 50 ),
										wxSize(800, 200) );


    // create buttons for moving the items around
    m_buttonExit = new wxButton( m_panel, BtnExit, wxT("E&xit"), wxPoint( 420, 120 ) );

	// Sizer for the listbox
    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );


	mainsizer->Add( m_pStatusArea, 0, wxGROW | wxALL, 10 );
	//mainsizer->Add( m_pbar, 0, wxGROW | wxALL, 10 );
	mainsizer->Add( m_pLogArea, 0, wxGROW | wxALL, 10 );

	// Sizer for the buttons
    wxBoxSizer *bottomsizer = new wxBoxSizer( wxHORIZONTAL );

    bottomsizer->Add( m_buttonExit, 0, wxALL, 10 );

    mainsizer->Add( bottomsizer, 0, wxCENTER );

    // tell frame to make use of sizer (or constraints, if any)
    m_panel->SetAutoLayout( TRUE );
    m_panel->SetSizer( mainsizer );

    // don't allow frame to get smaller than what the sizers tell ye
    mainsizer->SetSizeHints( this );

	// Initialize timeevents
	m_timer.SetOwner( this, RefreshTimer );
	m_timer.Start( 100 );  

	// Initialize the LIRC system object
	m_lirc.open("c:\\test.cfg");

    Show( TRUE );

}


///////////////////////////////////////////////////////////////////////////////
//  ~appFrame
//


appFrame::~appFrame()
{
  
}



///////////////////////////////////////////////////////////////////////////////
//  OnQuit
//

void appFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	Close( TRUE );
}


///////////////////////////////////////////////////////////////////////////////
//  wxCommandEvent
//

void appFrame::OnAbout( wxCommandEvent& WXUNUSED( event)  )
{
    wxMessageBox(wxT("LircTest\n© eurosource 2005"),
						wxT("About pd"),
						wxICON_INFORMATION, this );
}




 

///////////////////////////////////////////////////////////////////////////////
//  OnTimerRefresh
//

void appFrame::OnTimerRefresh( wxTimerEvent& event ) 
{
	wxString wxstr, msgstr, wxstrdata;

	m_secondCounter++;
	int count;
	count = m_lirc.dataAvailable();

	wxstr.Printf( _T("Number of events = %d"), count );
	m_pStatusArea->SetLabel( wxstr );

	if ( count > 0 ) {	
		
		wxstr = m_pLogArea->GetLabel();
		
		canalMsg msg;
		if ( m_lirc.readMsg( &msg ) ) {
			
			for ( int i=0; i<msg.sizeData; i++ ) {
				wxstrdata += wxstrdata.Format( _T(" %d"), msg.data[ i ] );
			}

			msgstr.Printf( _T("id=%d datacount=%d - %s\r\n"), msg.id, msg.sizeData, wxstrdata );
			wxstr = msgstr + wxstr;
		}

		m_pLogArea->SetLabel( wxstr );
	}
}

 

///////////////////////////////////////////////////////////////////////////////
//  OnButtonRefresh
//

void appFrame::OnButtonRefresh( wxCommandEvent& WXUNUSED( event ) )
{
		
}


 



