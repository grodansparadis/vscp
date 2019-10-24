/////////////////////////////////////////////////////////////////////////////
// Name:        testinterfaceapp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     15/04/2009 10:27:55
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

#include "testinterfaceapp.h"

////@begin XPM images
////@end XPM images


/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( TestInterfaceApp )
////@end implement app


/*!
 * TestInterfaceApp type definition
 */

IMPLEMENT_CLASS( TestInterfaceApp, wxApp )


/*!
 * TestInterfaceApp event table definition
 */

BEGIN_EVENT_TABLE( TestInterfaceApp, wxApp )

////@begin TestInterfaceApp event table entries
////@end TestInterfaceApp event table entries

END_EVENT_TABLE()


/*!
 * Constructor for TestInterfaceApp
 */

TestInterfaceApp::TestInterfaceApp()
{
    Init();
}


/*!
 * Member initialisation
 */

void TestInterfaceApp::Init()
{
////@begin TestInterfaceApp member initialisation
////@end TestInterfaceApp member initialisation
}

/*!
 * Initialisation for TestInterfaceApp
 */

bool TestInterfaceApp::OnInit()
{    
////@begin TestInterfaceApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	TestInterface* mainWindow = new TestInterface(NULL);
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end TestInterfaceApp initialisation

    return true;
}


/*!
 * Cleanup for TestInterfaceApp
 */

int TestInterfaceApp::OnExit()
{    
////@begin TestInterfaceApp cleanup
	return wxApp::OnExit();
////@end TestInterfaceApp cleanup
}

