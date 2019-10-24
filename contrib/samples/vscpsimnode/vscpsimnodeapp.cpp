/////////////////////////////////////////////////////////////////////////////
// Name:        vscpsimnodeapp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 10 Nov 2007 17:24:27 CET
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

#include "vscpsimnodeapp.h"

#include "../../common/vscp.h"

////@begin XPM images
////@end XPM images


// * * * Globals

/// Register storage
unsigned char g_registers[ 256 ];

/// Periodic node event 
vscpEventEx g_eventPeriodic;

/// Node events
vscpEventEx g_eventOne;
vscpEventEx g_eventTwo;
vscpEventEx g_eventThree;



/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( VSCPSimNodeApp )
////@end implement app


/*!
 * VSCPSimNodeApp type definition
 */

IMPLEMENT_CLASS( VSCPSimNodeApp, wxApp )


/*!
 * VSCPSimNodeApp event table definition
 */

BEGIN_EVENT_TABLE( VSCPSimNodeApp, wxApp )

////@begin VSCPSimNodeApp event table entries
////@end VSCPSimNodeApp event table entries

END_EVENT_TABLE()


/*!
 * Constructor for VSCPSimNodeApp
 */

VSCPSimNodeApp::VSCPSimNodeApp()
{
    Init();
}


/*!
 * Member initialisation
 */

void VSCPSimNodeApp::Init()
{
////@begin VSCPSimNodeApp member initialisation
////@end VSCPSimNodeApp member initialisation
}

/*!
 * Initialisation for VSCPSimNodeApp
 */

bool VSCPSimNodeApp::OnInit()
{    
////@begin VSCPSimNodeApp initialisation
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
	DlgSimulatedVSCPNode* mainWindow = new DlgSimulatedVSCPNode(NULL);
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end VSCPSimNodeApp initialisation

    return true;
}


/*!
 * Cleanup for VSCPSimNodeApp
 */

int VSCPSimNodeApp::OnExit()
{    
////@begin VSCPSimNodeApp cleanup
	return wxApp::OnExit();
////@end VSCPSimNodeApp cleanup
}

