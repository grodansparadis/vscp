/////////////////////////////////////////////////////////////////////////////
// Name:        devicebootloaderwizard.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:10
// RCS-ID:      
// Copyright:   (C) 2007-2013 
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
/////////////////////////////////////////////////////////////////////////////

#ifndef _DEVICEBOOTLOADERWIZARD_H_
#define _DEVICEBOOTLOADERWIZARD_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "devicebootloaderwizard.h"
#endif

/*!
* Includes
*/

////@begin includes
#include "wx/wizard.h"
#include "wx/html/htmlwin.h"
////@end includes

#include "../common/canalsuperwrapper.h"
#include "../common/mdf.h"

#include "bootdevice.h"
#include "bootdevice_pic1.h"

/*!
* Forward declarations
*/

////@begin forward declarations
class WizardPage;
class WizardPage1;
class WizardPage6;
class WizardPage3;
class WizardPage2;
class wxHtmlWindow;
class WizardPage5;
class WizardPage7;
////@end forward declarations

/*!
* Control identifiers
*/

////@begin control identifiers
#define SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME ID_DEVICEBOOTLOADERWIZARD
////@end control identifiers


/*!
* DeviceBootloaderwizard class declaration
*/

class DeviceBootloaderwizard: public wxWizard
{    
    DECLARE_DYNAMIC_CLASS( DeviceBootloaderwizard )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DeviceBootloaderwizard();
    DeviceBootloaderwizard( wxWindow* parent, wxWindowID id = SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DEVICEBOOTLOADERWIZARD_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Destructor
    ~DeviceBootloaderwizard();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin DeviceBootloaderwizard event handler declarations

    ////@end DeviceBootloaderwizard event handler declarations

    ////@begin DeviceBootloaderwizard member function declarations

  /// Runs the wizard
  bool Run();

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end DeviceBootloaderwizard member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin DeviceBootloaderwizard member variables
  WizardPage1* m_pgSelecInterface;
  WizardPage6* m_pgSelecDeviceId;
  WizardPage3* m_pgSelecAlgorithm;
  WizardPage2* m_pgLoadFile;
  /// Control identifiers
  enum {
    ID_DEVICEBOOTLOADERWIZARD = 32000
  };
    ////@end DeviceBootloaderwizard member variables

    /*!
        Bootcontrol code
        Holds control code for selected algorithm.
    */
    CBootDevice *m_pBootCtrl;

    /*!
    The wrapper for the CANAL 
    functionality.
    */
    CCanalSuperWrapper m_csw;

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
        GUID for node to bootload
    */
    cguid m_guid;
};

/*!
* WizardPage class declaration
*/

class WizardPage: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage();

    WizardPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage event handler declarations

    ////@end WizardPage event handler declarations

    ////@begin WizardPage member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage member variables
  /// Control identifiers
  enum {
    ID_WIZARDPAGE = 32001
  };
    ////@end WizardPage member variables
};

/*!
* WizardPage1 class declaration
*/

class WizardPage1: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage1 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage1();

    WizardPage1( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage1();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage1 event handler declarations

  /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_SELECT_INTERFACE
  void OnWizardpageSelectInterfacePageChanging( wxWizardEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON20
  void OnButtonSelectInterfaceClick( wxCommandEvent& event );

    ////@end WizardPage1 event handler declarations

    ////@begin WizardPage1 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage1 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage1 member variables
  wxStaticText* m_labelInterfaceSelected;
  /// Control identifiers
  enum {
    ID_WIZARDPAGE_SELECT_INTERFACE = 32002,
    ID_BUTTON20 = 32003
  };
    ////@end WizardPage1 member variables
};

/*!
* WizardPage2 class declaration
*/

class WizardPage2: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage2 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage2();

    WizardPage2( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage2();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage2 event handler declarations

  /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_LOAD_FILE
  void OnWizardpage3PageChanging( wxWizardEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILE
  void OnButtonChooseFileClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILE_FROM_MDF
  void OnButtonLoadFileFromMdfClick( wxCommandEvent& event );

    ////@end WizardPage2 event handler declarations

    ////@begin WizardPage2 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage2 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage2 member variables
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
    ////@end WizardPage2 member variables
};

/*!
* WizardPage3 class declaration
*/

class WizardPage3: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage3 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage3();

    WizardPage3( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage3();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage3 event handler declarations

  /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_BOOTLOADER_ALGORITHM
  void OnWizardpage2PageChanging( wxWizardEvent& event );

  /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE2
  void OnBootLoaderAlgorithmSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ALGORITHM_FROM_MDF
  void OnButtonAlgorithmFromMdfClick( wxCommandEvent& event );

    ////@end WizardPage3 event handler declarations

    ////@begin WizardPage3 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage3 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage3 member variables
  wxChoice* m_nBootAlgorithm;
  /// Control identifiers
  enum {
    ID_WIZARDPAGE_BOOTLOADER_ALGORITHM = 32004,
    ID_CHOICE2 = 32005,
    ID_BUTTON_ALGORITHM_FROM_MDF = 32006
  };
    ////@end WizardPage3 member variables
};



/*!
* WizardPage5 class declaration
*/

class WizardPage5: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage5 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage5();

    WizardPage5( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage5();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage5 event handler declarations

  /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE_PRE_BOOTLOAD
  void OnWizardpagePreBootloadPageChanging( wxWizardEvent& event );

    ////@end WizardPage5 event handler declarations

    ////@begin WizardPage5 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage5 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage5 member variables
  /// Control identifiers
  enum {
    ID_WIZARDPAGE_PRE_BOOTLOAD = 32010
  };
    ////@end WizardPage5 member variables
};

/*!
* WizardPage6 class declaration
*/

class WizardPage6: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage6 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage6();

    WizardPage6( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage6();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage6 event handler declarations

    ////@end WizardPage6 event handler declarations

    ////@begin WizardPage6 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage6 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage6 member variables
  wxComboBox* m_comboNodeID;
  /// Control identifiers
  enum {
    ID_WIZARDPAGE_NICKNAME_SELECTION = 32011,
    ID_COMBOBOX_NODEID = 19037
  };
    ////@end WizardPage6 member variables
};

/*!
* WizardPage7 class declaration
*/

class WizardPage7: public wxWizardPageSimple
{    
    DECLARE_DYNAMIC_CLASS( WizardPage7 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage7();

    WizardPage7( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage7();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    ////@begin WizardPage7 event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON21
  void OnButtonProgramClick( wxCommandEvent& event );

    ////@end WizardPage7 event handler declarations

    ////@begin WizardPage7 member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
    ////@end WizardPage7 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    ////@begin WizardPage7 member variables
  /// Control identifiers
  enum {
    ID_WIZARDPAGE_BOOTLOAD = 32012,
    ID_BUTTON21 = 32015
  };
    ////@end WizardPage7 member variables
};

#endif
// _DEVICEBOOTLOADERWIZARD_H_
