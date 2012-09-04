/////////////////////////////////////////////////////////////////////////////
// Name:        vscpsimnodeapp.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 10 Nov 2007 17:24:27 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _VSCPSIMNODEAPP_H_
#define _VSCPSIMNODEAPP_H_



/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "dlgsimulatedvscpnode.h"
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
 * VSCPSimNodeApp class declaration
 */

class VSCPSimNodeApp: public wxApp
{    
    DECLARE_CLASS( VSCPSimNodeApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    VSCPSimNodeApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin VSCPSimNodeApp event handler declarations

////@end VSCPSimNodeApp event handler declarations

////@begin VSCPSimNodeApp member function declarations

////@end VSCPSimNodeApp member function declarations

////@begin VSCPSimNodeApp member variables
////@end VSCPSimNodeApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(VSCPSimNodeApp)
////@end declare app

#endif
    // _VSCPSIMNODEAPP_H_
