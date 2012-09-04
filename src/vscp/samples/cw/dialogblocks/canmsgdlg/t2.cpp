/////////////////////////////////////////////////////////////////////////////
// Name:        t2.cpp
// Purpose:     
// Author:      CanMsgDlg
// Modified by: 
// Created:     20/07/2006 22:15:05
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "t2.h"
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

#include "t2.h"

////@begin XPM images

////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( T2App )
////@end implement app

/*!
 * T2App type definition
 */

IMPLEMENT_CLASS( T2App, wxApp )

/*!
 * T2App event table definition
 */

BEGIN_EVENT_TABLE( T2App, wxApp )

////@begin T2App event table entries
////@end T2App event table entries

END_EVENT_TABLE()

/*!
 * Constructor for T2App
 */

T2App::T2App()
{
////@begin T2App member initialisation
////@end T2App member initialisation
}

/*!
 * Initialisation for T2App
 */

bool T2App::OnInit()
{    
////@begin T2App initialisation
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
////@end T2App initialisation

    return true;
}

/*!
 * Cleanup for T2App
 */
int T2App::OnExit()
{    
////@begin T2App cleanup
    return wxApp::OnExit();
////@end T2App cleanup
}

