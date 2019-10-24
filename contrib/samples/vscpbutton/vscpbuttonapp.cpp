/////////////////////////////////////////////////////////////////////////////
// Name:        vscpbuttonapp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 15 Dec 2007 15:55:14 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
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

#include "vscpbuttonapp.h"

////@begin XPM images
////@end XPM images


/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( VscpbuttonApp )
////@end implement app


/*!
 * VscpbuttonApp type definition
 */

IMPLEMENT_CLASS( VscpbuttonApp, wxApp )


/*!
 * VscpbuttonApp event table definition
 */

BEGIN_EVENT_TABLE( VscpbuttonApp, wxApp )

////@begin VscpbuttonApp event table entries
////@end VscpbuttonApp event table entries

END_EVENT_TABLE()


/*!
 * Constructor for VscpbuttonApp
 */

VscpbuttonApp::VscpbuttonApp()
{
    Init();
}


/*!
 * Member initialisation
 */

void VscpbuttonApp::Init()
{
////@begin VscpbuttonApp member initialisation
////@end VscpbuttonApp member initialisation
}

/*!
 * Initialisation for VscpbuttonApp
 */

bool VscpbuttonApp::OnInit()
{    
////@begin VscpbuttonApp initialisation
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
	VSCPButton* mainWindow = new VSCPButton(NULL);
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end VscpbuttonApp initialisation
}


/*!
 * Cleanup for VscpbuttonApp
 */

int VscpbuttonApp::OnExit()
{    
////@begin VscpbuttonApp cleanup
	return wxApp::OnExit();
////@end VscpbuttonApp cleanup
}

