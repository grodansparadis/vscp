/////////////////////////////////////////////////////////////////////////////
// Name:        dlgconfiguration.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 18:30:18 CEST
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
// Copyright (C) 2000-2011 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifndef _DLGCONFIGURATION_H_
#define _DLGCONFIGURATION_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgconfiguration.h"
#endif

/*!
* Includes
*/

#include "wx/propdlg.h"
#include "wx/spinctrl.h"


/*!
* Forward declarations
*/

class wxSpinCtrl;


/*!
* Control identifiers
*/

#define SYMBOL_DLGCONFIGURATION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGCONFIGURATION_TITLE _("Program settings")
#define SYMBOL_DLGCONFIGURATION_IDNAME ID_DLGCONFIGURATION
#define SYMBOL_DLGCONFIGURATION_SIZE wxSize(600, 300)
#define SYMBOL_DLGCONFIGURATION_POSITION wxDefaultPosition


/*!
* dlgConfiguration class declaration
*/

class dlgConfiguration: public wxPropertySheetDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgConfiguration )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgConfiguration();
    dlgConfiguration( wxWindow* parent, 
                        wxWindowID id = SYMBOL_DLGCONFIGURATION_IDNAME, 
                        const wxString& caption = SYMBOL_DLGCONFIGURATION_TITLE, 
                        const wxPoint& pos = SYMBOL_DLGCONFIGURATION_POSITION, 
                        const wxSize& size = SYMBOL_DLGCONFIGURATION_SIZE, 
                        long style = SYMBOL_DLGCONFIGURATION_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGCONFIGURATION_IDNAME, 
                    const wxString& caption = SYMBOL_DLGCONFIGURATION_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGCONFIGURATION_POSITION, 
                    const wxSize& size = SYMBOL_DLGCONFIGURATION_SIZE, 
                    long style = SYMBOL_DLGCONFIGURATION_STYLE );

    /// Destructor
    ~dlgConfiguration();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /*!
        Get Dialogdata
        @param 	bWriteToConfigFile write data to system config file
        @return True if everything is OK
    */
    bool getDialogData( bool bWriteToConfigFile=true );

    ////@begin dlgConfiguration event handler declarations

    /// wxEVT_INIT_DIALOG event handler for ID_PANEL_RECEIVE
    void OnInitDialog( wxInitDialogEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
    void OnButtonSetRxGridTextColourClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
    void SetRxGridBackgroundColourClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
    void OnButtonSetTxGridTextColourClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON7
    void OnButtonSetTxGridBackgroundColourClick( wxCommandEvent& event );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxStaticText* m_labelLogFile;
    wxCheckBox* m_checkEnableLogging;
    wxChoice* m_comboLogLevel;
    wxCheckBox* m_checkConfirmDeletes;
    wxCheckBox* m_checkManufacturerInfo;
    wxChoice* m_comboNumericalBase;
    
    wxSpinCtrl* m_SpinCtrlmaxRetries;
    wxSpinCtrl* m_SpinCtrlreadTimeout;
    wxSpinCtrl* m_SpinCtrltotalTimeout;
    
    wxSpinCtrl* m_SpinCtrlResponseTimeout;
    wxSpinCtrl* m_SpinCtrlmaxTCPIPRetries;
    wxSpinCtrl* m_SpinCtrltcpipReadTimeout;
    wxSpinCtrl* m_SpinCtrltcpipTotalTimeout;

    wxCheckBox* m_ChkAutoScroll;
    wxCheckBox* m_chkPyjamasLook;
    wxCheckBox* m_chkUseSymbols;

    wxTextCtrl* m_editRxForeGroundColour;
    wxTextCtrl* m_editRxBackGroundColour;
    wxTextCtrl* m_editTxForeGroundColour;
    wxTextCtrl* m_editTxBackGroundColour;

    /// Control identifiers
    enum {
        ID_DLGCONFIGURATION = 15000,
        ID_PANEL_GENERAL = 15001,
        ID_CHECKBOX1 = 15040,
        ID_CHOICE3 = 15041,
        ID_CHECKBOX3 = 15042,
        ID_CHOICE = 15039,
        ID_PANEL_COMMUNICATION = 15003,
        ID_SPINCTRL = 15002,
        ID_SPINCTRL1 = 15038,
        ID_PANEL_COLORS = 15004,
        ID_PANEL_RECEIVE = 15005,
        ID_ChkAutoScroll = 15036,
        ID_ChkPyjamasLook = 15035,
        ID_CHECKBOX = 15037,
        ID_EditRxForeGroundColour = 15006,
        ID_BUTTON4 = 15007,
        ID_EditRxBackGroundColour = 15008,
        ID_BUTTON5 = 15009,
        ID_EditTxForeGroundColour = 15010,
        ID_BUTTON6 = 15011,
        ID_EditTxBackGroundColour = 15012,
        ID_BUTTON7 = 15013,
        ID_BUTTON = 15014,
        ID_BUTTON1 = 15015,
        ID_CHECKLISTBOX1 = 15016,
        ID_BUTTON3 = 15017,
        ID_BUTTON8 = 15018,
        ID_CHECKLISTBOX = 15019,
        ID_PANEL_TRANSMIT = 15020,
        ID_TEXTCTRL = 15021,
        ID_BUTTON2 = 15022,
        ID_TEXTCTRL1 = 15023,
        ID_BUTTON9 = 15024,
        ID_TEXTCTRL3 = 15025,
        ID_BUTTON10 = 15026,
        ID_TEXTCTRL4 = 15027,
        ID_BUTTON11 = 15028,
        ID_BUTTON12 = 15029,
        ID_BUTTON13 = 15030,
        ID_CHECKLISTBOX2 = 15031,
        ID_BUTTON14 = 15032,
        ID_BUTTON15 = 15033,
        ID_CHECKLISTBOX3 = 15034,
        ID_CHECKBOX_MANUFACTURER = 15050
    };
    
};

#endif
  // _DLGCONFIGURATION_H_
