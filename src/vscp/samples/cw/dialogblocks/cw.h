/////////////////////////////////////////////////////////////////////////////
// Name:        cw.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/02/2007 18:11:18
// RCS-ID:      
// Copyright:   (c) 2007 Ake Hedman, eurosource, <ake@eurosource.se>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _CW_H_
#define _CW_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "cw.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * cw class declaration
 */

class cw: public wxApp
{    
    DECLARE_CLASS( cw )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    cw();

    /// Initialises member variables
    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin cw event handler declarations
////@end cw event handler declarations

////@begin cw member function declarations
////@end cw member function declarations

////@begin cw member variables
////@end cw member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(cw)
////@end declare app

#endif
    // _CW_H_
