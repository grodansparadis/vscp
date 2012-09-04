/////////////////////////////////////////////////////////////////////////////
// Name:        cw.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/02/2007 18:11:18
// RCS-ID:      
// Copyright:   (c) 2007 Ake Hedman, eurosource, <ake@eurosource.se>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "cw.h"
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

#include "cw.h"

////@begin XPM images

////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( cw )
////@end implement app

/*!
 * cw type definition
 */

IMPLEMENT_CLASS( cw, wxApp )

/*!
 * cw event table definition
 */

BEGIN_EVENT_TABLE( cw, wxApp )

////@begin cw event table entries
////@end cw event table entries

END_EVENT_TABLE()

/*!
 * Constructor for cw
 */

cw::cw()
{
    Init();
}

/*!
 * Member initialisation 
 */

void cw::Init()
{
////@begin cw member initialisation
////@end cw member initialisation
}
/*!
 * Initialisation for cw
 */

bool cw::OnInit()
{    
////@begin cw initialisation
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
////@end cw initialisation

    return true;
}

/*!
 * Cleanup for cw
 */
int cw::OnExit()
{    
////@begin cw cleanup
    return wxApp::OnExit();
////@end cw cleanup
}

