/////////////////////////////////////////////////////////////////////////////
// Name:        t3.h
// Purpose:     
// Author:      CanMsgDlg
// Modified by: 
// Created:     20/07/2006 22:43:38
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _T3_H_
#define _T3_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "t3.h"
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
 * T3App class declaration
 */

class T3App: public wxApp
{    
    DECLARE_CLASS( T3App )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    T3App();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin T3App event handler declarations
////@end T3App event handler declarations

////@begin T3App member function declarations
////@end T3App member function declarations

////@begin T3App member variables
////@end T3App member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(T3App)
////@end declare app

#endif
    // _T3_H_
