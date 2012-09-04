/////////////////////////////////////////////////////////////////////////////
// Name:        t2.h
// Purpose:     
// Author:      CanMsgDlg
// Modified by: 
// Created:     20/07/2006 22:15:05
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _T2_H_
#define _T2_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "t2.h"
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
 * T2App class declaration
 */

class T2App: public wxApp
{    
    DECLARE_CLASS( T2App )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    T2App();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin T2App event handler declarations
////@end T2App event handler declarations

////@begin T2App member function declarations
////@end T2App member function declarations

////@begin T2App member variables
////@end T2App member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(T2App)
////@end declare app

#endif
    // _T2_H_
