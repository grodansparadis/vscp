/////////////////////////////////////////////////////////////////////////////
// Name:        dlgconfiguration.cpp
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgconfiguration.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/bookctrl.h"

#include <wx/colordlg.h>
#include <wx/imaglist.h>

#include "vscpworks.h"
#include "dlgconfiguration.h"
#include "dlgconfiguration_images.h"


extern appConfiguration g_Config;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgConfiguration, wxPropertySheetDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration event table definition
//

BEGIN_EVENT_TABLE( dlgConfiguration, wxPropertySheetDialog )

    EVT_BUTTON( ID_BUTTON4, dlgConfiguration::OnButtonSetRxGridTextColourClick )
    EVT_BUTTON( ID_BUTTON5, dlgConfiguration::SetRxGridBackgroundColourClick )
    EVT_BUTTON( ID_BUTTON6, dlgConfiguration::OnButtonSetTxGridTextColourClick )
    EVT_BUTTON( ID_BUTTON7, dlgConfiguration::OnButtonSetTxGridBackgroundColourClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration constructors
//

dlgConfiguration::dlgConfiguration()
{
    Init();
}

dlgConfiguration::dlgConfiguration( wxWindow* parent, 
                                        wxWindowID id, 
                                        const wxString& caption, 
                                        const wxPoint& pos, 
                                        const wxSize& size, 
                                        long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration creator
//

bool dlgConfiguration::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    SetSheetStyle(wxPROPSHEET_DEFAULT);
    wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

    CreateButtons(wxOK|wxCANCEL|wxHELP);
    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    LayoutDialog();
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration destructor
//

dlgConfiguration::~dlgConfiguration()
{
    ;   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgConfiguration::Init()
{
    m_labelLogFile = NULL;
    m_checkEnableLogging = NULL;
    m_comboLogLevel = NULL;
    m_checkConfirmDeletes = NULL;
    m_checkManufacturerInfo = NULL;
    m_comboNumericalBase = NULL;
    m_SpinCtrlmaxRetries = NULL;
    m_SpinCtrlreadTimeout = NULL;
    m_ChkAutoScroll = NULL;
    m_chkPyjamasLook = NULL;
    m_chkUseSymbols = NULL;
    m_editRxForeGroundColour = NULL;
    m_editRxBackGroundColour = NULL;
    m_editTxForeGroundColour = NULL;
    m_editTxBackGroundColour = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgConfiguration
//

void dlgConfiguration::CreateControls()
{    
    dlgConfiguration* itemPropertySheetDialog1 = this;

    wxImageList* itemPropertySheetDialog1ImageList = new wxImageList(16, 16, true, 5);
    {
        wxIcon itemPropertySheetDialog1Icon0( GetIconResource( wxT("copy.xpm") ) );
        itemPropertySheetDialog1ImageList->Add( itemPropertySheetDialog1Icon0 );
        wxIcon itemPropertySheetDialog1Icon1( GetIconResource( wxT("copy.xpm") ) );
        itemPropertySheetDialog1ImageList->Add( itemPropertySheetDialog1Icon1 );
        wxIcon itemPropertySheetDialog1Icon2( GetIconResource( wxT("copy.xpm") ) );
        itemPropertySheetDialog1ImageList->Add( itemPropertySheetDialog1Icon2 );
        wxIcon itemPropertySheetDialog1Icon3( GetIconResource( wxT("copy.xpm") ) );
        itemPropertySheetDialog1ImageList->Add( itemPropertySheetDialog1Icon3);
        wxIcon itemPropertySheetDialog1Icon4( GetIconResource( wxT("copy.xpm") ) );
        itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon4 );
    }
    GetBookCtrl()->AssignImageList( itemPropertySheetDialog1ImageList );

    // General
    wxPanel* itemPanel2 = new wxPanel;
    itemPanel2->Create( GetBookCtrl(), ID_PANEL_GENERAL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemPanel2->SetName(wxT("general"));
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    wxGridSizer* itemGridSizerGeneral = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizer3->Add(itemGridSizerGeneral, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemGridSizerGeneral->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerGeneral->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemPanel2, wxID_STATIC, _("Logfile :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    
    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerGeneral->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_labelLogFile = new wxStaticText;
    m_labelLogFile->Create( itemPanel2, wxID_STATIC, _("---"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_labelLogFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemPanel2, wxID_STATIC, _("Enable logging :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerGeneral->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_checkEnableLogging = new wxCheckBox;
    m_checkEnableLogging->Create( itemPanel2, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_checkEnableLogging->SetValue(false);
    itemBoxSizer9->Add(m_checkEnableLogging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText;
    itemStaticText11->Create( itemPanel2, wxID_STATIC, _("Log Level :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerGeneral->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxArrayString m_comboLogLevelStrings;
    m_comboLogLevelStrings.Add(_("DEBUG - Highest"));
    m_comboLogLevelStrings.Add(_("INFO"));
    m_comboLogLevelStrings.Add(_("NOTICE"));
    m_comboLogLevelStrings.Add(_("WARNING"));
    m_comboLogLevelStrings.Add(_("ERROR"));
    m_comboLogLevelStrings.Add(_("CRITICAL"));
    m_comboLogLevelStrings.Add(_("ALERT"));
    m_comboLogLevelStrings.Add(_("EMERGENCY . Lowest"));
    m_comboLogLevel = new wxChoice;
    m_comboLogLevel->Create( itemPanel2, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, m_comboLogLevelStrings, 0 );
    itemBoxSizer12->Add(m_comboLogLevel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemGridSizerGeneral->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemGridSizerGeneral->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText;
    itemStaticText16->Create( itemPanel2, wxID_STATIC, _("Confirm deletes :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerGeneral->Add(itemBoxSizer17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_checkConfirmDeletes = new wxCheckBox;
    m_checkConfirmDeletes->Create( itemPanel2, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_checkConfirmDeletes->SetValue(false);
    itemBoxSizer17->Add(m_checkConfirmDeletes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText;
    itemStaticText19->Create( itemPanel2, wxID_STATIC, _("Base to use for numbers :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerGeneral->Add(itemBoxSizer20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxArrayString m_comboNumericalBaseStrings;
    m_comboNumericalBaseStrings.Add(_("Hexadecimal"));
    m_comboNumericalBaseStrings.Add(_("Decimal"));
    m_comboNumericalBase = new wxChoice;
    m_comboNumericalBase->Create( itemPanel2, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_comboNumericalBaseStrings, 0 );
    itemBoxSizer20->Add(m_comboNumericalBase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticTextManufacturerInfo = new wxStaticText;
    itemStaticTextManufacturerInfo->Create( itemPanel2, wxID_STATIC, _( "Enable manufacturer functionality :" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerGeneral->Add( itemStaticTextManufacturerInfo, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    wxBoxSizer* itemBoxSizerManufacturer = new wxBoxSizer( wxHORIZONTAL );
    itemGridSizerGeneral->Add( itemBoxSizerManufacturer, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    m_checkManufacturerInfo = new wxCheckBox;
    m_checkManufacturerInfo->Create( itemPanel2, ID_CHECKBOX_MANUFACTURER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_checkManufacturerInfo->SetValue( false );
    itemBoxSizerManufacturer->Add( m_checkConfirmDeletes, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    GetBookCtrl()->AddPage(itemPanel2, _("General"), false, 0);


    // =============================================================================================================================


    // Communication
    wxPanel* itemPanelCommunication = new wxPanel;
    itemPanelCommunication->Create( GetBookCtrl(), ID_PANEL_COMMUNICATION, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizerCommunication = new wxBoxSizer(wxVERTICAL);
    itemPanelCommunication->SetSizer( itemBoxSizerCommunication );

    // ---

    wxGridSizer* itemGridSizerCommunication = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizerCommunication->Add( itemGridSizerCommunication, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    // ---

    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Header ( Level I )

    wxStaticText* itemStaticTextCANALCaption = new wxStaticText;
    itemStaticTextCANALCaption->Create( itemPanelCommunication, wxID_STATIC, _("Level I (CANAL)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticTextCANALCaption->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Sans")));
    itemGridSizerCommunication->Add(itemStaticTextCANALCaption, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Numer of retries

    wxStaticText* itemStaticTextRetries = new wxStaticText;
    itemStaticTextRetries->Create( itemPanelCommunication, wxID_STATIC, _("Max number of register read/write retries :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTextRetries, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerRetries = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizerRetries, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_SpinCtrlmaxRetries = new wxSpinCtrl;
    m_SpinCtrlmaxRetries->Create( itemPanelCommunication, ID_SPINCTRL, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 1 );
    itemBoxSizerRetries->Add( m_SpinCtrlmaxRetries, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Resend timeout

    wxStaticText* itemStaticTextResend = new wxStaticText;
    itemStaticTextResend->Create( itemPanelCommunication, wxID_STATIC, _("Read/write timeout in milliseconds before resend :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTextResend, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerResend = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizerResend, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_SpinCtrlreadTimeout = new wxSpinCtrl;
    m_SpinCtrlreadTimeout->Create( itemPanelCommunication, ID_SPINCTRL1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 10000, 1000 );
    itemBoxSizerResend->Add(m_SpinCtrlreadTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Total timeout

    wxStaticText* itemStaticTextTotal = new wxStaticText;
    itemStaticTextTotal->Create( itemPanelCommunication, wxID_STATIC, _("Total register read/write timeout in milliseconds :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTextTotal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerTotal = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizerTotal, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    
    m_SpinCtrltotalTimeout = new wxSpinCtrl;
    m_SpinCtrltotalTimeout->Create( itemPanelCommunication, ID_SPINCTRL1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1000, 20000, 1000 );
    itemBoxSizerTotal->Add(m_SpinCtrltotalTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -------------------------------------------------------------------------

    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);


    // -- Header Level II / TCP/IP

    wxStaticText* itemStaticTextTCPIPCaption = new wxStaticText;
    itemStaticTextTCPIPCaption->Create( itemPanelCommunication, wxID_STATIC, _("Level II (TCP/IP)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticTextTCPIPCaption->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Sans")));
    itemGridSizerCommunication->Add(itemStaticTextTCPIPCaption, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // --


    // -- TCP/IP - General resend timout in seconds
    wxStaticText* itemStaticTextTCPIPResponse = new wxStaticText;
    itemStaticTextTCPIPResponse->Create( itemPanelCommunication, wxID_STATIC, _("Command respons timout in milliseconds :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add( itemStaticTextTCPIPResponse, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerTCPIPResponse = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add( itemBoxSizerTCPIPResponse, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    
    m_SpinCtrlResponseTimeout = new wxSpinCtrl;
    m_SpinCtrlResponseTimeout->Create( itemPanelCommunication, ID_SPINCTRL, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1000, 60000, 3000 );
    itemBoxSizerTCPIPResponse->Add( m_SpinCtrlResponseTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


    // -- TCP/IP - Sleep after command in milliseconds
    wxStaticText* itemStaticTextSleepAfterCommand = new wxStaticText;
    itemStaticTextSleepAfterCommand->Create( itemPanelCommunication, wxID_STATIC, _( "Sleep after command (milliseconds) :" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add( itemStaticTextSleepAfterCommand, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    wxBoxSizer* itemBoxSizerTextSleepAfterCommand = new wxBoxSizer( wxHORIZONTAL );
    itemGridSizerCommunication->Add( itemBoxSizerTextSleepAfterCommand, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );


    // -- Number of retries
    wxStaticText* itemStaticTextTCPIPRetries = new wxStaticText;
    itemStaticTextTCPIPRetries->Create( itemPanelCommunication, wxID_STATIC, _("Max number of register read/write retries :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTextTCPIPRetries, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerTCPIPRetries = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizerTCPIPRetries, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_SpinCtrlmaxTCPIPRetries = new wxSpinCtrl;
    m_SpinCtrlmaxTCPIPRetries->Create( itemPanelCommunication, ID_SPINCTRL, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 3 );
    itemBoxSizerTCPIPRetries->Add(m_SpinCtrlmaxTCPIPRetries, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Resend timeout

    wxStaticText* itemStaticTextTCPIPResend = new wxStaticText;
    itemStaticTextTCPIPResend->Create( itemPanelCommunication, wxID_STATIC, _("Read/write timeout in milliseconds before resend :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTextTCPIPResend, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizerTCPIPResend = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizerTCPIPResend, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_SpinCtrltcpipReadTimeout = new wxSpinCtrl;
    m_SpinCtrltcpipReadTimeout->Create( itemPanelCommunication, ID_SPINCTRL1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 10000, 10000 );
    itemBoxSizerTCPIPResend->Add(m_SpinCtrltcpipReadTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // -- Total timeout

    wxStaticText* itemStaticTexttcpipTotal = new wxStaticText;
    itemStaticTexttcpipTotal->Create( itemPanelCommunication, wxID_STATIC, _("Total register read/write timeout in milliseconds :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerCommunication->Add(itemStaticTexttcpipTotal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizertcpipTotal = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerCommunication->Add(itemBoxSizertcpipTotal, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    
    m_SpinCtrltcpipTotalTimeout = new wxSpinCtrl;
    m_SpinCtrltcpipTotalTimeout->Create( itemPanelCommunication, ID_SPINCTRL1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1000, 20000, 3000 );
    itemBoxSizertcpipTotal->Add(m_SpinCtrltcpipTotalTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // ------------------------------------------------------------------------


    GetBookCtrl()->AddPage(itemPanelCommunication, _("Communication"), false, 1);


    // =============================================================================================================================


    // Colours
    wxPanel* itemPanelColours = new wxPanel;
    itemPanelColours->Create( GetBookCtrl(), ID_PANEL_COLORS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizerColours = new wxBoxSizer(wxHORIZONTAL);
    itemPanelColours->SetSizer( itemBoxSizerColours );

    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerCommunication->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    GetBookCtrl()->AddPage( itemPanelColours, _("Colours"), false, 2 );

    


    // =============================================================================================================================

    

    // VSCP Receive view
    wxPanel* itemPanelReceiveView = new wxPanel;
    itemPanelReceiveView->Create( GetBookCtrl(), ID_PANEL_RECEIVE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizerReceiveView = new wxBoxSizer(wxVERTICAL);
    itemPanelReceiveView->SetSizer(itemBoxSizerReceiveView);

    wxGridSizer* itemGridSizerReceiveView = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizerReceiveView->Add(itemGridSizerReceiveView, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
    itemGridSizerReceiveView->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerReceiveView->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerReceiveView->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ChkAutoScroll = new wxCheckBox;
    m_ChkAutoScroll->Create( itemPanelReceiveView, ID_ChkAutoScroll, _("Autoscroll"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ChkAutoScroll->SetValue(true);
    itemGridSizerReceiveView->Add(m_ChkAutoScroll, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerReceiveView->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_chkPyjamasLook = new wxCheckBox;
    m_chkPyjamasLook->Create( itemPanelReceiveView, ID_ChkPyjamasLook, _("Pyjamas Look"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkPyjamasLook->SetValue(true);
    itemGridSizerReceiveView->Add(m_chkPyjamasLook, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerReceiveView->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_chkUseSymbols = new wxCheckBox;
    m_chkUseSymbols->Create( itemPanelReceiveView, ID_CHECKBOX, _("Use symbols for class and type"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkUseSymbols->SetValue(true);
    itemGridSizerReceiveView->Add(m_chkUseSymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerReceiveView->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizerReceiveView->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText44 = new wxStaticText;
    itemStaticText44->Create( itemPanelReceiveView, wxID_STATIC, _("Text colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerReceiveView->Add(itemStaticText44, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerReceiveView->Add(itemBoxSizer45, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editRxForeGroundColour = new wxTextCtrl;
    m_editRxForeGroundColour->Create( itemPanelReceiveView, ID_EditRxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer45->Add(m_editRxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton47 = new wxButton;
    itemButton47->Create( itemPanelReceiveView, ID_BUTTON4, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer45->Add(itemButton47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText48 = new wxStaticText;
    itemStaticText48->Create( itemPanelReceiveView, wxID_STATIC, _("Background colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerReceiveView->Add(itemStaticText48, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerReceiveView->Add(itemBoxSizer49, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editRxBackGroundColour = new wxTextCtrl;
    m_editRxBackGroundColour->Create( itemPanelReceiveView, ID_EditRxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer49->Add(m_editRxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton51 = new wxButton;
    itemButton51->Create( itemPanelReceiveView, ID_BUTTON5, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer49->Add(itemButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText52 = new wxStaticText;
    itemStaticText52->Create( itemPanelReceiveView, wxID_STATIC, _("Text colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerReceiveView->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerReceiveView->Add(itemBoxSizer53, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editTxForeGroundColour = new wxTextCtrl;
    m_editTxForeGroundColour->Create( itemPanelReceiveView, ID_EditTxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer53->Add(m_editTxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton55 = new wxButton;
    itemButton55->Create( itemPanelReceiveView, ID_BUTTON6, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer53->Add(itemButton55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText56 = new wxStaticText;
    itemStaticText56->Create( itemPanelReceiveView, wxID_STATIC, _("Background colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerReceiveView->Add(itemStaticText56, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerReceiveView->Add(itemBoxSizer57, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editTxBackGroundColour = new wxTextCtrl;
    m_editTxBackGroundColour->Create( itemPanelReceiveView, ID_EditTxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer57->Add(m_editTxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton59 = new wxButton;
    itemButton59->Create( itemPanelReceiveView, ID_BUTTON7, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer57->Add(itemButton59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer60 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerReceiveView->Add(itemBoxSizer60, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer60->Add(itemBoxSizer61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton62 = new wxButton;
    itemButton62->Create( itemPanelReceiveView, ID_BUTTON, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer61->Add(itemButton62, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton63 = new wxButton;
    itemButton63->Create( itemPanelReceiveView, ID_BUTTON1, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer61->Add(itemButton63, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer60->Add(itemBoxSizer64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText65 = new wxStaticText;
    itemStaticText65->Create( itemPanelReceiveView, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer64->Add(itemStaticText65, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox66Strings;
    itemCheckListBox66Strings.Add(_("Dir"));
    itemCheckListBox66Strings.Add(_("Time"));
    itemCheckListBox66Strings.Add(_("GUID"));
    itemCheckListBox66Strings.Add(_("Class"));
    itemCheckListBox66Strings.Add(_("Type"));
    itemCheckListBox66Strings.Add(_("Head"));
    itemCheckListBox66Strings.Add(_("Data Count"));
    itemCheckListBox66Strings.Add(_("Data"));
    itemCheckListBox66Strings.Add(_("Timestamp"));
    itemCheckListBox66Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox66 = new wxCheckListBox;
    itemCheckListBox66->Create( itemPanelReceiveView, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, itemCheckListBox66Strings, wxLB_SINGLE );
    itemBoxSizer64->Add(itemCheckListBox66, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer60->Add(itemBoxSizer67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton68 = new wxButton;
    itemButton68->Create( itemPanelReceiveView, ID_BUTTON3, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer67->Add(itemButton68, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton69 = new wxButton;
    itemButton69->Create( itemPanelReceiveView, ID_BUTTON8, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer67->Add(itemButton69, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer60->Add(itemBoxSizer70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText71 = new wxStaticText;
    itemStaticText71->Create( itemPanelReceiveView, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer70->Add(itemStaticText71, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox72Strings;
    itemCheckListBox72Strings.Add(_("Dir"));
    itemCheckListBox72Strings.Add(_("Time"));
    itemCheckListBox72Strings.Add(_("GUID"));
    itemCheckListBox72Strings.Add(_("Class"));
    itemCheckListBox72Strings.Add(_("Type"));
    itemCheckListBox72Strings.Add(_("Head"));
    itemCheckListBox72Strings.Add(_("Data Count"));
    itemCheckListBox72Strings.Add(_("Data"));
    itemCheckListBox72Strings.Add(_("Timestamp"));
    itemCheckListBox72Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox72 = new wxCheckListBox;
    itemCheckListBox72->Create( itemPanelReceiveView, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, itemCheckListBox72Strings, wxLB_SINGLE );
    itemBoxSizer70->Add(itemCheckListBox72, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanelReceiveView, _("VSCP Receive View"), false, 3);


    // =============================================================================================================================


    // VSCP Tramit view
    wxPanel* itemPanelTransmitView = new wxPanel;
    itemPanelTransmitView->Create( GetBookCtrl(), ID_PANEL_TRANSMIT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizerTransmitView = new wxBoxSizer(wxVERTICAL);
    itemPanelTransmitView->SetSizer(itemBoxSizerTransmitView);

    wxGridSizer* itemGridSizerTransmitView = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizerTransmitView->Add(itemGridSizerTransmitView, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemGridSizerTransmitView->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemGridSizerTransmitView->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText76 = new wxStaticText;
    itemStaticText76->Create( itemPanelTransmitView, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerTransmitView->Add(itemStaticText76, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer77 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerTransmitView->Add(itemBoxSizer77, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl78 = new wxTextCtrl;
    itemTextCtrl78->Create( itemPanelTransmitView, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer77->Add(itemTextCtrl78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton79 = new wxButton;
    itemButton79->Create( itemPanelTransmitView, ID_BUTTON2, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer77->Add(itemButton79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText80 = new wxStaticText;
    itemStaticText80->Create( itemPanelTransmitView, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerTransmitView->Add(itemStaticText80, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer81 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerTransmitView->Add(itemBoxSizer81, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl82 = new wxTextCtrl;
    itemTextCtrl82->Create( itemPanelTransmitView, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer81->Add(itemTextCtrl82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton83 = new wxButton;
    itemButton83->Create( itemPanelTransmitView, ID_BUTTON9, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer81->Add(itemButton83, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText84 = new wxStaticText;
    itemStaticText84->Create( itemPanelTransmitView, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerTransmitView->Add(itemStaticText84, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer85 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerTransmitView->Add(itemBoxSizer85, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl86 = new wxTextCtrl;
    itemTextCtrl86->Create( itemPanelTransmitView, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer85->Add(itemTextCtrl86, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton87 = new wxButton;
    itemButton87->Create( itemPanelTransmitView, ID_BUTTON10, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer85->Add(itemButton87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText88 = new wxStaticText;
    itemStaticText88->Create( itemPanelTransmitView, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizerTransmitView->Add(itemStaticText88, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerTransmitView->Add(itemBoxSizer89, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl90 = new wxTextCtrl;
    itemTextCtrl90->Create( itemPanelTransmitView, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer89->Add(itemTextCtrl90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton91 = new wxButton;
    itemButton91->Create( itemPanelTransmitView, ID_BUTTON11, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer89->Add(itemButton91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer92 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerTransmitView->Add(itemBoxSizer92, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer93 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer92->Add(itemBoxSizer93, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton94 = new wxButton;
    itemButton94->Create( itemPanelTransmitView, ID_BUTTON12, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer93->Add(itemButton94, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton95 = new wxButton;
    itemButton95->Create( itemPanelTransmitView, ID_BUTTON13, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer93->Add(itemButton95, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer96 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer92->Add(itemBoxSizer96, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText97 = new wxStaticText;
    itemStaticText97->Create( itemPanelTransmitView, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer96->Add(itemStaticText97, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox98Strings;
    itemCheckListBox98Strings.Add(_("Dir"));
    itemCheckListBox98Strings.Add(_("Time"));
    itemCheckListBox98Strings.Add(_("GUID"));
    itemCheckListBox98Strings.Add(_("Class"));
    itemCheckListBox98Strings.Add(_("Type"));
    itemCheckListBox98Strings.Add(_("Head"));
    itemCheckListBox98Strings.Add(_("Data Count"));
    itemCheckListBox98Strings.Add(_("Data"));
    itemCheckListBox98Strings.Add(_("Timestamp"));
    itemCheckListBox98Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox98 = new wxCheckListBox;
    itemCheckListBox98->Create( itemPanelTransmitView, ID_CHECKLISTBOX2, wxDefaultPosition, wxDefaultSize, itemCheckListBox98Strings, wxLB_SINGLE );
    itemBoxSizer96->Add(itemCheckListBox98, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer99 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer92->Add(itemBoxSizer99, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton100 = new wxButton;
    itemButton100->Create( itemPanelTransmitView, ID_BUTTON14, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer99->Add(itemButton100, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton101 = new wxButton;
    itemButton101->Create( itemPanelTransmitView, ID_BUTTON15, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer99->Add(itemButton101, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer92->Add(itemBoxSizer102, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText103 = new wxStaticText;
    itemStaticText103->Create( itemPanelTransmitView, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer102->Add(itemStaticText103, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox104Strings;
    itemCheckListBox104Strings.Add(_("Dir"));
    itemCheckListBox104Strings.Add(_("Time"));
    itemCheckListBox104Strings.Add(_("GUID"));
    itemCheckListBox104Strings.Add(_("Class"));
    itemCheckListBox104Strings.Add(_("Type"));
    itemCheckListBox104Strings.Add(_("Head"));
    itemCheckListBox104Strings.Add(_("Data Count"));
    itemCheckListBox104Strings.Add(_("Data"));
    itemCheckListBox104Strings.Add(_("Timestamp"));
    itemCheckListBox104Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox104 = new wxCheckListBox;
    itemCheckListBox104->Create( itemPanelTransmitView, ID_CHECKLISTBOX3, wxDefaultPosition, wxDefaultSize, itemCheckListBox104Strings, wxLB_SINGLE );
    itemBoxSizer102->Add(itemCheckListBox104, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanelTransmitView, _("VSCP Transmission View"), false, 4);

    // Connect events and objects
    itemPanelReceiveView->Connect(ID_PANEL_RECEIVE, wxEVT_INIT_DIALOG, wxInitDialogEventHandler(dlgConfiguration::OnInitDialog), NULL, this);


    ////////////////////////////////////////////////////////////////////
    //                    Init dialog data
    ////////////////////////////////////////////////////////////////////
    if ( g_Config.m_bAutoscollRcv ) {
        m_ChkAutoScroll->SetValue( 1 );
    }
    else {
        m_ChkAutoScroll->SetValue( 0 );
    }

    // Pyjamas look
    if ( g_Config.m_VscpRcvFrameRxbPyamas ) {
        m_chkPyjamasLook->SetValue( 1 );
    }
    else {
        m_chkPyjamasLook->SetValue( 0 );
    }

    // Symbolic Names
    if ( g_Config.m_UseSymbolicNames ) {
        m_chkUseSymbols->SetValue( 1 );
    }
    else {
        m_chkUseSymbols->SetValue( 0 );
    }  

    m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameRxTextColour.Red(), 
        g_Config.m_VscpRcvFrameRxTextColour.Green(),
        g_Config.m_VscpRcvFrameRxTextColour.Blue()));

    m_editRxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameRxBgColour.Red(), 
        g_Config.m_VscpRcvFrameRxBgColour.Green(),
        g_Config.m_VscpRcvFrameRxBgColour.Blue()));

    m_editTxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameTxTextColour.Red(), 
        g_Config.m_VscpRcvFrameTxTextColour.Green(),
        g_Config.m_VscpRcvFrameTxTextColour.Blue()));

    m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameTxBgColour.Red(), 
        g_Config.m_VscpRcvFrameTxBgColour.Green(),
        g_Config.m_VscpRcvFrameTxBgColour.Blue()));

    // CANAL Communication parameters
	m_SpinCtrlmaxRetries->SetValue( g_Config.m_CANALRegMaxRetries );
	m_SpinCtrlreadTimeout->SetValue( g_Config.m_CANALRegResendTimeout );
    m_SpinCtrltotalTimeout->SetValue( g_Config.m_CANALRegErrorTimeout );

    // TCPIP communication parameters
    m_SpinCtrlResponseTimeout->SetValue( g_Config.m_TCPIP_ResponseTimeout );
    m_SpinCtrlmaxTCPIPRetries->SetValue( g_Config.m_TCPIPRegMaxRetries );
	m_SpinCtrltcpipReadTimeout->SetValue( g_Config.m_TCPIPRegResendTimeout );
    m_SpinCtrltcpipTotalTimeout->SetValue( g_Config.m_TCPIPRegErrorTimeout );

	m_labelLogFile->SetLabel( g_Config.m_strPathLogFile );
	m_checkEnableLogging->SetValue( g_Config.m_bEnableLog );
	m_comboLogLevel->Select( g_Config.m_logLevel );

	m_checkConfirmDeletes->SetValue( g_Config.m_bConfirmDelete );
    m_checkManufacturerInfo->SetValue( g_Config.bGuidWritable );

	m_comboNumericalBase->Select( g_Config.m_Numberbase );

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getDialogData
//

bool dlgConfiguration::getDialogData( bool bWriteToConfigFile )
{
    g_Config.m_bAutoscollRcv = m_ChkAutoScroll->GetValue();
    g_Config.m_VscpRcvFrameRxbPyamas = m_chkPyjamasLook->GetValue();	
    g_Config.m_UseSymbolicNames = m_chkUseSymbols->GetValue();
	
    g_Config.m_CANALRegMaxRetries = m_SpinCtrlmaxRetries->GetValue();
    g_Config.m_CANALRegResendTimeout = m_SpinCtrlreadTimeout->GetValue();
	g_Config.m_CANALRegErrorTimeout = m_SpinCtrltotalTimeout->GetValue();

    g_Config.m_TCPIP_ResponseTimeout = m_SpinCtrlResponseTimeout->GetValue();
    g_Config.m_TCPIPRegMaxRetries = m_SpinCtrlmaxTCPIPRetries->GetValue();
    g_Config.m_TCPIPRegResendTimeout = m_SpinCtrltcpipReadTimeout->GetValue();
    g_Config.m_TCPIPRegErrorTimeout = m_SpinCtrltcpipTotalTimeout->GetValue();

	g_Config.m_logLevel = m_comboLogLevel->GetSelection();
	g_Config.m_bEnableLog = m_checkEnableLogging->GetValue();
	g_Config.m_bConfirmDelete = m_checkConfirmDeletes->GetValue();
    g_Config.bGuidWritable = m_checkManufacturerInfo->GetValue();
    g_Config.m_Numberbase = m_comboNumericalBase->GetSelection();


    if ( bWriteToConfigFile ) {
        wxGetApp().writeConfiguration();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgConfiguration::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgConfiguration::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgConfiguration::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if (name == wxT("fatbee_v2.ico")) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }
    else if (name == wxT("copy.xpm")) {
        wxIcon icon(copy_xpm);
        return icon;
    }

    return wxNullIcon;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonSetRxGridTextColourClick
//

void dlgConfiguration::OnButtonSetRxGridTextColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameRxTextColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameRxTextColour = dlg.GetColourData().GetColour();
        m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SetRxGridBackgroundColourClick
//

void dlgConfiguration::SetRxGridBackgroundColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameRxBgColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameRxBgColour = dlg.GetColourData().GetColour();
        m_editRxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip();  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonSetTxGridTextColourClick
//

void dlgConfiguration::OnButtonSetTxGridTextColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameTxTextColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameTxTextColour = dlg.GetColourData().GetColour();
        m_editTxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonSetTxGridBackgroundColourClick
//

void dlgConfiguration::OnButtonSetTxGridBackgroundColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameTxBgColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameTxBgColour = dlg.GetColourData().GetColour();
        m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnInitDialog
//

void dlgConfiguration::OnInitDialog( wxInitDialogEvent& event )
{
    if ( g_Config.m_bAutoscollRcv ) {
        m_ChkAutoScroll->SetValue( 1 );
    }
    else {
        m_ChkAutoScroll->SetValue( 0 );
    }
    event.Skip();
}

