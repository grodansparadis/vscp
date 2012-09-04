/////////////////////////////////////////////////////////////////////////////
// Name:        testinterfaceapp.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     15/04/2009 10:27:55
// RCS-ID:      
// Copyright:   (c) 2009 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _TESTINTERFACEAPP_H_
#define _TESTINTERFACEAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "testinterface.h"
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
 * TestInterfaceApp class declaration
 */

class TestInterfaceApp: public wxApp
{    
    DECLARE_CLASS( TestInterfaceApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    TestInterfaceApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin TestInterfaceApp event handler declarations

////@end TestInterfaceApp event handler declarations

////@begin TestInterfaceApp member function declarations

////@end TestInterfaceApp member function declarations

////@begin TestInterfaceApp member variables
////@end TestInterfaceApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(TestInterfaceApp)
////@end declare app

#endif
    // _TESTINTERFACEAPP_H_
