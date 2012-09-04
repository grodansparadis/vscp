/////////////////////////////////////////////////////////////////////////////
// Name:        TestdcApp.cpp
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

#include "TestdcApp.h"
#include "TestdcFrm.h"
#include "resource.h"



// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. TestdcApp and
// not wxApp)
IMPLEMENT_APP(TestdcApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

TestdcApp::TestdcApp()
{
}

TestdcApp::~TestdcApp()
{
}

// 'Main program' equivalent: the program execution "starts" here
bool TestdcApp::OnInit()
{

    // create the main application window
    TestdcFrame *pFrame = new TestdcFrame(_("Koan Industrial Controls"),
                                 wxPoint(50, 50), wxSize(700, 585));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    // pFrame->Maximize(true) ;
    pFrame->Show(TRUE);
	SetTopWindow(pFrame);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

