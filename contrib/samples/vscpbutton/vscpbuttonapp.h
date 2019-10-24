/////////////////////////////////////////////////////////////////////////////
// Name:        vscpbuttonapp.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 15 Dec 2007 15:55:15 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _VSCPBUTTONAPP_H_
#define _VSCPBUTTONAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "vscpbutton.h"
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
 * VscpbuttonApp class declaration
 */

class VscpbuttonApp: public wxApp
{    
    DECLARE_CLASS( VscpbuttonApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    VscpbuttonApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin VscpbuttonApp event handler declarations

////@end VscpbuttonApp event handler declarations

////@begin VscpbuttonApp member function declarations

////@end VscpbuttonApp member function declarations

////@begin VscpbuttonApp member variables
////@end VscpbuttonApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(VscpbuttonApp)
////@end declare app

#endif
    // _VSCPBUTTONAPP_H_
