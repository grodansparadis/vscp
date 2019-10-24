/////////////////////////////////////////////////////////////////////////////
// Name:        autoself.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:41:07 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _AUTOSELF_H_
#define _AUTOSELF_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "autoself.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "frmmain.h"
////@end includes

#define AUTOSELFCTRL_CONFIG_FILE_NAME   "autoself.config"

#define MAX_NUMBER_OF_NODES     1024

// Structure for nodes
typedef struct {
    unsigned char m_GUID[16];
    wxString m_strRealname;
    wxString m_strHostname;
    unsigned char m_zone;
} _nodeinfo;

// Structure for configuration
typedef struct {
    unsigned char m_placeCode;  // 0 = local config file, 1  = global config file
    int m_sizeWidth;    		    // Initial main frame width
    int m_sizeHeight;   		    // Initial main frame height
    int m_xpos;         		    // Initial main frame x position
    int m_ypos;         		    // Initial main frame y position
		wxString m_strServer;			// VSCP Server URL
		wxString m_strPort;				  // VSCP Server Port
		wxString m_strUsername;		  // Username for VSCP server
		wxString m_strPassword;		  // Password for VSCP server
    bool m_bLogFile;            // True for writes to logfile
    wxString m_strPathLogFile;  // Oath to logfile
} _appConfiguration;	


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
 * AutoselfApp class declaration
 */

class AutoselfApp: public wxApp
{    
    DECLARE_CLASS( AutoselfApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    AutoselfApp();

    /// Initialises member variables
    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();
    
    /*!
        Load configuration data
    */
    bool loadConfigData( void );
    
    
    /*!
        Save configuration data
    */
    bool saveConfigData( void );
    

////@begin AutoselfApp event handler declarations

////@end AutoselfApp event handler declarations

////@begin AutoselfApp member function declarations

////@end AutoselfApp member function declarations

////@begin AutoselfApp member variables
////@end AutoselfApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(AutoselfApp)
////@end declare app

#endif
    // _AUTOSELF_H_
