/////////////////////////////////////////////////////////////////////////////
// Name:        frmdeviceconfig.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sun 04 May 2009 17:28:13 CEST
//      
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

//
// http://www.iconarchive.com/
//

#ifndef _FRMDEVICECONFIG_H_
#define _FRMDEVICECONFIG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmdeviceconfig.h"
#endif


#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/tglbtn.h"
#include "wx/toolbook.h"
#include "wx/grid.h"
#include "wx/html/htmlwin.h"

#include "vscpworks.h"
#include <canalsuperwrapper.h>
#include <register.h>
#include <mdf.h>

#define MAX_CONFIG_REGISTER_PAGE    22

#define STATUSBAR_STATUS_RIGHT      1
#define STATUSBAR_STATUS_LEFT       0

class wxToggleButton;
class wxToolbook;
class wxGrid;
class wxHtmlWindow;

#define SYMBOL_FRMDEVICECONFIG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMDEVICECONFIG_TITLE _("VSCP Configuration Window Session")
#define SYMBOL_FRMDEVICECONFIG_IDNAME ID_FRMDEVICECONFIG

#ifdef WIN32
#define VSCPWORKS_DEVICECONFIG_SIZE wxSize(1020, 724)
#define VSCPWORKS_DEVICECONFIG_POSITION wxDefaultPosition
#else
#define VSCPWORKS_DEVICECONFIG_SIZE wxSize(1040, 790)
#define VSCPWORKS_DEVICECONFIG_POSITION wxDefaultPosition
#endif


DECLARE_EVENT_TYPE(wxVSCP_STATUS_CHANGE_EVENT, wxID_ANY )       // status change

enum {
    Menu_Popup_Register_Read_Value = 2000,
    Menu_Popup_Register_Write_Value,
    Menu_Popup_Register_Update,
    Menu_Popup_Load_MDF,
    Menu_Popup_Undo,
    Menu_Popup_Default,
    Menu_Popup_dm_enable_row,
    Menu_Popup_dm_disable_row,
    Menu_Popup_dm_row_wizard,
    Menu_Popup_go_VSCP,
    Menu_Popup_GotoRegPage,
    Menu_Popup_Full_Edit,        
    Menu_Popup_Abstraction_Update,
    Menu_Popup_Abstraction_Read_Value,
    Menu_Popup_Abstraction_Write_Value,
    Menu_Popup_Abstraction_Undo,
    Menu_Popup_Abstraction_Default,
};

// Register grid positions
#define GRID_REGISTER_COLUMN_REGISTER           0
#define GRID_REGISTER_COLUMN_ACCESS_RIGHTS      1
#define GRID_REGISTER_COLUMN_VALUE              2
#define GRID_REGISTER_COLUMN_DESCRIPTION        3

// Abstraction grid positions
#define GRID_ABSTRACTION_COLUMN_NAME            0
#define GRID_ABSTRACTION_COLUMN_TYPE            1
#define GRID_ABSTRACTION_COLUMN_ACCESS_RIGHTS   2
#define GRID_ABSTRACTION_COLUMN_VALUE           3
#define GRID_ABSTRACTION_COLUMN_DESCRIPTION     4

class frmDeviceConfig : public wxFrame {
    DECLARE_CLASS(frmDeviceConfig)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmDeviceConfig();
    frmDeviceConfig( wxWindow* parent, 
                        wxWindowID id = SYMBOL_FRMDEVICECONFIG_IDNAME, 
                        const wxString& caption = SYMBOL_FRMDEVICECONFIG_TITLE, 
                        const wxPoint& pos = VSCPWORKS_DEVICECONFIG_POSITION,
                        const wxSize& size = VSCPWORKS_DEVICECONFIG_SIZE,
                        long style = SYMBOL_FRMDEVICECONFIG_STYLE);

    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_FRMDEVICECONFIG_IDNAME, 
                    const wxString& caption = SYMBOL_FRMDEVICECONFIG_TITLE, 
                    const wxPoint& pos = VSCPWORKS_DEVICECONFIG_POSITION,
                    const wxSize& size = VSCPWORKS_DEVICECONFIG_SIZE,
                    long style = SYMBOL_FRMDEVICECONFIG_STYLE );

    /// Destructor
    ~frmDeviceConfig();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /*!
        Enable communication interface
        @return true on success
     */
    bool enableInterface(void);

    /*!
        Disable communication interface
        @return true on success
     */
    bool disableInterface(void);


    /*!
        Write status information
     */
    void writeStatusInfo(void);


    /*!
        Read all Level 1 registers for anode
        @return True on success, false on failure.
     */
    //bool readAllLevel2Registers( void );

    bool writeChangedLevel1Registers(unsigned char nodeid);

    bool writeChangedLevel2Registers(void);


    /*!
        Clear grid and other data content
     */
    void clearAllContent(void);
    
    /*!
        Update selected register rows
        @param bUpdate Updates abstractions and dm after write.
     */
    void doWriteValueSelectedRegisterRow( bool bUpdate = true ) ;

    /*!
        Read value for selected row
     */
    void readValueSelectedRegisterRow(wxCommandEvent& event);

    /*!
        Write value for selected row
     */
    void writeValueSelectedRegisterRow(wxCommandEvent& event );

    /*!
        Undo value for selected row
     */
    void undoValueSelectedRegisterRow(wxCommandEvent& event);

    /*!
        Default value for selected row
     */
    void defaultValueSelectedRegisterRow(wxCommandEvent& event);
    
    /*!
        Read value for selected row
     */
    void readValueSelectedAbstractionRow(wxCommandEvent& event);

    /*!
        Write value for selected row
     */
    void writeValueSelectedAbstractionRow(wxCommandEvent& event);

    /*!
        Undo value for selected row
     */
    void undoValueSelectedAbstractionRow(wxCommandEvent& event);

    /*!
        Default value for selected row
     */
    void defaultValueSelectedAbstractionRow(wxCommandEvent& event);
    
    /*!
        Write modified abstraction rows
     */
    void writeAbstractionRows(wxCommandEvent& event);
    
    /*!
        Goto register page
     */
    void gotoRegisterPage( wxCommandEvent& event );
    
    /*!
        Read content of remote device for a specific register and update the
        corresponding row.
        @param row the register row to update.
        @param bUpdateDM Updates DM data after register update if true.
        @param bUpdateAbstractions Updates abstraction data after register update if true
        @return true on success.
     */
    bool readUpdateRegisterRow(uint16_t row, bool bUpdateDM = true, bool bUpdateAbstractions = true);

    /*! 
        Update the DM grid
     */
    void updateDmGrid(void);

    /*!
        Update decision matrix grid if a certain page:row is part 
        of it.
     */
    void updateDmGridConditional(uint16_t reg, uint32_t page);

    /*!
        Update the abstraction grid
     */
    void updateAbstractionGrid(void);

    /*!
        Update abstraction grid if a certain page:row is part 
        of it.
     */
    void updateAbstractionGridConditional(uint16_t reg, uint32_t page);
    
    /*!
     * Save edited abstraction rows to register grid
     * @param bShowDialog True to show dialog.
     */
    void saveAbstractionEdits( bool bShowDialog = true );

    /*!
        Enable selected DM row
     */
    void dmEnableSelectedRow(wxCommandEvent& event);

    /*!
        Disable selected DM row
     */
    void dmDisableSelectedRow(wxCommandEvent& event);

    /*!
        Start wizard that help users to set up a DM row
     */
    void dmRowWizard(wxCommandEvent& event);

    /*!
        Get register from cell
        @return Register
     */
    uint32_t getRegFromCell(int row);

    /*!
        Get page from cell
        @return Page
     */
    uint16_t getPageFromCell(int row);

    /*!
        Fill grid with standard registers
     */
    void fillStandardRegisters(void);


    /*!
        Do a full update
    */
    void doUpdate( void );

    /*!
        Get formatted value according to set configuration

        @param val Value to format
        @return Formatted value.
     */
    wxString getFormattedValue(uint8_t val);

    /*!
        Return the row for a register:page
        @param reg Register to look for.
        @aram page Page to look for.
        @return Grid row where register:page is located. -1 is
        returned if row is not found.
     */
    int getRegisterGridRow(uint32_t reg, uint16_t page);

    /*!
        Fetch the GUID for a daemon interface from it's name
        @param VSCP daemon interface structure for TCP/IP interface.
        @return True on success.
     */
    bool fetchIterfaceGUID(void);

    /*!
        Message handler that sets status messages on status bar
    */
    void eventStatusChange( wxCommandEvent &evt );

    /// Windows resize handler
    void OnResizeWindow( wxSizeEvent& event );

    /// wxEVT_CLOSE_WINDOW event handler for ID_FRMDEVICECONFIG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SAVE_REGSITERS
    void OnMenuitemSaveRegistersClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM
    void OnMenuitemSaveSelectedRegistersClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_LOAD_REGISTES
    void OnMenuitemLoadRegistersClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_ADD_GUIDS
    void OnMenuitemAddGuidsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SAVE_GUIDS
    void OnMenuitemSaveGuidsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_LOAD_GUIDS
    void OnMenuitemLoadGuidsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SET_MANUFACTURER_INFO
    void OnMenuitemSetManufacturerInfoClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_EXIT
    void OnMenuitemExitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP
    void OnMenuitemVscpHelpClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_FAQ
    void OnMenuitemVscpFaqClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_KB_SHRTCUTS
    void OnMenuitemVscpShortcutsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_THANKS
    void OnMenuitemVscpThanksClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CREDITS
    void OnMenuitemVscpCreditsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GO_VSCP_SITE
    void OnMenuitemVscpVscpSiteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_ABOUT
    void OnMenuitemVscpAboutClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX4
    void OnComboNodeIDSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COMBOBOX4
    void OnComboNodeIDUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CHECK_LEVEL2
    void OnBitmapbuttonTestDeviceClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOGGLEBUTTON1
    void OnInterfaceActivate( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_REGISTER
    void OnGotoRegister( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_TOP
    void OnGotoTop( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE1
    void OnGotoPage1( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE2
    void OnGotoPage2( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE3
    void OnGotoPage3( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE4
    void OnGotoPage4( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE5
    void OnGotoPage5( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE6
    void OnGotoPage6( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE7
    void OnGotoPage7( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE8
    void OnGotoPage8( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_PAGE9
    void OnGotoPage9( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_DM
    void OnGotoDM( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_GOTO_STANDARD
    void OnGotoStandard( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_MENU_SELECTED event handler for Menu_Popup_Full_Edit
    void OnAbstractionFullEdit( wxCommandEvent& eventv );

    /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_GRID_REGISTERS
    void OnCellLeftClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_REGISTERS
    void OnCellRightClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_GRID_REGISTERS
    void OnLeftDClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_CHANGE event handler for ID_GRID_REGISTERS
    void OnRegisterEdited( wxGridEvent& event );

    /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW1
    void OnHtmlwindowCellClicked( wxHtmlLinkEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
    void OnButtonUpdateClick( wxCommandEvent& event );
    
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
    void OnButtonUpdateAllClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON17
    void OnButtonLoadDefaultsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON19
    void OnButtonWizardClick( wxCommandEvent& event );
    
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    // Array with GUID's
    wxArrayString m_guidarray;

    /// Flag for first read
    bool m_bFirstRead;

    /// Interface type
    int m_interfaceType;

    /*! 
        GUID for interface or all
        zero if no interface selected
        uint8_t m_interfaceGUID[ 16 ];
     */
    cguid m_ifguid;

    /*! 
        Name of interface or NULL string if
        interface is not used.
        wxString m_interfaceName;
    */
    
    /*!
        Data for the selected device
        id holds type of i/f (DLL, TCP/IP
    */
    canal_interface m_canalif;
    vscp_interface m_vscpif;

    /*!
        CANAL driver level
     */
    unsigned char m_driverLevel;

    /*!
        The wrapper for the CANAL 
        functionality.
     */
    CCanalSuperWrapper m_csw;

    /// Standard registers
    CStandardRegisters m_stdRegisters;

    /// User registers
    CUserRegisters m_userRegisters;

    /// Module description file functionality
    CMDF m_mdf;

    //static const int m_constGridRegisterWidth = 70;

    enum {
        m_constGridRegisterDefaultWidth = 70
    };

    enum {
        m_constGridAccessRightDefaultWidth = 20
    };

    enum {
        m_constGridContentdefaultWidth = 80
    };

    enum {
        m_constGridDescriptionDefaultWidth = 4000
    };

    enum {
        m_constGridAbstractionNameDefaultWidth = 230
    };

    enum {
        m_constGridAbstractionTypeDefaultWidth = 170
    };

    enum {
        m_constGridAbstractionAccessDefaultWidth = 50
    };

    enum {
        m_constGridAbstractionContentdefaultWidth = 100
    };

    enum {
        m_constGridAbstractionDescriptionDefaultWidth = 4000
    };

    enum {
        m_constGridDMOrigAddressDefaultWidth = 90
    };

    enum {
        m_constGridDMFlagsDefaultWidth = 90
    };

    enum {
        m_constGridDMMasksDefaultWidth = 90
    };

    enum {
        m_constGridDMActionDefaultWidth = 90
    };

    enum {
        m_constGridDMActionParamDefaultWidth = 90
    };

    // Last register click column/row
    int m_lastLeftRegisterClickCol;
    int m_lastLeftRegisterClickRow;

    // Last register click column/row
    int m_lastLeftAbstractionClickCol;
    int m_lastLeftAbstractionClickRow;
    
    // Last register click column/row
    int m_lastLeftDMClickCol;
    int m_lastLeftDMClickRow;
    
    // Last register click column/row
    int m_lastLeftWizardClickCol;
    int m_lastLeftWizardClickRow;
    
    // Row where standard registers start
    int m_rowStandardRegisterStart;
    
    /// Should we show tooltips?
    static bool ShowToolTips();

    wxStatusBar* m_pitemStatusBar;
    wxComboBox* m_comboNodeID;
    wxCheckBox* m_bLevel2;
    wxToggleButton* m_BtnActivateInterface;
    wxToolbook* m_choiceBook;
    wxPanel* m_panelRegisters;
    wxGrid* m_gridRegisters;
    wxGrid* m_gridAbstractions;
    wxGrid* m_gridDM;
    wxGrid* m_gridWizard;
    wxHtmlWindow* m_StatusWnd;
    wxCheckBox* m_chkFullUpdate;
    wxCheckBox* m_chkMdfFromFile;
    wxButton* m_ctrlButtonLoadMDF;
    wxButton* m_ctrlButtonWizard;
  
    /// Control identifiers
    enum {
        ID_FRMDEVICECONFIG = 19000,
        ID_MENUITEM_SAVE_REGISTERS = 19001,
        ID_MENUITEM = 19002,
        ID_MENUITEM_LOAD_REGISTES = 19003,
        ID_MENUITEM_ADD_GUIDS = 19004,
        ID_MENUITEM_SAVE_GUIDS = 19005,
        ID_MENUITEM_LOAD_GUIDS = 10006,
        ID_MENUITEM_EXIT = 19007,
        ID_MENUITEM_HELP = 19008,
        ID_MENUITEM_FAQ = 19009,
        ID_MENUITEM_KB_SHRTCUTS = 19010,
        ID_MENUITEM_THANKS = 19011,
        ID_MENUITEM_CREDITS = 19012,
        ID_MENUITEM_GO_VSCP_SITE = 19013,
        ID_MENUITEM_ABOUT = 19014,
        ID_TOOLBAR_DEVICE_CONFIG = 19015,
        ID_TOOL6 = 19016,
        ID_TOOL7 = 19017,
        ID_TOOL8 = 19018,
        ID_TOOL9 = 19019,
        ID_TOOL10 = 19020,
        ID_TOOL11 = 19021,
        ID_COMBOBOX4 = 19021,
        ID_CHECK_LEVEL2 = 19023,
        ID_CHECKBOX_LEVEL22 = 19024,
        ID_TOGGLEBUTTON1 = 19025,
        ID_PANEL_DEVICE_CONFIG = 19026,
        ID_CHOICEBOOK = 19027,
        ID_PANEL_REGISTERS = 19028,
        ID_GRID_REGISTERS = 19029,
        ID_PANEL_ABSTRACTIONS = 19030,
        ID_GRID_ABSTRACTIONS = 19031,
        ID_PANEL_DM = 19032,
        ID_PANEL_WIZARD = 19033,
        ID_GRID_DM = 19034,
        ID_GRID_WIZARD = 19035,
        ID_HTMLWINDOW1 = 19036,
        ID_CHECKBOX_FULL_UPDATE = 19037,
        ID_CHECKBOX_MDF_FROM_FILE = 19038,
        ID_BUTTON_UPDATE = 19039,
        ID_BUTTON_LOAD_DEFAULTS = 19040,
        ID_BUTTON_WIZARD = 19041,
        ID_STATUSBAR = 19042,
        ID_MENUITEM_SET_MANUFACTURER_INFO = 19043,
        ID_MENUITEM_UPDATE = 19044,
        ID_MENUITEM_UPDATE_ALL = 19045,
        ID_MENUITEM_LOAD_DEFAULTS = 19046,
        ID_MENUITEM_WIZARD = 19047,
        ID_MENUITEM_GOTO_REGISTER = 19048,
        ID_MENUITEM_GOTO_TOP = 19049,
        ID_MENUITEM_GOTO_PAGE1 = 19050,
        ID_MENUITEM_GOTO_PAGE2 = 19051,
        ID_MENUITEM_GOTO_PAGE3 = 19052,
        ID_MENUITEM_GOTO_PAGE4 = 19053,
        ID_MENUITEM_GOTO_PAGE5 = 19054,
        ID_MENUITEM_GOTO_PAGE6 = 19055,
        ID_MENUITEM_GOTO_PAGE7 = 19056,
        ID_MENUITEM_GOTO_PAGE8 = 19057,
        ID_MENUITEM_GOTO_PAGE9 = 19058,
        ID_MENUITEM_GOTO_DM = 19059,
        ID_MENUITEM_GOTO_STANDARD = 19060,
    };

};



#endif
// _FRMDEVICECONFIG_H_
