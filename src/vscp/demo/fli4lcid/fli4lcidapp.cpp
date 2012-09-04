/////////////////////////////////////////////////////////////////////////////
// Name:        fli4lcidapp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 15 Nov 2007 15:24:05 CET
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

#include "fli4lcidapp.h"

////@begin XPM images
////@end XPM images


/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( Fli4lcidApp )
////@end implement app


/*!
 * Fli4lcidApp type definition
 */

IMPLEMENT_CLASS( Fli4lcidApp, wxApp )


/*!
 * Fli4lcidApp event table definition
 */

BEGIN_EVENT_TABLE( Fli4lcidApp, wxApp )

////@begin Fli4lcidApp event table entries
////@end Fli4lcidApp event table entries

END_EVENT_TABLE()


/*!
 * Constructor for Fli4lcidApp
 */

Fli4lcidApp::Fli4lcidApp()
{
    Init();
}


/*!
 * Member initialisation
 */

void Fli4lcidApp::Init()
{
////@begin Fli4lcidApp member initialisation
////@end Fli4lcidApp member initialisation
}

/*!
 * Initialisation for Fli4lcidApp
 */

bool Fli4lcidApp::OnInit()
{    
////@begin Fli4lcidApp initialisation
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
	fli4lCIDtest* mainWindow = new fli4lCIDtest(NULL, ID_FLI4LCIDTEST, _("fli4l CID test"));
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end Fli4lcidApp initialisation

    return true;
}


/*!
 * Cleanup for Fli4lcidApp
 */

int Fli4lcidApp::OnExit()
{    
////@begin Fli4lcidApp cleanup
	return wxApp::OnExit();
////@end Fli4lcidApp cleanup
}

