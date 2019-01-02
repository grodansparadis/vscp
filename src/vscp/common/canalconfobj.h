// FILE: canalobj.h 
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#ifndef _CANALOBJ_H_
#define _CANALOBJ_H_

#include "wx/wx.h"
#include <wx/window.h>
#include "wx/wizard.h"
#include "wx/html/htmlwin.h"

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <canal.h>
#include <canaldlldef.h>
#include <dllwrapper.h>
#include <crc.h>

#define MAX_PARAMETERS 128
#define MAX_FLAGS 32

#define DEFAULT_STOCK_TEXT_WIDTH    400     // Max width for descriptionsa, headers etc

enum canalobj_type {
    type_unknown,
    type_string,
    type_boolean,
    type_int32,
    type_uint32,
    type_float,
    type_choice,
    type_isotime,
    type_isodate,
    type_isodatetime,
    type_guid,
    type_event,
    type_filter,
    type_mask
};

enum typeDescription {
    type_text,
    type_html
};

enum flagtype {
    flagtype_bool,
    flagtype_value,
    flagtype_choice
};

// Forward declarations
class WizardCanalConfigPageStart;
class WizardPageCanalConfig;
class WizardPageFlagsConfig;

/*!
    CCanalObj_Choice
*/

class CCanalObj_Choice
{

public:

    CCanalObj_Choice();
    ~CCanalObj_Choice();

    std::string m_value;             // The value for this choice
    std::string m_description;       // User description of item

};

WX_DECLARE_LIST( CCanalObj_Choice, CANALOBJ_CHOICE_LIST );

/*!
    CCanalObj_OneItem

    Encapsulates one item of the configuration string
*/

class CCanalObj_OneItem
{

public:

    CCanalObj_OneItem();
    ~CCanalObj_OneItem();

    uint16_t m_pos;             // Position in configuration string (base = 0)
    bool m_bOptional;           // True if optional
    canalobj_type m_type;       // Type for this
    std::string m_description;     // User description of item
    std::string m_infourl;         // Pointer to web-based help page

    CANALOBJ_CHOICE_LIST m_listChoice;  // List with choices if type = choice
    std::string m_strValue;                // Value (always in string form)

};

WX_DECLARE_LIST( CCanalObj_OneItem, CANALOBJ_ITEM_LIST );

/*!
CCanalObj_OneItem

Encapsulates one item of the configuration string
*/

class CCanalObj_FlagBit
{

public:

    CCanalObj_FlagBit();
    ~CCanalObj_FlagBit();

    uint16_t m_pos;             // Position in flags (base = 0)
    uint8_t m_width;            // Width in number of bits
    std::string m_description;     // User description of item
    std::string m_infourl;         // Pointer to web-based help page
    flagtype m_type;            // Type for flag bool, value or choice
    uint32_t m_defaultVal;      // Default value for 'value' type

    CANALOBJ_CHOICE_LIST m_listChoice;  // List with choices if type = choice

};

WX_DECLARE_LIST( CCanalObj_FlagBit, CANALOBJ_FLAGBIT_LIST );

/*!
    CCanalConfObj
*/

class CCanalConfObj
{

public:

    CCanalConfObj();
    ~CCanalConfObj();

    /*!
        Parse driver info
        @param  info Driver info fetched from file
        @return true if the parsing went well.
    */
    bool parseDriverInfo( std::string& xmldata );

    /*!
        Run the graphical wizard that assist the user in setting the parameters.
        @param parent Pointer to parent window.
        @param inputConfigString Original configuration string
        @param inputConfigFlags Original configuration flags
        @param resultConfigString Configuration  result string.
        @param resultConfigFlags Resulting flag value
        @return true if the parsing went well.
    */
    bool runWizard( wxWindow* parent,
                        std::string& inputConfigString,
                        uint32_t inputConfigFlags,
                        std::string& resultConfigString, 
                        uint32_t *presultConfigFlags );

    std::string m_decription;              // Description of DLL
    typeDescription m_TypeDescription;  // Format for description (text or html)
    std::string m_infourl;                 // Pointer to site with info about driver

    int m_level;                        // Level for driver
    bool m_bBlocking;                   // True if driver is blocking

    CDllWrapper m_dll;

    CANALOBJ_ITEM_LIST m_listItem;          // List with configuration item definitions
    CANALOBJ_FLAGBIT_LIST m_listFlagBits;   // List with flag bit definitions
};


#define CANAL_CONFIG_WIZARD_TITLE _( "CANAL Configurator Wizard" )


///////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard class declaration
//

class CanalConfigWizard : public wxWizard
{
    DECLARE_DYNAMIC_CLASS( CanalConfigWizard )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CanalConfigWizard();
    CanalConfigWizard( wxWindow* parent,
                            wxWindowID id = ID_CANALCONFIGRWIZARD,
                            const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent,
                    wxWindowID id = ID_CANALCONFIGRWIZARD,
                    const wxPoint& pos = wxDefaultPosition );

    /// Destructor
    ~CanalConfigWizard();

    /// Initialises member variables
    void Init();

    /*! 
        Creates the controls and sizers
        @param pObj Ponter to the canal config object
        @param inputConfigString Original configuration string.
        @param inputConfigFlags Original configuration flags.
    */
    void CreateControls( CCanalConfObj  *pObj, std::string& inputConfigString, uint32_t inputConfigFlags );

    /// Runs the wizard
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    WizardCanalConfigPageStart* m_pgStart;
    WizardPageCanalConfig  *m_pgConfig[ MAX_PARAMETERS ];
    WizardPageFlagsConfig  *m_pgConfigFlags[ MAX_FLAGS ];

    /// Control identifiers
    enum {
        ID_CANALCONFIGRWIZARD = 32000
    };

    // The configuration object
    CCanalConfObj  *m_pconfigObj;
};


///////////////////////////////////////////////////////////////////////////////
// WizardCanalConfigPageStart
//

class WizardCanalConfigPageStart : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardCanalConfigPageStart )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardCanalConfigPageStart();

    WizardCanalConfigPageStart( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardCanalConfigPageStart();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Control identifiers
    enum {
        ID_WIZARDPAGE = 32001
    };

    // The configuration object
    CCanalConfObj  *m_pconfigObj;

};


///////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig
//

class WizardPageCanalConfig : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPageCanalConfig )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageCanalConfig();

    WizardPageCanalConfig( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageCanalConfig();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_SELECT_INTERFACE
    void OnWizardPageChanging( wxWizardEvent& event );

    // Id to use for creating controls. Icrease by one for every control created.
    int m_windowsID;

    // Text for head  "Parameter 0", "Parameter 1"  etc 
    std::string m_strHead;

    // Parameter item associated with this wizard page
    CCanalObj_OneItem *m_pItem;

    // True if this parametr is optional
    bool m_bOptional;

    // This is the string value for the configuration string
    std::string  m_strValue;

    // Choices for strings are here
    wxArrayString m_listBoxStrings;

    wxTextCtrl* m_textField;    // Used for most things
    wxListBox* m_listBox;       // Used for items with a choice
    wxCheckBox* m_boolChoice;     // Used for single options

    /// Control identifiers
    enum {
        ID_WIZARDPAGE_BASE= 32011
    };

};


///////////////////////////////////////////////////////////////////////////////
// WizardPageFlagsConfig
//

class WizardPageFlagsConfig : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPageCanalConfig )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageFlagsConfig();

    WizardPageFlagsConfig( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageFlagsConfig();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_SELECT_INTERFACE
    void OnWizardPageChanging( wxWizardEvent& event );

    /*!
        Get the flags data
    */
    uint32_t getData( uint8_t pos, uint8_t width );

    // Id to use for creating controls. Icrease by one for every control created.
    int m_windowsID;

    // Text for head  "Flags setting 0", "Flags setting 1"  etc 
    std::string m_strHead;

    // Parameter flag item associated with this wizard page
    CCanalObj_FlagBit *m_pItem;

    // This is the value for the bit or the bit combination. Values from different
    // flag config values are ored together forming the end result flag value.
    uint32_t m_value;

    // This is the original flags value
    uint32_t m_flags;

    // Choices for strings are here
    wxArrayString m_listBoxStrings;

    wxTextCtrl* m_textField;    // Used for most things
    wxListBox* m_listBox;       // Used for items with a choice
    wxCheckBox* m_boolChoice;     // Used for single options

    /// Control identifiers
    enum {
        ID_WIZARDPAGE_BASE = 37011
    };

};












#endif


