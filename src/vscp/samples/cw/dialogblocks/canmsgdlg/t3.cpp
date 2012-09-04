/////////////////////////////////////////////////////////////////////////////
// Name:        t3.cpp
// Purpose:     
// Author:      CanMsgDlg
// Modified by: 
// Created:     20/07/2006 22:43:38
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "t3.h"
#endif

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

#include "t3.h"

////@begin XPM images

////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( T3App )
////@end implement app

/*!
 * T3App type definition
 */

IMPLEMENT_CLASS( T3App, wxApp )

/*!
 * T3App event table definition
 */

BEGIN_EVENT_TABLE( T3App, wxApp )

////@begin T3App event table entries
////@end T3App event table entries

END_EVENT_TABLE()

/*!
 * Constructor for T3App
 */

T3App::T3App()
{
////@begin T3App member initialisation
////@end T3App member initialisation
}

/*!
 * Initialisation for T3App
 */

bool T3App::OnInit()
{    
////@begin T3App initialisation
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
////@end T3App initialisation

    return true;
}

/*!
 * Cleanup for T3App
 */
int T3App::OnExit()
{    
////@begin T3App cleanup
    return wxApp::OnExit();
////@end T3App cleanup
}

