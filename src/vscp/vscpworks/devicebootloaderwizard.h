/////////////////////////////////////////////////////////////////////////////
// Name:        devicebootloaderwizard.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:10
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

#ifndef _DEVICEBOOTLOADERWIZARD_H_
#define _DEVICEBOOTLOADERWIZARD_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "devicebootloaderwizard.h"
#endif

/*!
* Includes
*/


#include "wx/wizard.h"
#include "wx/html/htmlwin.h"

#include "canalsuperwrapper.h"
#include "mdf.h"

#include "bootdevice.h"
#include "bootdevice_pic1.h"

// Bootloader index in selection combo
#define BOOTLOADER_VSCP       0
#define BOOTLOADER_PIC1       1

#define BOOT_LOADER_WIZARD_TITLE _( "VSCP  Bootloader Wizard" )

/*!
* Forward declarations
*/

class WizardPageStart;
class WizardPageSelecInterface;
class WizardPageSelectFirmware;
class WizardPageSelectBootloader;
class WizardPageBootload;
class WizardPageSetGUID;
class WizardPageProgramDevice;

class wxHtmlWindow;


/*!
* Control identifiers
*/


#define SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME ID_DEVICEBOOTLOADERWIZARD



///////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard class declaration
//

class DeviceBootloaderwizard : public wxWizard
{
    DECLARE_DYNAMIC_CLASS( DeviceBootloaderwizard )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DeviceBootloaderwizard();
    DeviceBootloaderwizard( wxWindow* parent,
                            wxWindowID id = SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME,
                            const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME,
                 const wxPoint& pos = wxDefaultPosition );

    /// Destructor
    ~DeviceBootloaderwizard();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Runs the wizard
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    WizardPageSelecInterface* m_pgSelecInterface;
    WizardPageSetGUID* m_pgSelecDeviceId;
    WizardPageSelectBootloader* m_pgSelecAlgorithm;
    WizardPageSelectFirmware* m_pgLoadFile;

    /// Control identifiers
    enum {
        ID_DEVICEBOOTLOADERWIZARD = 32000
    };

    /*!
        Bootcontrol code
        Holds control code for selected algorithm.
    */
    CBootDevice *m_pBootCtrl;

    /*!
    Type of the current selected interface
    */
    uint8_t m_iftype;

    /// Holder for dll i/f
    canal_interface m_canalif;

    /*!
        DLL interface
    */
    CDllWrapper m_dll;

    /// Holder for tcp/ip i/f
    vscp_interface m_vscpif;

    /*!
        TCP/IP interface
    */
    VscpRemoteTcpIf m_tcpip;

    /// True if node found
    bool m_bDeviceFound;

    /// Flag for interface selection
    bool m_bInterfaceSelected;


    /// Flag for firmware file loaded
    bool m_bHexFileLoaded;


    /*!
        Flag to indicate thet the mdf has been fetched.
        */
    bool m_bMDFLoaded;


    /*!
        MDF informaton for node under bootload.
    */
    CMDF m_mdf;


    /*!
        GUID for interface where node is located
        Set from interface name
    */
    cguid m_ifguid;


    /*!
        GUID for node to bootload
        Note! This is a duplicate to m_vscpif.m_GUID
        here for convinience.
        node is for CANAL node is stored in lsb
     */
    cguid m_guid;
   
};


///////////////////////////////////////////////////////////////////////////////
// WizardPageStart class declaration
//

class WizardPageStart : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageStart();

    WizardPageStart( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageStart();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Control identifiers
    enum {
        ID_WIZARDPAGE = 32001
    };

};

///////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface class declaration
//

class WizardPageSelecInterface : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageSelecInterface();

    WizardPageSelecInterface( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageSelecInterface();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_SELECT_INTERFACE
    void OnWizardPageChanging( wxWizardEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON20
    void OnButtonSelectInterfaceClick( wxCommandEvent& event );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxStaticText* m_labelInterfaceSelected;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE_SELECT_INTERFACE = 32002,
        ID_BUTTON20 = 32003
    };

};


///////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID class declaration
//

class WizardPageSetGUID : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage6 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageSetGUID();

    WizardPageSetGUID( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageSetGUID();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_SELECT_INTERFACE
    void OnWizardPageChanging( wxWizardEvent& event );

    wxComboBox* m_comboNodeID;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE_NICKNAME_SELECTION = 32011,
        ID_COMBOBOX_NODEID = 19037
    };

};


///////////////////////////////////////////////////////////////////////////////
// WizardPage3 class declaration
//

class WizardPageSelectBootloader : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage3 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageSelectBootloader();

    WizardPageSelectBootloader( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageSelectBootloader();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /*!
        Select bootloader
    */
    void selectBootLoader( int nBootloader = BOOTLOADER_VSCP );

    /*!
        Fetch bootloader algorithm from mdf
    */
    void fetchAlgorithmFromMdf( void );


    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_BOOTLOADER_ALGORITHM
    void OnWizardPageChanging( wxWizardEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE2
    void OnBootLoaderAlgorithmSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ALGORITHM_FROM_MDF
    void OnButtonAlgorithmFromMdfClick( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxChoice* m_nBootAlgorithm;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE_BOOTLOADER_ALGORITHM = 32004,
        ID_CHOICE2 = 32005,
        ID_BUTTON_ALGORITHM_FROM_MDF = 32006
    };

};


///////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware class declaration
//

class WizardPageSelectFirmware : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage2 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageSelectFirmware();

    WizardPageSelectFirmware( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageSelectFirmware();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_LOAD_FILE
    void OnWizardpagePageChanging( wxWizardEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILE
    void OnButtonChooseFileClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILE_FROM_MDF
    void OnButtonLoadFileFromMdfClick( wxCommandEvent& event );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxHtmlWindow* m_hexFileInfo;
    wxStaticText* m_selectedFile;
    wxButton* m_ctrlBtnLoadFile;
    wxButton* m_ctrlBtnLoadFileFromMDF;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE_LOAD_FILE = 32007,
        ID_HTMLWINDOW4 = 32008,
        ID_BUTTON_LOAD_FILE = 32009,
        ID_BUTTON_LOAD_FILE_FROM_MDF = 32013
    };

};


///////////////////////////////////////////////////////////////////////////////
// WizardPageBootload class declaration
//

class WizardPageBootload : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage5 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageBootload();

    WizardPageBootload( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageBootload();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_PRE_BOOTLOAD
    void OnWizardpagePreBootloadPageChanging( wxWizardEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Control identifiers
    enum {
        ID_WIZARDPAGE_PRE_BOOTLOAD = 32010
    };

};



///////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice class declaration
//

class WizardPageProgramDevice : public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPageProgramDevice )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPageProgramDevice();

    WizardPageProgramDevice( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPageProgramDevice();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON21
    void OnButtonProgramClick( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Control identifiers
    enum {
        ID_WIZARDPAGE_BOOTLOAD = 32012,
        ID_BUTTON21 = 32015
    };

};

#endif
// _DEVICEBOOTLOADERWIZARD_H_
