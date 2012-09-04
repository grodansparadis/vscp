/////////////////////////////////////////////////////////////////////////////
// Name:        vscpboot.cpp
// Purpose:     VSCP Bootloader wizard
// Author:      Ake Hedman, akhe@eurosource.se
// Modified by:
// Created:     2004-09-02
// RCS-ID:      $Id: vscpboot.cpp,v 1.22 2005/01/05 12:50:58 akhe Exp $
// Copyright:   Copyright (c)  2004-2010 Ake Hedman, akhe@eurosource.se
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
// $RCSfile: vscpboot.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.22 $ 

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "vscpboot.cpp"
    #pragma interface "vscpboot.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/wizard.h"

#ifndef __WXMSW__
    #include "wiztest.xpm"
    #include "wiztest2.xpm"
#endif

#include "bootcontrol.h"
#include "SelectInterfacePage.h"
#include "welcome.h"
#include "SelectInputFile.h"
#include "BootHexFileInfo.h"
#include "NickNameSelectionPage.h"
#include "BootLoaderPage.h"
#include "ByeByePage.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// ids for menu items
enum
{
    vscpboot_Quit = 100,
    vscpboot_Run,
    vscpboot_About = 1000
};

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

CBootControl gBootCtrl;



// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit( wxCommandEvent& event);
    void OnAbout( wxCommandEvent& event);
    void OnRunWizard( wxCommandEvent& event );
    void OnWizardCancel( wxWizardEvent& event );

private:
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// some pages for our wizard
// ----------------------------------------------------------------------------

// this shows how to simply control the validity of the user input by just
// overriding TransferDataFromWindow() - of course, in a real program, the
// check wouldn't be so trivial and the data will be probably saved somewhere
// too
//
// it also shows how to use a different bitmap for one of the pages


// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// event tables and such
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(vscpboot_Quit,  MyFrame::OnQuit)
    EVT_MENU(vscpboot_About, MyFrame::OnAbout)
    EVT_MENU(vscpboot_Run,   MyFrame::OnRunWizard)

    EVT_WIZARD_CANCEL(-1, MyFrame::OnWizardCancel)
END_EVENT_TABLE()


IMPLEMENT_APP(MyApp)

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// MyApp::OnInit
//

// `Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame(_T("VSCP - Bootloader Wizard"));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show( TRUE );

    // we're done
    return TRUE;
}

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// MyFrame::MyFrame
//

MyFrame::MyFrame(const wxString& title)
       : wxFrame((wxFrame *)NULL, -1, title,
                  wxDefaultPosition, wxSize(550, 600) )  
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append( vscpboot_Run, _T("&Run bootloader wizard...\tCtrl-R") );
    menuFile->AppendSeparator();
    menuFile->Append( vscpboot_Quit, _T("E&xit\tAlt-X"), _T("Quit this program") );
 
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append( vscpboot_About, _T("&About...\tF1"), _T("Show about dialog") );

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append( menuFile, _T("&File") );
    menuBar->Append( helpMenu, _T("&Help") );

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

    // also create status bar which we use in OnWizardCancel
    CreateStatusBar();

	// Launch the wizzard
	wxCommandEvent wxwiz;
	OnRunWizard( wxwiz );

	Close( TRUE );
}



///////////////////////////////////////////////////////////////////////////////
// MyFrame::OnQuit
//

void MyFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
    // TRUE is to force the frame to close
    Close(TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// MyFrame::OnAbout
//

void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    wxMessageBox(_T("VSCP - Bootloader Wizard\n")
                 _T("GPL License © 2004-2010 VSCP Team, http://www.vscp.org"),
                 _T("About vscpboot"), wxOK | wxICON_INFORMATION, this);
}

///////////////////////////////////////////////////////////////////////////////
// MyFrame::OnRunWizard
//

void MyFrame::OnRunWizard( wxCommandEvent& WXUNUSED( event ) )
{	
	// Create the wizard
	wxWizard *wizard = new wxWizard( this,
										-1,
										_T("VSCP Bootloader Wizard"),
										wxBITMAP( wiztest ) );

	// Set the wizard window size
	wxSize size( 350, 200 );

    // Create the wizard pages
	wxWelcomePage *page1 = new wxWelcomePage( wizard );

	wxInterfaceSelectPage *page2 = new wxInterfaceSelectPage( wizard );

	wxFileSelectPage *page3 = new wxFileSelectPage( wizard );

    //wxValidationPage *page4 = new wxValidationPage( wizard );
	wxBootHexFileInfoPage *page4 = new wxBootHexFileInfoPage( wizard );
	
	wxNickNameSelectionPage *page5 = new wxNickNameSelectionPage( wizard );

	wxBootLoadPage *page6 = new wxBootLoadPage( wizard );

	wxGoodbyPage *page7 = new wxGoodbyPage( wizard );

    // set the page order using a convenience function - could also use
    // SetNext/Prev directly as below
	wxWizardPageSimple::Chain(page1, page2); // i/f select
	wxWizardPageSimple::Chain(page2, page3); // Select boot file
    wxWizardPageSimple::Chain(page3, page4); // HEX file info
	wxWizardPageSimple::Chain(page4, page5); // Nickname selection
	wxWizardPageSimple::Chain(page5, page6); // Do Bootload
	wxWizardPageSimple::Chain(page6, page7); // Goodby
    
	//page1->SetNext( page2 );
    //page3->SetPrev( page2 );

	// Set the pagesize
    wizard->SetPageSize( size );

    if ( wizard->RunWizard( page1 ) ) {

        //wxMessageBox( _T("vscpboot successfully completed"), _T("VSCP Bootloader wizard"),
        //            wxICON_INFORMATION | wxOK);

    }



	delete page1;
	delete page2;
	delete page3;
	delete page4;
	delete page5;
	delete page6;
	delete page7;
    wizard->Destroy();
}

///////////////////////////////////////////////////////////////////////////////
// MyFrame::OnWizardCancel
//

void MyFrame::OnWizardCancel( wxWizardEvent& WXUNUSED( even t) )
{
    wxLogStatus(this, wxT("vscpboot was cancelled."));
}
