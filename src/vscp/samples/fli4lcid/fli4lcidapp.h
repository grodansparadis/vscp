/////////////////////////////////////////////////////////////////////////////
// Name:        fli4lcidapp.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 15 Nov 2007 15:24:05 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FLI4LCIDAPP_H_
#define _FLI4LCIDAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "fli4lcidtest.h"
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
 * Fli4lcidApp class declaration
 */

class Fli4lcidApp: public wxApp
{    
    DECLARE_CLASS( Fli4lcidApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    Fli4lcidApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin Fli4lcidApp event handler declarations

////@end Fli4lcidApp event handler declarations

////@begin Fli4lcidApp member function declarations

////@end Fli4lcidApp member function declarations

////@begin Fli4lcidApp member variables
////@end Fli4lcidApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(Fli4lcidApp)
////@end declare app

#endif
    // _FLI4LCIDAPP_H_
