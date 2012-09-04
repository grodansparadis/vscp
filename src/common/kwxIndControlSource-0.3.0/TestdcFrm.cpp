/////////////////////////////////////////////////////////////////////////////
// Name:        TestdcFrm.cpp
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#include "kprec.h"		//#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/image.h>

#include <wx/statbox.h>

/*
#include "res/lampon.xpm"
#include "res/lampoff.xpm"
#include "res/lamponsel.xpm"
#include "res/lampoffsel.xpm"
*/
#include "res/reset_green.xpm"
#include "res/reset_orange.xpm"
#include "res/submit_green.xpm"
#include "res/submit_orange.xpm"

#include "res/koan200.xpm"

#include "res/true.xpm"
#include "res/false.xpm"
#include "res/test.xpm"

#include "TestdcApp.h"
#include "TestdcFrm.h"

#include "BmpCheckBox.h"
#include "AngularMeter.h"
#include "LinearMeter.h"
#include "BmpSwitcher.h"
#include "LCDWindow.h"
#include "LCDClock.h"
#include "ClockThread.h"
#include "LinearRegulator.h"
#include "AngularRegulator.h"


#define IDB_BITBUTTON	2048
#define IDB_LINEARREG 2049
#define IDB_ANGREG 2050

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------
// the application icon
#if defined(__WXGTK__) || defined(__WXMOTIF__)
    #include "res/Testdc.xpm"
#endif

#include "res/help.xpm"	// help_xpm

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = 1,
    Minimal_About
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
IMPLEMENT_CLASS(TestdcFrame, wxFrame)
BEGIN_EVENT_TABLE(TestdcFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  TestdcFrame::OnQuit)
    EVT_MENU(Minimal_About, TestdcFrame::OnAbout)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
TestdcFrame::TestdcFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size, style)
{
    // set the frame icon
    SetIcon(wxICON(Testdc));

    // create menu bars
    wxMenu *fileMenu = new wxMenu("", wxMENU_TEAROFF);
    wxMenu *helpMenu = new wxMenu("", wxMENU_TEAROFF);
	
    fileMenu->Append(Minimal_Quit, _("E&xit\tAlt-X"), _("Quit this program"));
    helpMenu->Append(Minimal_About, _("&About...\tCtrl-A"), _("Show about dialog"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _("&File"));
    menuBar->Append(helpMenu, _("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

    // load the image wanted on the toolbar
//  wxBitmap aboutImage("res/help.bmp", wxBITMAP_TYPE_BMP);
//	wxBitmap aboutImage = wxBITMAP(help) ;
	wxBitmap aboutImage(help_xpm);

    // create the toolbar and add our 1 tool to it
    wxToolBar* toolbar = CreateToolBar();
    toolbar->AddTool(Minimal_About, _("About"), aboutImage, _("About this program"));
    toolbar->Realize();

    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(_("Ready"));

	// ---------- Create New Widget --------

	new MyPanel(this);
	
}

// frame destructor
TestdcFrame::~TestdcFrame()
{
}

// event handlers
void TestdcFrame::OnQuit(wxCommandEvent& event)
{
    // TRUE is to force the frame to close
    Close(TRUE);
}

void TestdcFrame::OnAbout(wxCommandEvent& event)
{
    // called when help - about is picked from the menu or toolbar
    wxString msg;
    msg.Printf(_("About Koan Industrial Controls...\n"));
    wxMessageBox(msg, _("About Koan Industrial Controls"), wxOK | wxICON_INFORMATION, this);
//	MyPanel.mLCD->SetValue("AAA") ;
}


// ----------------------------------------------------------------------------
// MyPanel
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyPanel,wxPanel)
	EVT_COMMAND_SCROLL(-1, MyPanel::OnSlider)
	EVT_BITBUTTON_CLICK(IDB_BITBUTTON, MyPanel::OnBitButton)
	EVT_LINEARREG_CHANGE(IDB_LINEARREG, MyPanel::OnLinearRegChange)
	EVT_ANGULARREG_CHANGE(IDB_ANGREG, MyPanel::OnAngRegChange)
	EVT_BUTTON(5, MyPanel::OnToggle )
END_EVENT_TABLE()


void MyPanel::OnBitButton(wxCommandEvent& event)
{
    wxString msg;
	bool state = focusbtn->GetState() ;

	if(state)
		msg.Printf(_("BmpCheckBox Event\nState Pressed\n"));
	else
		msg.Printf(_("BmpCheckBox Event\nState Not Pressed\n"));

    wxMessageBox(msg,"BmpCheckBox", wxOK | wxICON_INFORMATION, this);

}

void MyPanel::OnLinearRegChange(wxCommandEvent& event)
{
	wxString val ;

	val.Printf("%d", linearreg->GetVal()) ;

	mLinearLCD->SetValue(val) ;
}

void MyPanel::OnAngRegChange(wxCommandEvent& event)
{
	wxString val ;
	val.Printf("%d", angularreg->GetValue()) ;
	mAngularLCD->SetValue(val) ;
}

void MyPanel::OnToggle(wxCommandEvent& event)
{
	bmpswitcher->IncState() ;
}


MyPanel::~MyPanel()
{
	delete angularmeter ;
	delete linearmeter ;
	delete bmpswitcher ;
	delete font ;
	delete on ;
	delete off ;
	delete onsel ;
	delete offsel ;
	delete focusbtn ;
	delete mAngularLCD ;
	delete mLCD ;
	delete mClock ;
}

MyPanel::MyPanel(wxWindow* parent,
                 wxWindowID id,
                 const wxPoint& pos,
	             const wxSize&  size,
                 const long     style)
: wxPanel(parent, id, pos, size, style)
{
	int mWidth, mHeight;
	GetParent()->GetClientSize(&mWidth,&mHeight);
	SetSize(mWidth,mHeight);


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//								Visuali
/////////////////////////////////////////////////////////////////////////////////////

	int m_min = 0 ;
	int m_max = 151 ;
	m_nVal = 75 ;

	//Slider
	int flags = wxSL_VERTICAL;
	wxSlider *m_slider;
    m_slider = new wxSlider(this, -1,
                            75, m_min, m_max,
                            wxPoint(5,30), wxDefaultSize,
                            flags);

	
	font = new wxFont(8, wxMODERN , wxNORMAL, wxNORMAL);

	wxStaticText *m_angulartext ;
	m_angulartext = new wxStaticText(this, -1,"AngularMeter", wxPoint(120,10), wxDefaultSize,0, "Label") ;


	//Angular Meter
	angularmeter = new kwxAngularMeter(this, -1, "Focus me",
  				   wxPoint(50,30), wxSize(200,200) );
	angularmeter->SetNumSectors(3);
	angularmeter->SetSectorColor(0, *wxWHITE);
	angularmeter->SetSectorColor(1, *wxGREEN);
	angularmeter->SetSectorColor(2, *wxBLUE);
	angularmeter->SetSectorColor(3, *wxCYAN);
	angularmeter->SetNumTick(9);
	angularmeter->SetAngle(-20, 200);
	angularmeter->SetRange(m_min, m_max);
	angularmeter->SetNeedleColour(*wxRED);
//	angularmeter->SetBackColour(*wxLIGHT_GREY);
	angularmeter->SetTxtFont(*font);
	angularmeter->SetValue(m_nVal);
//	angularmeter->UseNeedle(false) ;
	angularmeter->DrawCurrent(false) ;


	wxStaticText *m_lineartext ;
	m_lineartext = new wxStaticText(this, -1,"LinearMeter", wxPoint(278,10), wxDefaultSize,0, "Label") ;


	//Linear Meter
	linearmeter = new kwxLinearMeter(this,-1,"Focus me",
  				   wxPoint(280,30), wxSize(50,200) );

	linearmeter->SetLabel("Test") ;
	linearmeter->SetRangeVal(m_min, m_max);
	linearmeter->SetActiveBarColour(wxBLUE);
	linearmeter->SetPassiveBarColour(wxWHITE);
	linearmeter->SetTxtLimitColour(wxBLACK);
	linearmeter->SetTxtValueColour(wxRED);
	linearmeter->SetBorderColour(wxRED);
	linearmeter->SetTagsColour(wxGREEN) ;
	linearmeter->SetTxtFont(*font);
	linearmeter->SetOrizDirection(false);
	linearmeter->ShowCurrent(true) ;
	linearmeter->ShowLimits(true) ;
	linearmeter->SetValue(m_nVal);
	linearmeter->AddTag(90) ;
	linearmeter->AddTag(20) ;
	linearmeter->AddTag(40) ;

	wxStaticText *m_displaytxt ;
	m_displaytxt = new wxStaticText(this, -1,"LCDDisplay", wxPoint(25,230), wxDefaultSize,0, "Label") ;

	//LCD Display
	mLCD = new kwxLCDDisplay( this, wxPoint(25,250), wxSize(150,40) );
	mLCD->SetNumberDigits( 6 );
	mLCD->SetValue("75") ;


	//LCD Clock
	mClock = new kwxLCDClock(this, wxPoint(190,250), wxSize(140,40) );
	mClock->SetNumberDigits( 8 );
	mClock->StartClock() ;

	wxStaticText *m_lcdtext ;
	m_lcdtext = new wxStaticText(this, -1,"LCDClock", wxPoint(190,230), wxDefaultSize,0, "LCDClock") ;



	falseImage = new wxBitmap(false_xpm);
	trueImage = new wxBitmap(true_xpm);
	testImage = new wxBitmap(test_xpm);


	wxStaticText *m_switchtxt ;
	m_switchtxt = new wxStaticText(this, -1,"BmpSwitcher", wxPoint(113,300), wxDefaultSize,0, "Label") ;


	//Bmp Switcher	
	bmpswitcher = new kwxBmpSwitcher(this, -1, "Focus me", wxPoint(110,315), wxSize(60,60) ) ;
	bmpswitcher->SetLabel("Test") ;
	bmpswitcher->SetState(0) ;
	bmpswitcher->AddBitmap(falseImage) ;
	bmpswitcher->AddBitmap(trueImage) ;
	bmpswitcher->AddBitmap(testImage) ;

	wxButton *m_button ;
	m_button = new wxButton(this, 5, "Toggle Bitmap", wxPoint(20,330), wxDefaultSize) ;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//								Controlli
/////////////////////////////////////////////////////////////////////////////////////
	
	wxStaticText *m_lineregtxt ;
	m_lineregtxt = new wxStaticText(this, -1,"LinearRegulator", wxPoint(380,10), wxDefaultSize,0, "Label") ;
	
	wxStaticText *m_angregtxt ;
	m_angregtxt = new wxStaticText(this, -1,"AngularRegulator", wxPoint(510,27), wxDefaultSize,0, "Label") ;

	linearreg = new kwxLinearRegulator(this,IDB_LINEARREG,"Focus me", wxPoint(390,30), wxSize(50,200), wxBORDER_NONE );
	linearreg->SetLabel("Test") ;
	linearreg->SetTxtFont(*font);
/*
	linearreg->SetOrizDirection(false) ;
	linearreg->SetRangeVal(m_min, 100);
	linearreg->SetActiveBarColour(wxBLUE);
	linearreg->SetPassiveBarColour(wxWHITE);
	linearreg->SetTxtLimitColour(wxBLACK);
	linearreg->SetTxtValueColour(wxRED);
	linearreg->SetBorderColour(wxRED);
	linearreg->SetTagsColour(wxGREEN) ;	
*/
	linearreg->SetOrizDirection(false) ;

	linearreg->ShowCurrent(false) ;

	linearreg->AddTag(30) ;
	linearreg->AddTag(80) ;
	linearreg->SetVal(10);	//deve essere l'ultimo impostato

	mLinearLCD = new kwxLCDDisplay( this, wxPoint(390,235), wxSize(49,31) );
	mLinearLCD->SetNumberDigits( 3 );
	mLinearLCD->SetValue("10") ;

	border = new wxColour(82,150,243);
	knob = new wxColour(181,209,246);
	
	angularreg = new kwxAngularRegulator(this,IDB_ANGREG,"Focus me",
  				   wxPoint(500,50), wxSize(100,100), wxBORDER_NONE );
	angularreg->SetRange(m_min, m_max) ;
	angularreg->SetAngle(-45, 215) ;
	angularreg->SetExtCircleColour(border) ;
	angularreg->SetIntCircleColour(wxBLUE) ;
	angularreg->SetTagsColour(wxWHITE);
	angularreg->SetKnobBorderColour(wxLIGHT_GREY) ;
	angularreg->SetKnobColour(knob) ;
	angularreg->AddTag(40) ;
//	angularreg->AddTag(60) ;
//	angularreg->AddTag(70) ;
	angularreg->AddTag(100) ;
	angularreg->SetValue(0) ;

	mAngularLCD = new kwxLCDDisplay( this, wxPoint(530,160), wxSize(49,31) );
	mAngularLCD->SetNumberDigits( 3 );
	mAngularLCD->SetValue("0") ;


	// Focus demo
	on = new wxBitmap(submit_green_xpm);
	onsel = new wxBitmap(submit_orange_xpm);

	off = new wxBitmap(reset_green_xpm);
	offsel = new wxBitmap(reset_orange_xpm);


	wxStaticText *m_focustxt ;
	m_focustxt = new wxStaticText(this, -1,"BmpCheckBox", wxPoint(510,220), wxDefaultSize,0, "Label") ;


	focusbtn = new kwxBmpCheckBox(this, IDB_BITBUTTON, "Focus me", *on,*off,*onsel,*offsel,
  				   wxPoint(510,240), wxSize(83,30) , wxBORDER_NONE);


	focusbtn->SetLabel("Test") ;
	focusbtn->SetBorder(false, wxDOT);



//	wxBitmap *koan = new wxBitmap(submit_green_xpm);

	wxStaticBitmap *staticbmp ;

	staticbmp = new wxStaticBitmap(this,-1,wxBitmap(koan200_xpm),wxPoint(470,320), wxSize(200,150));


	wxStaticText *m_url ;
	m_url = new wxStaticText(this, -1,"http://www.koansoftware.com", wxPoint(5,436), wxDefaultSize,0, "Label") ;
	m_url->SetFont(wxFont(25, wxSWISS, wxNORMAL, wxNORMAL, FALSE, _T("Tahoma")));
	
////////////////////////////////////////////////////////////
	//TEST ANGULAR METER
/*
	speedmeter = new kwxAngularMeter(this, -1, "Focus me",
  				   wxPoint(400,25), wxSize(200,200) );

	speedmeter->SetAngle(-40, 180);
	speedmeter->SetBorderColour(*wxGREEN) ;
	speedmeter->SetRange(m_min, m_max);

	tempmeter = new kwxAngularMeter(this, -1, "Focus me",
  				   wxPoint(500,140), wxSize(70,70) );

	tempmeter->SetAngle(-45, 135);
	tempmeter->SetRange(m_min, m_max);
	
*/
/////////////////////////////////////////////////////////////	
}

void MyPanel::OnSlider(wxScrollEvent& event)
{		
	int pos = event.GetPosition() ;
	wxString sPos;

	sPos.Printf("%d", pos) ;

//	wxLogTrace("pos = %d", pos) ;

	angularmeter->SetValue(pos);

//	speedmeter->SetValue(pos);

	linearmeter->SetValue(pos);
	mLCD->SetValue(sPos) ;

}

