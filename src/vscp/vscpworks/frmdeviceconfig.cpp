/////////////////////////////////////////////////////////////////////////////
//  Name:        frmdeviceconfig.cpp
//  Purpose:     
//  Author:      Ake Hedman
//  Modified by: 
//  Created:     Sun 04 May 2007 17:28:12 CEST
//  RCS-ID:      
//  Copyright:   (C) 2007-2015 
//  Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//  Licence:     
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version
//  2 of the License, or (at your option) any later version.
// 
//  This file is part of the VSCP (http://www.vscp.org) 
// 
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this file see the file COPYING.  If not, write to
//  the Free Software Foundation, 59 Temple Place - Suite 330,
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
#pragma implementation "frmdeviceconfig.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/imaglist.h>
#include <wx/url.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/html/htmlwin.h>
#include <wx/html/htmlcell.h>

#include "vscpworks.h"
#include <canal.h>
#include <vscp.h>
#include <vscphelper.h>
#include <mdf.h>
#include "dlgabout.h"
#include "dlgeditlevel1dmrow.h"
#include "dlgabstractionedit.h"
#include "readregister.h"
#include "frmdeviceconfig_images.h"
#include "frmdeviceconfig.h"

extern appConfiguration g_Config;


DEFINE_EVENT_TYPE(wxVSCP_STATUS_CHANGE_EVENT)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig type definition
//

IMPLEMENT_CLASS(frmDeviceConfig, wxFrame)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig event table definition
//

BEGIN_EVENT_TABLE(frmDeviceConfig, wxFrame)

    EVT_CLOSE( frmDeviceConfig::OnCloseWindow )
    EVT_SIZE( frmDeviceConfig::OnResizeWindow )
    EVT_MENU( ID_MENUITEM_SAVE_REGSITERS, frmDeviceConfig::OnMenuitemSaveRegistersClick )
    EVT_MENU( ID_MENUITEM, frmDeviceConfig::OnMenuitemSaveSelectedRegistersClick )
    EVT_MENU( ID_MENUITEM_LOAD_REGISTES, frmDeviceConfig::OnMenuitemLoadRegistersClick )
    EVT_MENU( ID_MENUITEM_ADD_GUIDS, frmDeviceConfig::OnMenuitemAddGuidsClick )
    EVT_MENU( ID_MENUITEM_SAVE_GUIDS, frmDeviceConfig::OnMenuitemSaveGuidsClick )
    EVT_MENU( ID_MENUITEM_LOAD_GUIDS, frmDeviceConfig::OnMenuitemLoadGuidsClick )
    EVT_MENU( ID_MENUITEM_EXIT, frmDeviceConfig::OnMenuitemExitClick )
    EVT_MENU( ID_MENUITEM_HELP, frmDeviceConfig::OnMenuitemVscpHelpClick )
    EVT_MENU( ID_MENUITEM_FAQ, frmDeviceConfig::OnMenuitemVscpFaqClick )
    EVT_MENU( ID_MENUITEM_KB_SHRTCUTS, frmDeviceConfig::OnMenuitemVscpShortcutsClick )
    EVT_MENU( ID_MENUITEM_THANKS, frmDeviceConfig::OnMenuitemVscpThanksClick )
    EVT_MENU( ID_MENUITEM_CREDITS, frmDeviceConfig::OnMenuitemVscpCreditsClick )
    EVT_MENU( ID_MENUITEM_GO_VSCP_SITE, frmDeviceConfig::OnMenuitemVscpVscpSiteClick )
    EVT_MENU( ID_MENUITEM_ABOUT, frmDeviceConfig::OnMenuitemVscpAboutClick )
    EVT_COMBOBOX( ID_COMBOBOX4, frmDeviceConfig::OnComboNodeIDSelected )
    EVT_TEXT( ID_COMBOBOX4, frmDeviceConfig::OnComboNodeIDUpdated )
    EVT_BUTTON( ID_CHECK_LEVEL2, frmDeviceConfig::OnBitmapbuttonTestDeviceClick )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON1, frmDeviceConfig::OnInterfaceActivate )
    EVT_GRID_CELL_LEFT_CLICK( frmDeviceConfig::OnCellLeftClick )
    EVT_GRID_CELL_RIGHT_CLICK( frmDeviceConfig::OnCellRightClick )
    EVT_GRID_CELL_LEFT_DCLICK( frmDeviceConfig::OnLeftDClick )
    EVT_GRID_CELL_CHANGE( frmDeviceConfig::OnRegisterEdited )
    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW1, frmDeviceConfig::OnHtmlwindowCellClicked )
    EVT_BUTTON( ID_BUTTON16, frmDeviceConfig::OnButtonUpdateClick )
    EVT_BUTTON( ID_BUTTON17, frmDeviceConfig::OnButtonLoadDefaultsClick )
    EVT_BUTTON( ID_BUTTON19, frmDeviceConfig::OnButtonWizardClick )

    EVT_MENU(Menu_Popup_Read_Value, frmDeviceConfig::readValueSelectedRow)
    EVT_MENU(Menu_Popup_Write_Value, frmDeviceConfig::writeValueSelectedRow)
    EVT_MENU(Menu_Popup_Update, frmDeviceConfig::OnButtonUpdateClick)
    EVT_MENU(Menu_Popup_Load_MDF, frmDeviceConfig::OnButtonLoadDefaultsClick)
    EVT_MENU(Menu_Popup_Undo, frmDeviceConfig::undoValueSelectedRow)
    EVT_MENU(Menu_Popup_Default, frmDeviceConfig::defaultValueSelectedRow)
    EVT_MENU(Menu_Popup_go_VSCP, frmDeviceConfig::OnMenuitemVscpVscpSiteClick)

    EVT_MENU(Menu_Popup_dm_enable_row, frmDeviceConfig::dmEnableSelectedRow)
    EVT_MENU(Menu_Popup_dm_disable_row, frmDeviceConfig::dmDisableSelectedRow)
    EVT_MENU(Menu_Popup_dm_row_wizard, frmDeviceConfig::dmRowWizard)

    EVT_COMMAND(ID_FRMDEVICECONFIG, wxVSCP_STATUS_CHANGE_EVENT, frmDeviceConfig::eventStatusChange )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig constructors
//

frmDeviceConfig::frmDeviceConfig() {
    Init();
}

frmDeviceConfig::frmDeviceConfig( wxWindow* parent, 
                                        wxWindowID id, 
                                        const wxString& caption, 
                                        const wxPoint& pos, 
                                        const wxSize& size, 
                                        long style) 
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig creator
//

bool frmDeviceConfig::Create( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style) 
{
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig destructor
//

frmDeviceConfig::~frmDeviceConfig() 
{
    ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmDeviceConfig::Init() 
{
    m_comboNodeID = NULL;
    m_bLevel2 = NULL;
    m_BtnActivateInterface = NULL;
    m_choiceBook = NULL;
    m_panel0 = NULL;
    m_gridRegisters = NULL;
    m_gridAbstractions = NULL;
    m_gridDM = NULL;
    m_StatusWnd = NULL;
    m_chkFullUpdate = NULL;
    m_chkMdfFromFile = NULL;
    m_ctrlButtonLoadMDF = NULL;
    m_ctrlButtonWizard = NULL;

    m_lastLeftClickCol = 0;
    m_lastLeftClickRow = 0;

    // No interface
    m_ifguid.clear();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CreateControls
// 
// Control creation for frmDeviceConfig
//

void frmDeviceConfig::CreateControls() {

    frmDeviceConfig* itemFrame1 = this;

    // Statusbar
    m_pitemStatusBar = new wxStatusBar;
    m_pitemStatusBar->Create( itemFrame1, 
                                ID_STATUSBAR, 
                                wxST_SIZEGRIP | wxNO_BORDER );
    m_pitemStatusBar->SetFieldsCount(2);
    itemFrame1->SetStatusBar( m_pitemStatusBar );

    wxMenuBar* menuBar = new wxMenuBar;
    
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(ID_MENUITEM_SAVE_REGSITERS, _("Save registers to file.."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM, _("Save selected registers to file.."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM_LOAD_REGISTES, _("Load Registers from file..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(ID_MENUITEM_ADD_GUIDS, _("Add GUID..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM_SAVE_GUIDS, _("Save GUID's..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MENUITEM_LOAD_GUIDS, _("Load GUID's..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(ID_MENUITEM_EXIT, _("Exit"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    
    wxMenu* itemMenu13 = new wxMenu;
    menuBar->Append(itemMenu13, _("Edit"));
    
    wxMenu* itemMenu14 = new wxMenu;
    itemMenu14->Append(ID_MENUITEM_HELP, _("VSCP-Works Help"), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->Append(ID_MENUITEM_FAQ, _("Frequently Asked Questions"), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->Append(ID_MENUITEM_KB_SHRTCUTS, _("Keyboard shortcuts"), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->AppendSeparator();
    itemMenu14->Append(ID_MENUITEM_THANKS, _("Thanks..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->Append(ID_MENUITEM_CREDITS, _("Credits..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->AppendSeparator();
    itemMenu14->Append(ID_MENUITEM_GO_VSCP_SITE, _("Go to VSCP site"), wxEmptyString, wxITEM_NORMAL);
    itemMenu14->AppendSeparator();
    itemMenu14->Append(ID_MENUITEM_ABOUT, _("About"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu14, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    wxToolBar* itemToolBar25 = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR_DEVICE_CONFIG );
    wxBitmap itemtool26Bitmap(itemFrame1->GetBitmapResource(wxT("open.xpm")));
    wxBitmap itemtool26BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL6, wxEmptyString, itemtool26Bitmap, itemtool26BitmapDisabled, wxITEM_NORMAL, _("Fetch data from file"), wxEmptyString);
    wxBitmap itemtool27Bitmap(itemFrame1->GetBitmapResource(wxT("save.xpm")));
    wxBitmap itemtool27BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL7, wxEmptyString, itemtool27Bitmap, itemtool27BitmapDisabled, wxITEM_NORMAL, _("Save data to file"), wxEmptyString);
    itemToolBar25->AddSeparator();
    wxBitmap itemtool29Bitmap(itemFrame1->GetBitmapResource(wxT("cut.xpm")));
    wxBitmap itemtool29BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL8, wxEmptyString, itemtool29Bitmap, itemtool29BitmapDisabled, wxITEM_NORMAL, _("Remove selected row(s)"), wxEmptyString);
    wxBitmap itemtool30Bitmap(itemFrame1->GetBitmapResource(wxT("copy.xpm")));
    wxBitmap itemtool30BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL9, wxEmptyString, itemtool30Bitmap, itemtool30BitmapDisabled, wxITEM_NORMAL, _("Copy selected row(s) \nto the clipboard"), wxEmptyString);
    wxBitmap itemtool31Bitmap(itemFrame1->GetBitmapResource(wxT("paste.xpm")));
    wxBitmap itemtool31BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL10, wxEmptyString, itemtool31Bitmap, itemtool31BitmapDisabled, wxITEM_NORMAL, _("Paste row(s) from clipboard"), wxEmptyString);
    itemToolBar25->AddSeparator();
    wxBitmap itemtool33Bitmap(itemFrame1->GetBitmapResource(wxT("Print.xpm")));
    wxBitmap itemtool33BitmapDisabled;
    itemToolBar25->AddTool(ID_TOOL11, wxEmptyString, itemtool33Bitmap, itemtool33BitmapDisabled, wxITEM_NORMAL, _("Print selected or all row(s)"), wxEmptyString);
    itemToolBar25->AddSeparator();
    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemToolBar25, wxID_STATIC, _("Node id: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText35->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
    itemToolBar25->AddControl(itemStaticText35);
    wxArrayString m_comboNodeIDStrings;
    
    // Nodeid/guid combobox
    m_comboNodeID = new wxComboBox;
    m_comboNodeID->Create( itemToolBar25, 
                                ID_COMBOBOX4, 
                                wxEmptyString, 
                                wxDefaultPosition, 
                                wxSize(410, -1), 
                                m_comboNodeIDStrings, 
                                wxCB_DROPDOWN );
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_comboNodeID->SetToolTip( _("Set nickname or GUID for node here") );
    }
    m_comboNodeID->SetBackgroundColour(wxColour(255, 255, 210));
    itemToolBar25->AddControl(m_comboNodeID);

    // Button for connection test
    wxBitmapButton* itemBitmapButton37 = new wxBitmapButton;
    itemBitmapButton37->Create( itemToolBar25, ID_CHECK_LEVEL2, itemFrame1->GetBitmapResource(wxT("find.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    if ( frmDeviceConfig::ShowToolTips() ) {
        itemBitmapButton37->SetToolTip( _("Test if device is present.") );
    }
    itemToolBar25->AddControl(itemBitmapButton37);

    itemToolBar25->AddSeparator();
    
    // Level II Checkbox
    m_bLevel2 = new wxCheckBox;
    m_bLevel2->Create( itemToolBar25, ID_CHECKBOX_LEVEL22, _("Level II"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bLevel2->SetValue(false);
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_bLevel2->SetToolTip( _("Mark this interface as a full Level II interface") );
    }

    itemToolBar25->AddControl(m_bLevel2);
    itemToolBar25->AddSeparator();

    m_BtnActivateInterface = new wxToggleButton;
    m_BtnActivateInterface->Create( itemToolBar25, ID_TOGGLEBUTTON1, _("Connected"), wxDefaultPosition, wxSize(120, -1), 0 );
    m_BtnActivateInterface->SetValue(true);
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_BtnActivateInterface->SetToolTip( _("Acticate/Deactivate the interface") );
    }
    //m_BtnActivateInterface->SetForegroundColour(wxColour(255, 255, 255));
    //m_BtnActivateInterface->SetBackgroundColour(wxColour(165, 42, 42));
    //m_BtnActivateInterface->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));

    itemToolBar25->AddControl(m_BtnActivateInterface);
    itemToolBar25->Realize();
    itemFrame1->SetToolBar(itemToolBar25);

    wxPanel* itemPanel44 = new wxPanel;
    itemPanel44->Create( itemFrame1, ID_PANEL_DEVICE_CONFIG, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
    itemPanel44->SetSizer(itemBoxSizer45);

    wxBoxSizer* itemBoxSizer46 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer45->Add(itemBoxSizer46, 0, wxGROW|wxALL, 5);

    m_choiceBook = new wxToolbook;
    m_choiceBook->Create( itemPanel44, ID_CHOICEBOOK, wxDefaultPosition, wxSize(600, 400), wxBK_DEFAULT );
    wxImageList* m_choiceBookImageList = new wxImageList(32, 32, true, 3);
    {
        wxIcon m_choiceBookIcon0(itemFrame1->GetIconResource(wxT("icons/png/32x32/database_process.png")));
        m_choiceBookImageList->Add(m_choiceBookIcon0);
        wxIcon m_choiceBookIcon1(itemFrame1->GetIconResource(wxT("icons/png/32x32/database_accept.png")));
        m_choiceBookImageList->Add(m_choiceBookIcon1);
        wxIcon m_choiceBookIcon2(itemFrame1->GetIconResource(wxT("icons/png/32x32/database_down.png")));
        m_choiceBookImageList->Add(m_choiceBookIcon2);
    }
    m_choiceBook->AssignImageList(m_choiceBookImageList);

    m_panel0 = new wxPanel;
    m_panel0->Create( m_choiceBook, ID_PANEL_REGISTERS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panel0->Show(false);
    wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxVERTICAL);
    m_panel0->SetSizer(itemBoxSizer49);

    m_gridRegisters = new wxGrid;
    m_gridRegisters->Create( m_panel0, ID_GRID_REGISTERS, wxDefaultPosition, wxSize(400, 340), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_gridRegisters->SetBackgroundColour(wxColour(240, 240, 240));
    m_gridRegisters->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
    m_gridRegisters->SetDefaultColSize(50);
    m_gridRegisters->SetDefaultRowSize(18);
    m_gridRegisters->SetColLabelSize(18);
    m_gridRegisters->SetRowLabelSize(40);
    m_gridRegisters->CreateGrid(1, 4, wxGrid::wxGridSelectRows);
    itemBoxSizer49->Add(m_gridRegisters, 0, wxGROW|wxALL, 5);

    m_choiceBook->AddPage(m_panel0, _("Registers"), false, 0);

    wxPanel* itemPanel51 = new wxPanel;
    itemPanel51->Create( m_choiceBook, ID_PANEL_ABSTRACTIONS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemPanel51->Show(false);
    wxBoxSizer* itemBoxSizer52 = new wxBoxSizer(wxVERTICAL);
    itemPanel51->SetSizer(itemBoxSizer52);

    m_gridAbstractions = new wxGrid;
    m_gridAbstractions->Create( itemPanel51, ID_GRID_ABSTRACTIONS, wxDefaultPosition, wxSize(400, 340), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_gridAbstractions->SetBackgroundColour(wxColour(240, 240, 240));
    m_gridAbstractions->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
    m_gridAbstractions->SetDefaultColSize(50);
    m_gridAbstractions->SetDefaultRowSize(18);
    m_gridAbstractions->SetColLabelSize(18);
    m_gridAbstractions->SetRowLabelSize(40);
    m_gridAbstractions->CreateGrid(1, 5, wxGrid::wxGridSelectRows);
    itemBoxSizer52->Add(m_gridAbstractions, 0, wxGROW|wxALL, 5);

    m_choiceBook->AddPage(itemPanel51, _("Abstraction"), false, 1);

    wxPanel* itemPanel54 = new wxPanel;
    itemPanel54->Create( m_choiceBook, ID_PANEL_DM, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemPanel54->Show(false);
    wxBoxSizer* itemBoxSizer55 = new wxBoxSizer(wxVERTICAL);
    itemPanel54->SetSizer(itemBoxSizer55);

    m_gridDM = new wxGrid;
    m_gridDM->Create( itemPanel54, ID_GRID_DM, wxDefaultPosition, wxSize(400, 340), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_gridDM->SetBackgroundColour(wxColour(240, 240, 240));
    m_gridDM->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
    m_gridDM->SetDefaultColSize(50);
    m_gridDM->SetDefaultRowSize(18);
    m_gridDM->SetColLabelSize(18);
    m_gridDM->SetRowLabelSize(40);
    m_gridDM->CreateGrid(1, 8, wxGrid::wxGridSelectRows);
    itemBoxSizer55->Add(m_gridDM, 0, wxGROW|wxALL, 5);

    m_choiceBook->AddPage(itemPanel54, _("Decision Matrix"), false, 2);

    itemBoxSizer46->Add(m_choiceBook, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer45->Add(itemBoxSizer57, 0, wxALIGN_RIGHT|wxALL, 0);

    wxBoxSizer* itemBoxSizer58 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer57->Add(itemBoxSizer58, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer59 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer58->Add(itemBoxSizer59, 0, wxALIGN_TOP|wxALL, 5);

    itemBoxSizer59->Add(15, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer * itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer58->Add(itemBoxSizer61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StatusWnd = new wxHtmlWindow;
    m_StatusWnd->Create( itemPanel44, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(680, 180), wxHW_SCROLLBAR_AUTO|wxRAISED_BORDER|wxWANTS_CHARS|wxHSCROLL|wxVSCROLL );
    itemBoxSizer61->Add(m_StatusWnd, 0, wxALIGN_RIGHT|wxALL, 0);

    wxBoxSizer* itemBoxSizer63 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer58->Add(itemBoxSizer63, 0, wxALIGN_TOP|wxALL, 5);

    m_chkFullUpdate = new wxCheckBox;
    m_chkFullUpdate->Create( itemPanel44, ID_CHECKBOX_FULL_UPDATE, _("Full Update"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkFullUpdate->SetValue(false);
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_chkFullUpdate->SetToolTip( _("All registers will be read from device again if checked when update is pressed.") );
    }

    itemBoxSizer63->Add(m_chkFullUpdate, 0, wxALIGN_LEFT|wxALL, 5);

    m_chkMdfFromFile = new wxCheckBox;
    m_chkMdfFromFile->Create( itemPanel44, ID_CHECKBOX_MDF_FROM_FILE, _("Use local MDF"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkMdfFromFile->SetValue(false);
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_chkMdfFromFile->SetToolTip( _("Load the MDF from disk instead from a remote location.") );
    }

    itemBoxSizer63->Add(m_chkMdfFromFile, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton66 = new wxButton;
    itemButton66->Create( itemPanel44, ID_BUTTON16, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton66->SetDefault();
    if ( frmDeviceConfig::ShowToolTips() ) {
        itemButton66->SetToolTip( _("Read registers from device (first time)/Write changed registers (red) to device.") );
    }

    itemBoxSizer63->Add(itemButton66, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    m_ctrlButtonLoadMDF = new wxButton;
    m_ctrlButtonLoadMDF->Create( itemPanel44, ID_BUTTON17, _("Load defaults"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlButtonLoadMDF->Enable(false);
    if ( frmDeviceConfig::ShowToolTips() ) {
        m_ctrlButtonLoadMDF->SetToolTip( _("Write default values to all registers.") );
    }

    itemBoxSizer63->Add(m_ctrlButtonLoadMDF, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemBoxSizer63->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ctrlButtonWizard = new wxButton;
    m_ctrlButtonWizard->Create( itemPanel44, ID_BUTTON19, _("Wizard"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlButtonWizard->Enable(false);
    itemBoxSizer63->Add(m_ctrlButtonWizard, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    // Register Page 0
    m_gridRegisters->SetColSize(0, m_constGridRegisterDefaultWidth);
    m_gridRegisters->SetColLabelValue(0, _("Register"));

    m_gridRegisters->SetColSize(1, m_constGridAccessRightDefaultWidth);
    m_gridRegisters->SetColLabelValue(1, _(" "));

    m_gridRegisters->SetColSize(2, m_constGridContentdefaultWidth);
    m_gridRegisters->SetColLabelValue(2, _("Content"));

    m_gridRegisters->SetColSize(3, m_constGridDescriptionDefaultWidth);
    m_gridRegisters->SetColLabelValue(3, _("Description"));

    m_gridRegisters->DeleteRows(0);


    // Register Page Abstraction
    m_gridAbstractions->SetColSize(0, m_constGridAbstractionNameDefaultWidth);
    m_gridAbstractions->SetColLabelValue(0, _("Name"));

    m_gridAbstractions->SetColSize(1, m_constGridAbstractionTypeDefaultWidth);
    m_gridAbstractions->SetColLabelValue(1, _("Type"));

    m_gridAbstractions->SetColSize(2, m_constGridAbstractionAccessDefaultWidth);
    m_gridAbstractions->SetColLabelValue(2, _("Access"));

    m_gridAbstractions->SetColSize(3, m_constGridAbstractionContentdefaultWidth);
    m_gridAbstractions->SetColLabelValue(3, _("Content"));

    m_gridAbstractions->SetColSize(4, m_constGridAbstractionDescriptionDefaultWidth);
    m_gridAbstractions->SetColLabelValue(4, _("Description"));

    m_gridAbstractions->DeleteRows(0);

    // Register Page Decsion Matrix
    m_gridDM->SetColSize(0, m_constGridDMOrigAddressDefaultWidth);
    m_gridDM->SetColLabelValue(0, _("O-addr."));

    m_gridDM->SetColSize(1, m_constGridDMFlagsDefaultWidth);
    m_gridDM->SetColLabelValue(1, _("Flags"));

    m_gridDM->SetColSize(2, m_constGridDMMasksDefaultWidth);
    m_gridDM->SetColLabelValue(2, _("Class Mask"));

    m_gridDM->SetColSize(3, m_constGridDMMasksDefaultWidth);
    m_gridDM->SetColLabelValue(3, _("Class Filter"));

    m_gridDM->SetColSize(4, m_constGridDMMasksDefaultWidth);
    m_gridDM->SetColLabelValue(4, _("Type Mask"));

    m_gridDM->SetColSize(5, m_constGridDMMasksDefaultWidth);
    m_gridDM->SetColLabelValue(5, _("Type Filter"));

    m_gridDM->SetColSize(6, m_constGridDMActionDefaultWidth);
    m_gridDM->SetColLabelValue(6, _("Action"));

    m_gridDM->SetColSize(7, m_constGridDMActionParamDefaultWidth);
    m_gridDM->SetColLabelValue(7, _("Param."));

    m_gridDM->DeleteRows(0);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmDeviceConfig::OnCloseWindow(wxCloseEvent& event) 
{
    wxBusyCursor wait;

    m_pitemStatusBar->SetStatusText( _("Closing window..."), STATUSBAR_STATUS_LEFT );

    // Close the communication channel
    disableInterface();

    // Hide window
    Show(false);

    // Destroy the window
    Destroy();

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enableInterface
//

bool frmDeviceConfig::enableInterface(void) 
{
    bool rv = true;

    m_pitemStatusBar->SetStatusText( _("Opening interface..."), STATUSBAR_STATUS_LEFT );

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Opening interface..."),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL);

    progressDlg.Pulse(_("opening interface..."));

    if (0 != m_csw.doCmdOpen()) {

        m_pitemStatusBar->SetStatusText( _("Interface is open."), STATUSBAR_STATUS_LEFT );
        progressDlg.Pulse(_("Interface is open."));

        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

            progressDlg.Pulse(_("Checking if interface is working..."));

            // We use a dll.
            //
            // Verify that we have a connection with the interface
            // We do that by using the status command.
            canalStatus canalstatus;
            if (CANAL_ERROR_SUCCESS != m_csw.doCmdStatus(&canalstatus)) {
                wxString strbuf;
                strbuf.Printf(
                        _("Unable to open interface to driver. errorcode=%lu, suberrorcode=%lu, Description: %s"),
                        canalstatus.lasterrorcode,
                        canalstatus.lasterrorsubcode,
                        canalstatus.lasterrorstr);
                wxLogStatus(strbuf);

                wxMessageBox(_("No response received from interface. May not work correctly!"));
                rv = false;
                goto error;
            }

        } 
        else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

            progressDlg.Pulse(_("Checking if interface is working..."));

            // Test
            if (CANAL_ERROR_SUCCESS != m_csw.doCmdNOOP()) {
                wxMessageBox(_("Interface test command failed. May not work correctly!"));
                rv = false;
                goto error;
            }

            // TCP/IP interface
            progressDlg.Pulse(_("TCP/IP Interface Open."));

            // Should we fetch interface GUID
            if (m_vscpif.m_strInterfaceName.Length()) {

                progressDlg.Pulse(_("Fetching interface GUID."));

                if ( fetchIterfaceGUID() ) {

                    progressDlg.Pulse(_("Interface GUID found."));

                    // Fill the combo
                    wxString str;
                    cguid guid = m_ifguid;
                    wxArrayString strings;
                    for (int i = 1; i < 256; i++) {
                        guid.setLSB(i);
                        guid.toString(str);
                        strings.Add( str );
                    }
                    m_comboNodeID->Append(strings);

                    guid.setLSB(1);
                    guid.toString(str);
                    m_comboNodeID->SetValue(str);

                } 
                else {
                    m_comboNodeID->SetValue(_("Enter full GUID of remote node here"));
                }

            } 
            else {

                // No interface selected
                m_comboNodeID->SetValue(_("Enter full GUID of remote node here"));

            }

        }
    } 
    else {
        progressDlg.Pulse(_("Failed to open Interface."));
        wxMessageBox(_("Failed to Open Interface."));
         m_pitemStatusBar->SetStatusText( _("Failed to Open Interface."), STATUSBAR_STATUS_LEFT );

        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
        return false;
    }

    // Move to top of z-order
    Raise();

    // Needed on Windows to make the panels size correctly
    int width;
    int height;
    GetSize(&width, &height);
    SetSize(width + 10, height);

error:

    return rv;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disableInterface
//

bool frmDeviceConfig::disableInterface(void) 
{
    m_pitemStatusBar->SetStatusText( _("Interface is closed."), STATUSBAR_STATUS_LEFT );
    m_csw.doCmdClose();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemExitClick
//

void frmDeviceConfig::OnMenuitemExitClick(wxCommandEvent& event) 
{
    Close();
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpHelpClick
//

void frmDeviceConfig::OnMenuitemVscpHelpClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpFaqClick
//

void frmDeviceConfig::OnMenuitemVscpFaqClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpShortcutsClick
//

void frmDeviceConfig::OnMenuitemVscpShortcutsClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpThanksClick
//

void frmDeviceConfig::OnMenuitemVscpThanksClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpCreditsClick
//

void frmDeviceConfig::OnMenuitemVscpCreditsClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpVscpSiteClick
//

void frmDeviceConfig::OnMenuitemVscpVscpSiteClick(wxCommandEvent& event) 
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpAboutClick
//

void frmDeviceConfig::OnMenuitemVscpAboutClick(wxCommandEvent& event) 
{
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnBitmapbuttonTestDeviceClick
//

void frmDeviceConfig::OnBitmapbuttonTestDeviceClick(wxCommandEvent& event) 
{    
    unsigned char nodeid;
    ::wxBeginBusyCursor();

    m_pitemStatusBar->SetStatusText( _("Testing if device is present"), STATUSBAR_STATUS_RIGHT ); 

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());

        if ((0 == nodeid) || (nodeid > 254)) {
            wxMessageBox(_("Invalid Node ID! Must be between 1-254"));
            ::wxEndBusyCursor();
            return;
        }

        unsigned char val;
        if ( CANAL_ERROR_SUCCESS == m_csw.getDllInterface()->readLevel1Register( nodeid, 0, 0xd0, &val ) ) {
            wxMessageBox(_("Device found!"));
        } 
        else {
            wxMessageBox(_("Device was not found! Check nodeid."));
        }

    } 
    else if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {

        // Get the destination GUID
        cguid destGUID;
        destGUID.getFromString(m_comboNodeID->GetValue());

        unsigned char val;
        if ( VSCP_ERROR_SUCCESS == 
             m_csw.getTcpIpInterface()->readLevel2Register( m_bLevel2->GetValue() ? 0xffffffd0 : 0xd0,
                                            0,          // page
                                            &val,
                                            m_ifguid,
                                            &destGUID,                                            
                                            m_bLevel2->GetValue())) {
            wxMessageBox(_("Device found!"));
        } 
        else {
            wxMessageBox(_("Device was not found! Check interface GUID + nodeid."));
        }

    }

    m_pitemStatusBar->SetStatusText( _(""), STATUSBAR_STATUS_RIGHT ); 

    ::wxEndBusyCursor();
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnInterfaceActivate
//

void frmDeviceConfig::OnInterfaceActivate(wxCommandEvent& event) 
{
    wxBusyCursor wait;

    if (!m_BtnActivateInterface->GetValue()) {
        disableInterface();
        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
    } 
    else {
        if (enableInterface()) {
            m_BtnActivateInterface->SetValue(true);
            m_BtnActivateInterface->SetLabel(_("Connected"));
        }
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnRegisterEdited
//

void frmDeviceConfig::OnRegisterEdited(wxGridEvent& event) 
{
    uint8_t val;
    wxString strBuf;
    wxString str;

    if (2 == event.GetCol()) {
        str = m_gridRegisters->GetCellValue(event.GetRow(), event.GetCol());
        val = vscp_readStringValue(str);
        strBuf.Printf(_("0x%02x"), val);
        m_gridRegisters->SetCellValue(event.GetRow(), event.GetCol(), strBuf);

        m_gridRegisters->SetCellTextColour(event.GetRow(),
                event.GetCol(),
                *wxRED);
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCellLeftClick
//

void frmDeviceConfig::OnCellLeftClick(wxGridEvent& event) 
{
    // Save clicked cell/row
    m_lastLeftClickCol = event.GetCol();
    m_lastLeftClickRow = event.GetRow();

    if (ID_GRID_REGISTERS == event.GetId()) {
        // Select the row
        m_gridRegisters->SelectRow(m_lastLeftClickRow);
    } 
    else if (ID_GRID_ABSTRACTIONS == event.GetId()) {
        // Select the row
        m_gridAbstractions->SelectRow(m_lastLeftClickRow);
    } 
    else if (ID_GRID_DM == event.GetId()) {
        // Select the row
        m_gridDM->SelectRow(m_lastLeftClickRow);
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmDeviceConfig::ShowToolTips() 
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmDeviceConfig::GetBitmapResource(const wxString& name) 
{
    // Bitmap retrieval
    wxUnusedVar(name);
    if ( name == _T("open.xpm") ) {
        wxBitmap bitmap(open_xpm);
        return bitmap;
    }
    else if ( name == _T("save.xpm") ) {
        wxBitmap bitmap(save_xpm);
        return bitmap;
    }
    else if ( name == _T("cut.xpm") ) {
        wxBitmap bitmap(cut_xpm);
        return bitmap;
    }
    else if ( name == _T("copy.xpm") ) {
        wxBitmap bitmap(copy_xpm);
        return bitmap;
    }
    else if (name == _T("paste.xpm") ) {
        wxBitmap bitmap(paste_xpm);
        return bitmap;
    }
    else if ( name == _T("Print.xpm") ) {
        wxBitmap bitmap(print_xpm);
        return bitmap;
    }
    else if ( name == _T("find.xpm") ) {
        wxBitmap bitmap(find_xpm);
        return bitmap;
    }

    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmDeviceConfig::GetIconResource(const wxString& name) 
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == _T("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }
    else if ( name == _T("icons/png/32x32/database_process.png" ) ) {
        wxIcon icon(database_process_xpm);
        return icon;
    }
    else if ( name == _T("icons/png/32x32/database_accept.png") ) {
        wxIcon icon(database_accept_xpm);
        return icon;
    }
    else if ( name == _T("icons/png/32x32/database_down.png") ) {
        wxIcon icon(database_down_xpm);
        return icon;
    }
    
    return wxNullIcon;
}


//////////////////////////////////////////////////////////////////////////////
// writeChangedLevel1Registers
//

bool frmDeviceConfig::writeChangedLevel1Registers(unsigned char nodeid) 
{
    int i;
    uint8_t val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;
    uint16_t page;
    uint8_t reg;

    // Nothing to the first time.
    if ( m_bFirstRead ) return true;

    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Writing modified registers"),
                                    m_gridRegisters->GetNumberRows()+1,
                                    this,
                                    wxPD_ELAPSED_TIME |
                                        wxPD_AUTO_HIDE |
                                        wxPD_APP_MODAL |
                                        wxPD_CAN_ABORT );

    // *********************************
    // Write data from changed registers
    // *********************************

    progressDlg.Pulse(_("Looking for changed registers..."));

    for (i = 0; i < m_gridRegisters->GetNumberRows(); i++) {

        if ( !progressDlg.Update(i) ) {
            rv = false;
            break;
        }

        if ( *wxRED == m_gridRegisters->GetCellTextColour(i, 2) ) {

            reg = getRegFromCell(i);
            page = getPageFromCell(i);
            val = vscp_readStringValue( m_gridRegisters->GetCellValue(i, 2 )); 

            progressDlg.Pulse (wxString::Format(_("Writing register %d:%d"), page, reg ) );
            
            if ( CANAL_ERROR_SUCCESS == m_csw.getDllInterface()->writeLevel1Register(nodeid, page, reg, &val ) ) {

                // Set value
                m_userRegisters.setValue( page, reg, val );

                // Update display
                strBuf = getFormattedValue(val);
                m_gridRegisters->SetCellValue(i, 2, strBuf);
                m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, i, 2);
                m_gridRegisters->SetCellTextColour(i, 2, *wxBLUE);
                m_gridRegisters->SelectRow(i);
                m_gridRegisters->MakeCellVisible(i, 2);
                m_gridRegisters->Update();

                updateDmGridConditional(reg, page);
                updateAbstractionGridConditional(reg, page);

            } 
            else {
                wxMessageBox(_("Failed to write register."));
                break;
            }
        
        } // Changed register

    } // for

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeChangedLevel2Registers
//

bool frmDeviceConfig::writeChangedLevel2Registers(void) 
{
    int i;
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;
    uint32_t reg;
    uint16_t page;

    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Writing modified registers"),
                                    m_gridRegisters->GetNumberRows()+1,
                                    this,
                                    wxPD_ELAPSED_TIME |
                                        wxPD_AUTO_HIDE |
                                        wxPD_APP_MODAL |
                                        wxPD_CAN_ABORT );

    // *********************************
    // Write data from changed registers
    // *********************************

    progressDlg.Pulse(_("Looking for changed registers..."));

    // Get the destination GUID
    cguid destGUID;
    uint8_t nodeid;
    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());

    }
    else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface GUID
        destGUID.getFromString(m_comboNodeID->GetValue());

    }

    for (i = 0; i < m_gridRegisters->GetNumberRows(); i++) {

        if (!progressDlg.Update(i)) {
            rv = false;
            break;
        }

        if (*wxRED == m_gridRegisters->GetCellTextColour(i, 2)) {

            reg = getRegFromCell(i);
            page = getPageFromCell(i);

            progressDlg.Pulse (wxString::Format(_("Writing register %d:%d"), page, reg ) );

            val = vscp_readStringValue(m_gridRegisters->GetCellValue(i, 2));

            if ( VSCP_ERROR_SUCCESS == 
                    m_csw.getTcpIpInterface()->writeLevel2Register( reg,
                                                                        page,
                                                                        &val,
                                                                        m_ifguid,
                                                                        &destGUID,
                                                                        m_bLevel2->GetValue() ) ) {
                // Set value
                m_userRegisters.setValue( page, reg, val );

                // Update display
                strBuf = getFormattedValue(val);
                m_gridRegisters->SetCellValue(i, 2, strBuf);
                m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, i, 2);
                m_gridRegisters->SetCellTextColour(i, 2, *wxBLUE);
                m_gridRegisters->SelectRow(i);
                m_gridRegisters->MakeCellVisible(i, 2);
                m_gridRegisters->Update();

                updateDmGridConditional(reg, page);
                updateAbstractionGridConditional(reg, page);
            }
            else {
               wxMessageBox(_("Failed to update data."));
            }

        } // Changed register

    } // for

    return rv;
}



//////////////////////////////////////////////////////////////////////////////
// writeStatusInfo
//

void frmDeviceConfig::writeStatusInfo(void) 
{
    wxString strHTML;
    wxString str;

    strHTML = _("<html><body>");
    strHTML += _("<h1>Node data</h1>");
    strHTML += _("<font color=\"#009900\">");

    strHTML += _("<b>Node id</b> : ");
    str = wxString::Format(_("%d"), m_stdRegisters.getNickname());
    strHTML += str;
    strHTML += _("<br>");

    if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
        strHTML += _("<b>Daemon Interface</b> : ");
        strHTML += m_comboNodeID->GetValue();
        strHTML += _("<br>");
    }

    // we have to change the order for the fetched GUID so
    // it match internal reprenentation
    strHTML += _("<b>GUID</b>: ");
    uint8_t guid[16];
    memcpy(guid, m_stdRegisters.getGUID(), 16);
    vscp_reverseGUID(guid);
    vscp_writeGuidArrayToString(m_stdRegisters.getGUID(), str);
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("<b>MDF URL</b>: ");
    strHTML += _("<a href=\"");
	m_stdRegisters.getMDF( str );
    strHTML += str;
    strHTML += _("\">");
    strHTML += str;
    strHTML += _("</a>");
    strHTML += _("<br>");

    strHTML += _("<b>Alarm:</b> ");
    if (m_stdRegisters.getAlarm()) {
        strHTML += _("Yes");
    } 
    else {
        strHTML += _("No");
    }
    strHTML += _("<br>");


    strHTML += _("<b>Node Control Flags:</b> ");
    if (m_stdRegisters.getNodeControl() & 0x10) {
        strHTML += _("[Register Write Protect] ");
    } 
    else {
        strHTML += _("[Register Read/Write] ");
    }

    switch ((m_stdRegisters.getNodeControl() & 0xC0) >> 6) {

        case 1:
            strHTML += _(" [Initialized] ");
            break;

        default:
            strHTML += _(" [Uninitialized] ");
            break;
    }

    strHTML += _("<br>");

    strHTML += _("<b>Firmware VSCP confirmance:</b> ");
    strHTML += wxString::Format(_("%d.%d"),
            m_stdRegisters.getConfirmanceVersionMajor(),
            m_stdRegisters.getConfirmanceVersonMinor());
    strHTML += _("<br>");

    strHTML += _("<b>User Device ID:</b> ");
    strHTML += wxString::Format(_("%d.%d.%d.%d.%d"),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_ID),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_ID + 1),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_ID + 2),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_ID + 3),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_ID + 3));
    strHTML += _("<br>");

    strHTML += _("<b>Manufacturer Device ID:</b> ");
    strHTML += wxString::Format(_("0x%08X - %d.%d.%d,%d"),
            m_stdRegisters.getManufacturerDeviceID(),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANDEV_ID),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 1),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 2),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANDEV_ID + 3));
    strHTML += _("<br>");

    strHTML += _("<b>Manufacturer sub device ID:</b> ");
    strHTML += wxString::Format(_("0x%08X - %d.%d.%d.%d"),
            m_stdRegisters.getManufacturerSubDeviceID(),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 1),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 2),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 3)
            );
    strHTML += _("<br>");

    strHTML += _("<b>Page select:</b> ");
    strHTML += wxString::Format(_("%d (MSB=%d LSB=%d)"),
            m_stdRegisters.getPage(),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_PAGE_SELECT_MSB),
            m_stdRegisters.getStandardReg(VSCP_STD_REGISTER_PAGE_SELECT_LSB));
    strHTML += _("<br>");

    strHTML += _("<b>Firmware version:</b> ");
    strHTML += m_stdRegisters.getFirmwareVersionString();
    strHTML += _("<br>");

    strHTML += _("<b>Boot loader algorithm:</b> ");
    strHTML += wxString::Format(_("%d - "),
            m_stdRegisters.getBootloaderAlgorithm());

    switch (m_stdRegisters.getBootloaderAlgorithm()) {

        case 0x00:
            strHTML += _("VSCP universal algorithm");
            break;

        case 0x01:
            strHTML += _("Microchip PIC algorithm 0");
            break;

        case 0x10:
            strHTML += _("Atmel AVR algorithm 0");
            break;

        case 0x20:
            strHTML += _("NXP ARM algorithm 0");
            break;

        case 0x30:
            strHTML += _("ST ARM algorithm 0");
            break;

        case 0xFF:
            strHTML += _("No bootloader implemented.");
            break;

        default:
            strHTML += _("Unknown algorithm.");
            break;
    }

    strHTML += _("<br>");

    strHTML += _("<b>Buffer size:</b> ");
    strHTML += wxString::Format(_("%d bytes. "),
            m_stdRegisters.getBufferSize());

    if (!m_stdRegisters.getBufferSize()) strHTML += _(" ( == default size (8 or 487 bytes) )");
    strHTML += _("<br>");

    strHTML += _("<b>Number of register pages:</b> ");
    strHTML += wxString::Format(_("%d"),
            m_stdRegisters.getNumberOfRegisterPages());
    if (m_stdRegisters.getNumberOfRegisterPages() > 22) {
        strHTML += _(" (Note: VSCP Works display max 22 pages.) ");
    }
    strHTML += _("<br>");

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        uint8_t data[8];
        memset(data, 0, 8);

        if (m_csw.getLevel1DmInfo(m_stdRegisters.getNickname(), data)) {
            strHTML += _("<b>Decision Matrix:</b> Rows=");
            strHTML += wxString::Format(_("%d "), data[0]);
            strHTML += _(" Offset=");
            strHTML += wxString::Format(_("%d (0x%02x)"), data[1], data[1]);
            strHTML += _(" Page start=");
            strHTML += wxString::Format(_("%d (0x%04x)"),
                    (data[2] << 8) + data[3],
                    (data[2] << 8) + data[3]);
            strHTML += _(" Page end=");
            strHTML += wxString::Format(_("%d (0x%04x)"),
                    (data[4] << 8) + data[5],
                    (data[4] << 8) + data[5]);
            strHTML += _("<br>");
        } else {
            strHTML += _("No Decision Matrix is available on this device.");
            strHTML += _("<br>");
        }

    } 
    else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

        unsigned char data[ 8 ];
        memset(data, 0, 8);

        if (m_csw.getLevel2DmInfo(m_ifguid, data)) {
            strHTML += _("<b>Decison Matrix:</b> Rows:");
            strHTML += wxString::Format(_("%d "), data[0]);
            strHTML += _(" <b>Offset:</b>");
            strHTML += wxString::Format(_("%d 0x%02x"), data[1], data[1]);
            strHTML += _(" Page start=");
            strHTML += wxString::Format(_("%d 0x%04x"),
                    (data[2] << 8) + data[3],
                    (data[2] << 8) + data[3]);
            strHTML += _(" <b>Page end:</b>");
            strHTML += wxString::Format(_("%d 0x%04x"),
                    (data[4] << 8) + data[5],
                    (data[4] << 8) + data[5]);
            strHTML += _("<br>");
        } 
        else {
            strHTML += _("No Decision Matrix is available on this device.");
            strHTML += _("<br>");
        }

    } // tcp/ip connection

    strHTML += _("</font>");

    // MDF Info
    strHTML += _("<h1>MDF Information</h1>");

    strHTML += _("<font color=\"#009900\">");

    // Manufacturer data
    strHTML += _("<b>Module name :</b> ");
    strHTML += m_mdf.m_strModule_Name;
    strHTML += _("<br>");

    strHTML += _("<b>Module model:</b> ");
    strHTML += m_mdf.m_strModule_Model;
    strHTML += _("<br>");

    strHTML += _("<b>Module version:</b> ");
    strHTML += m_mdf.m_strModule_Version;
    strHTML += _("<br>");

    strHTML += _("<b>Module last change:</b> ");
    strHTML += m_mdf.m_changeDate;
    strHTML += _("<br>");

    strHTML += _("<b>Module description:</b> ");
    strHTML += m_mdf.m_strModule_Description;
    strHTML += _("<br>");

    strHTML += _("<b>Module URL</b> : ");
    strHTML += _("<a href=\"");
    strHTML += m_mdf.m_strModule_InfoURL;
    strHTML += _("\">");
    strHTML += m_mdf.m_strModule_InfoURL;
    strHTML += _("</a>");
    strHTML += _("<br>");


    MDF_MANUFACTURER_LIST::iterator iter;
    for (iter = m_mdf.m_list_manufacturer.begin();
            iter != m_mdf.m_list_manufacturer.end(); ++iter) {

        strHTML += _("<hr><br>");

        CMDF_Manufacturer *manufacturer = *iter;
        strHTML += _("<b>Manufacturer:</b> ");
        strHTML += manufacturer->m_strName;
        strHTML += _("<br>");

        MDF_ADDRESS_LIST::iterator iterAddr;
        for (iterAddr = manufacturer->m_list_Address.begin();
                iterAddr != manufacturer->m_list_Address.end(); ++iterAddr) {

            CMDF_Address *address = *iterAddr;
            strHTML += _("<h4>Address</h4>");
            strHTML += _("<b>Street:</b> ");
            strHTML += address->m_strStreet;
            strHTML += _("<br>");
            strHTML += _("<b>Town:</b> ");
            strHTML += address->m_strTown;
            strHTML += _("<br>");
            strHTML += _("<b>City:</b> ");
            strHTML += address->m_strCity;
            strHTML += _("<br>");
            strHTML += _("<b>Post Code:</b> ");
            strHTML += address->m_strPostCode;
            strHTML += _("<br>");
            strHTML += _("<b>State:</b> ");
            strHTML += address->m_strState;
            strHTML += _("<br>");
            strHTML += _("<b>Region:</b> ");
            strHTML += address->m_strRegion;
            strHTML += _("<br>");
            strHTML += _("<b>Country:</b> ");
            strHTML += address->m_strCountry;
            strHTML += _("<br><br>");
        }

        MDF_ITEM_LIST::iterator iterPhone;
        for (iterPhone = manufacturer->m_list_Phone.begin();
                iterPhone != manufacturer->m_list_Phone.end(); ++iterPhone) {

            CMDF_Item *phone = *iterPhone;
            strHTML += _("<b>Phone:</b> ");
            strHTML += phone->m_strItem;
            strHTML += _(" ");
            strHTML += phone->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterFax;
        for (iterFax = manufacturer->m_list_Fax.begin();
                iterFax != manufacturer->m_list_Fax.end(); ++iterFax) {

            CMDF_Item *fax = *iterFax;
            strHTML += _("<b>Fax:</b> ");
            strHTML += fax->m_strItem;
            strHTML += _(" ");
            strHTML += fax->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterEmail;
        for (iterEmail = manufacturer->m_list_Email.begin();
                iterEmail != manufacturer->m_list_Email.end(); ++iterEmail) {

            CMDF_Item *email = *iterEmail;
            strHTML += _("<b>Email:</b> <a href=\"");
            strHTML += email->m_strItem;
            strHTML += _("\" >");
            strHTML += email->m_strItem;
            strHTML += _("</a> ");
            strHTML += email->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterWeb;
        for (iterWeb = manufacturer->m_list_Web.begin();
                iterWeb != manufacturer->m_list_Web.end(); ++iterWeb) {

            CMDF_Item *web = *iterWeb;
            strHTML += _("<b>Web:</b> <a href=\"");
            strHTML += web->m_strItem;
            strHTML += _("\">");
            strHTML += web->m_strItem;
            strHTML += _("</a> ");
            strHTML += web->m_strDescription;
            strHTML += _("<br>");
        }

    } // manufacturer

    strHTML += _("</font>");
    strHTML += _("</body></html>");

    m_StatusWnd->SetPage(strHTML);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnComboNodeIDSelected
//

void frmDeviceConfig::OnComboNodeIDSelected(wxCommandEvent& event) 
{
    // Clear all content
    clearAllContent();

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnComboNodeIDUpdated
//

void frmDeviceConfig::OnComboNodeIDUpdated(wxCommandEvent& event) 
{
    // Clear all content
    clearAllContent();

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////////
// clearAllContent
//

void frmDeviceConfig::clearAllContent(void) 
{
    // Mark as first time
    m_bFirstRead = true;

    // Clear register grid
    if (m_gridRegisters->GetNumberRows()) {
        m_gridRegisters->DeleteRows(0, m_gridRegisters->GetNumberRows());
    }

    // Clear abstraction grid
    if (m_gridAbstractions->GetNumberRows()) {
        m_gridAbstractions->DeleteRows(0, m_gridAbstractions->GetNumberRows());
    }

    // Clear DM grid
    if (m_gridDM->GetNumberRows()) {
        m_gridDM->DeleteRows(0, m_gridDM->GetNumberRows());
    }


    // Clear the text area
    m_StatusWnd->SetPage(_(""));

    // Diable the "extra buttons"
    m_ctrlButtonLoadMDF->Enable(false);
    m_ctrlButtonWizard->Enable(false);

    // Clean po MDF storage
    m_mdf.clearStorage();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillStandardRegisters
//

void frmDeviceConfig::fillStandardRegisters() 
{
    wxString strBuf;
    wxString str;
    int i, j;

    wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
    wxFont fontBold = defaultFont;
    fontBold.SetStyle(wxFONTSTYLE_NORMAL);
    fontBold.SetWeight(wxFONTWEIGHT_BOLD);

    ///////////////////////////////////
    //       Standard Registers
    ///////////////////////////////////

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("80");
    } 
    else {
        strBuf = _("FFFFFF80");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x80)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Alarm Status Register"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("81");
    } 
    else {
        strBuf = _("FFFFFF81");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x81)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("VSCP Major version number"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("82");
    } 
    else {
        strBuf = _("FFFFFF82");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x82)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("VSCP Minor version number"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("83");
    } 
    else {
        strBuf = _("FFFFFF83");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x83)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Node Control Flags\r test"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("84");
    } 
    else {
        strBuf = _("FFFFFF84");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x84)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("User ID 0"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("85");
    } 
    else {
        strBuf = _("FFFFFF85");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x85)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("User ID 1"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("86");
    } 
    else {
        strBuf = _("FFFFFF86");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x86)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("User ID 2"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("87");
    } 
    else {
        strBuf = _("FFFFFF87");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x87)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("User ID 3"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("88");
    } 
    else {
        strBuf = _("FFFFFF88");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x88)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("User ID 4"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("89");
    } 
    else {
        strBuf = _("FFFFFF89");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x89)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer device id 0"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8A");
    } 
    else {
        strBuf = _("FFFFFF8A");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8A)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer device id 1"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8B");
    } 
    else {
        strBuf = _("FFFFFF8B");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8B)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer device id 2"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8C");
    } 
    else {
        strBuf = _("FFFFFF8C");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8C)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer device id 3"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8D");
    } 
    else {
        strBuf = _("FFFFFF8D");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8D)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer sub device id 0"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8E");
    } 
    else {
        strBuf = _("FFFFFF8E");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8E)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer sub device id 1"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("8F");
    } 
    else {
        strBuf = _("FFFFFF8F");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x8F)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer sub device id 2"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("90");
    } 
    else {
        strBuf = _("FFFFFF90");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x90)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Manufacturer sub device id 3"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("91");
    }
    else {
        strBuf = _("FFFFFF91");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x91)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Nickname id"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("92");
    } 
    else {
        strBuf = _("FFFFFF92");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x92)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Page select register MSB"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("93");
    } 
    else {
        strBuf = _("FFFFFF93");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x93)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Page select register LSB"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("94");
    } 
    else {
        strBuf = _("FFFFFF94");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x94)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Firmware major version number"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("95");
    } 
    else {
        strBuf = _("FFFFFF95");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x95)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Firmware minor version number"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("96");
    } 
    else {
        strBuf = _("FFFFFF96");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x96)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Firmware sub minor version number"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("97");
    } 
    else {
        strBuf = _("FFFFFF97");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x97)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Boot loader algorithm"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("98");
    } 
    else {
        strBuf = _("FFFFFF98");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x98)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Buffer Size"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("99");
    } 
    else {
        strBuf = _("FFFFFF99");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0x99)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Number of register pages used."));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D0");
    } 
    else {
        strBuf = _("FFFFFFD0");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD0)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 15 MSB"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }

    // Write to grid also
    vscp_writeGuidArrayToString(m_stdRegisters.getGUID(), str);
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 15, LSB\nGUID=") + str);

    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D1");
    } 
    else {
        strBuf = _("FFFFFFD1");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD1)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 14"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D2");
    } 
    else {
        strBuf = _("FFFFFFD2");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD2)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 13"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    strBuf.Printf(_("0x%02x"), 0xd3 );
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D3");
    } 
    else {
        strBuf = _("FFFFFFD3");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD3)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 12"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D4");
    } 
    else {
        strBuf = _("FFFFFFD4");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD4)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 11"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    strBuf.Printf(_("0x%02x"), 0xd5);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D5");
    } 
    else {
        strBuf = _("FFFFFFD5");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD5)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 10"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D6");
    } 
    else {
        strBuf = _("FFFFFFD6");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD6)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 9"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D7");
    } 
    else {
        strBuf = _("FFFFFFD7");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD7)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 8"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    strBuf.Printf(_("0x%02x"), 0xd8);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D8");
    } 
    else {
        strBuf = _("FFFFFFD8");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD8)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 7"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D9");
    } 
    else {
        strBuf = _("FFFFFFD9");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xD9)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 6"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DA");
    } 
    else {
        strBuf = _("FFFFFFDA");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDA)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 5"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DB");
    } 
    else {
        strBuf = _("FFFFFFDB");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDB)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 4"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DC");
    } 
    else {
        strBuf = _("FFFFFFDC");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDC)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 3"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DD");
    } 
    else {
        strBuf = _("FFFFFFDD");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDD)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 2"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DE");
    } 
    else {
        strBuf = _("FFFFFFDE");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDE)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 1"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("DF");
    } 
    else {
        strBuf = _("FFFFFFDF");
    }
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
            2,
            getFormattedValue(m_stdRegisters.getStandardReg(0xDF)));
    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 0, LSB"));
    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

    for (i = 0; i < 4; i++) {
        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0x12));
    }


    // MDF
    bool bFirstMDFrow = true;
    uint32_t nFirstMDFrow = 0;
    for (i = 0; i < 32; i++) {

        m_gridRegisters->AppendRows(1);

        if (bFirstMDFrow) {
            nFirstMDFrow = m_gridRegisters->GetNumberRows() - 1;
            bFirstMDFrow = false;
        }

        if (!m_bLevel2->GetValue()) {
            strBuf.Printf(_("%02X"), 0xe0 + i);
        } 
        else {
            strBuf.Printf(_("%08lX"), 0xFFFFFFE0 + i);
        }
        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
        m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
        m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("r-"));
        m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
        m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                2,
                getFormattedValue(m_stdRegisters.getStandardReg(0xE0 + i)));
        m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
        m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

        if (0 == i) {
            str = _(", MSB");
        } 
        else if (31 == i) {
            str = _(", LSB");
        } 
        else {
            str = _("");
        }

        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("Module Description File URL") + str);
        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
        m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
        
        // Make all parts of the row visible
        m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

        for (j = 0; j < 4; j++) {
            m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1,
                    j, wxColour(0xff, 0xff, 0xd2));
        }

    }

    // Write to grid also
    m_stdRegisters.getMDF( str );
    m_gridRegisters->SetCellValue(nFirstMDFrow, 3, _("Module Description File URL, MSB\n") + str);

    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(nFirstMDFrow);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonUpdateClick
//

void frmDeviceConfig::OnButtonUpdateClick( wxCommandEvent& event ) 
{
    
    wxString strPath;
    uint8_t nodeid = 0;
    cguid destGUID;

    wxBusyCursor wait;
    
    // Check if we should do a full update
    if ( m_chkFullUpdate->GetValue() ) {

        // Update changed registers first
        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel1Registers(nodeid);
        } 
        else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel2Registers();
        }

        clearAllContent();
        m_bFirstRead = true;

    }

    

    // Get nickname
    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());
    } 
    else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
        destGUID.getFromString(m_comboNodeID->GetValue());
    }

    if ( m_bFirstRead ) {

        // Check if MDF should be fetched from local file or server
        if ( m_chkMdfFromFile->GetValue() ) {

            // Get MDF from local file
            wxFileDialog dlg( this,
                              _( "Choose file to load MDF from " ),
                              wxStandardPaths::Get().GetUserDataDir(),
                              _( "" ),
                              _( "MDF (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*" ) );

            if ( wxID_OK == dlg.ShowModal() ) {
                strPath = dlg.GetPath();
            }

        }

        wxProgressDialog progressDlg( _( "VSCP Works" ),
                                      _( "Updating Data" ),
                                      100,
                                      this,
                                      wxPD_ELAPSED_TIME |
                                      wxPD_AUTO_HIDE |
                                      wxPD_APP_MODAL );

        // Driver
        if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

            // Read standard registers           
            progressDlg.Update( 10, _("Reading standard registers of device 1/8.")); 

            if ( CANAL_ERROR_SUCCESS !=
                m_csw.getDllInterface()->readRegistersfromLevel1Device( nodeid,
                                                                            0x80,
                                                                            0,      // page
                                                                            128,    // count
                                                                            m_stdRegisters.getRegs() ) ) {
                ::wxMessageBox(_("Failed to read standard registers of device."), _("VSCP Works"), wxICON_ERROR);
                return;
            }
 
            // Fetch MDF from local file or server
            if ( !m_chkMdfFromFile->GetValue() ) {

                // Get MDF from device
                progressDlg.Update( 15, _("Fetching MDF path from device. 2/8.") );

                // We need it to continue
                m_stdRegisters.getMDF( strPath );
                if ( 0 == strPath.Length() ) {
                    ::wxMessageBox( _( "Empty MDF path returned." ), _( "VSCP Works" ), wxICON_ERROR );
                    return;
                }

            }

            // translate mdf path if translation available for this url
            wxString translate = g_Config.m_mfProxyHashTable[ strPath  ];
            if ( translate.Length() ) {
                ::wxGetApp().logMsg( wxString::Format( _( "Device URL %s translated to %s." ), strPath.mbc_str(), translate.mbc_str() ),
                                     VSCPWORKS_LOGMSG_INFO );
                strPath = translate;
            }

            // Load and parse the MDF
            progressDlg.Update( 20, _("Loading and parsing MDF. 2/8.") );            
            m_mdf.load( strPath, m_chkMdfFromFile->GetValue() );

            wxArrayLong pageArray;
            uint32_t nPages = m_mdf.getPages( pageArray );

            if ( 0 == nPages ) {
                wxMessageBox( _("MDF returns zero pages (which is an error) we still will read one page."), 
                                _("VSCP Works"), 
                                wxICON_WARNING );
                // Add standard page
                nPages = 1;
                pageArray.Add(0);
            }

            m_userRegisters.init( pageArray );

            for ( uint32_t i=0; i<nPages; i++ ) {

				long currpage = pageArray.Item( i );
                wxString str = wxString::Format( _("Fetching user registers for page %ld 3/8"), currpage );
                progressDlg.Update( 25, str );

                if ( CANAL_ERROR_SUCCESS !=
                        m_csw.getDllInterface()->readRegistersfromLevel1Device( nodeid,
                                                                                    0,
                                                                                    pageArray[i],   // page
                                                                                    128,            // count
                                                                                    m_userRegisters.getRegs4Page( pageArray[i] ) ) ) {
                    ::wxMessageBox(_("Failed to read user registers of device."), _("VSCP Works"), wxICON_ERROR );
                    return;
                }

            }

            wxString strBuf;
            wxString str;

            progressDlg.Update( 30, _("Filling in register information 2/8.") );
            wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
            wxFont fontBold = defaultFont;
            fontBold.SetStyle(wxFONTSTYLE_NORMAL);
            fontBold.SetWeight(wxFONTWEIGHT_BOLD);

            // Fill in register descriptions
            MDF_REGISTER_LIST::iterator iter;
            for (iter = m_mdf.m_list_register.begin(); iter != m_mdf.m_list_register.end(); ++iter) {

                CMDF_Register *reg = *iter;
                int cnt = 0;

                // Add a new row
                m_gridRegisters->AppendRows( 1 );

                // Register
                strBuf.Printf( _("%04X:%02X"), reg->m_nPage, reg->m_nOffset );
   
                m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1, 0, strBuf );
                m_gridRegisters->SetCellAlignment( wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0 );
                m_gridRegisters->SetCellFont( m_gridRegisters->GetNumberRows() - 1, 0, fontBold );
                m_gridRegisters->SetReadOnly( m_gridRegisters->GetNumberRows() - 1, 0 );

                // Value
                reg->m_value = m_userRegisters.getValue( reg->m_nPage, reg->m_nOffset );
                m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                        2,
                                                        getFormattedValue( reg->m_value ) );
                m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
                m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);

                str.Printf(reg->m_strName + _("\n") + reg->m_strDescription);
                m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                    3,
                                                    str);
                m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);

                wxString strAccess;
                if (reg->m_nAccess & MDF_ACCESS_READ) strAccess = _("r");
                if (reg->m_nAccess & MDF_ACCESS_WRITE) {
                    strAccess += _("w");
                } 
                else {
                    strAccess += _("-");
                }

                m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                    1,
                                                    strAccess);
                // Protect cell if readonly
                if (wxNOT_FOUND == strAccess.Find(_("w"))) {
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);
                }
                m_gridRegisters->SetCellAlignment(m_gridRegisters->GetNumberRows() - 1, 1, wxALIGN_CENTRE, wxALIGN_CENTRE);

                // Make all parts of the row visible
                m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

            }

            // Fill grid with standard registers
            progressDlg.Update( 40, _("Filling in standard register information 4/8.") );
            fillStandardRegisters();

            // Write status
            progressDlg.Update( 50, _("Filling in status information 5/8.") );
            writeStatusInfo();
            
            m_bFirstRead = false;

        } 
        // Remote server/device
        else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

            // Read standard registers           
            progressDlg.Update( 10, _("Reading standard registers of device 1/8.")); 
            if ( VSCP_ERROR_SUCCESS != 
                m_csw.getTcpIpInterface()->readLevel2Registers( m_bLevel2->GetValue() ? 0xFFFFFF80 : 0x80,
                                                                    0,      // page
                                                                    128,    // count
                                                                    m_stdRegisters.getRegs(),
                                                                    m_ifguid,
                                                                    &destGUID,
                                                                    m_bLevel2->GetValue() ) ) {
                ::wxMessageBox(_("Failed to read standard registers of device."), _("VSCP Works"), wxICON_ERROR);
                return;
            }

            // Check if MDF should be fetched from local file or server
            if ( !m_chkMdfFromFile->GetValue() ) {

                // Get MDF from device
                progressDlg.Update( 10, _("Fetching MDF path from device 2/8.") );

                // We need it to continue
                m_stdRegisters.getMDF( strPath );
                if ( 0 == strPath.Length() ) {
                    ::wxMessageBox( _( "Empty MDF path returned." ), _( "VSCP Works" ), wxICON_ERROR );
                    return;
                }

            }

            // translate mdf path if translation available for this url
            wxString translate = g_Config.m_mfProxyHashTable[ strPath ];
            if ( translate.Length() ) {
                ::wxGetApp().logMsg( wxString::Format( _( "Device URL %s translated to %s." ), strPath.mbc_str(), translate.mbc_str() ),
                                     VSCPWORKS_LOGMSG_INFO );
                strPath = translate;
            }

            
            // Load and parse the MDF
            progressDlg.Update( 20, _("Loading and parsing MDF 2/8.") );

            if ( !m_mdf.load( strPath, m_chkMdfFromFile->GetValue() ) ) {
                // We try to continue anyway
                wxMessageBox(_("Failed to load MDF but will try to continue anyway.") );
            }

            // Get the application registers
            progressDlg.Update( 40, _("Reading application registers 3/8.") );

            wxArrayLong pageArray;
            uint32_t nPage = m_mdf.getPages( pageArray );

            if ( 0 == nPage ) {
                wxMessageBox( _("MDF returns zero pages (which is an error) we still will read one page."), 
                                _("VSCP Works"), 
                                wxICON_WARNING );

                // Add standard page
                nPage = 1;
                pageArray.Add(0);

            }

            m_userRegisters.init( pageArray );

            for ( uint32_t i = 0; i<nPage; i++ ) {

                wxString str = wxString::Format( _("Fetching user regsisters for page %d 4/8"), pageArray[i] );
                progressDlg.Update( 30, str );

                if ( VSCP_ERROR_SUCCESS != 
                    m_csw.getTcpIpInterface()->readLevel2Registers( 0,                  // From reg=0
                                                                    pageArray[i],       // page
                                                                    128,                // count
                                                                    m_userRegisters.getRegs4Page( pageArray[i] ),
                                                                    m_ifguid,
                                                                    &destGUID,
                                                                    m_bLevel2->GetValue() ) ) {
                    ::wxMessageBox(_("Failed to read user registers of device."), _("VSCP Works"), wxICON_ERROR );
                    return;
                }

            }

            wxString strBuf;
            wxString str;

            wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
            wxFont fontBold = defaultFont;
            fontBold.SetStyle(wxFONTSTYLE_NORMAL);
            fontBold.SetWeight(wxFONTWEIGHT_BOLD);

            // Fill in register descriptions
            uint8_t progress = 0;
            uint8_t progress_count;

            if (m_mdf.m_list_register.GetCount()) progress_count = 256 / m_mdf.m_list_register.GetCount();
            MDF_REGISTER_LIST::iterator iter;
            for (iter = m_mdf.m_list_register.begin(); iter != m_mdf.m_list_register.end(); ++iter) {

                CMDF_Register *reg = *iter;
                int cnt = 0;
                if (reg->m_nPage < MAX_CONFIG_REGISTER_PAGE) {

                    // Add a new row
                    m_gridRegisters->AppendRows(1);

                    // Register
                    strBuf.Printf(_("%04X:%X"), reg->m_nPage, reg->m_nOffset);
                    progressDlg.Update( 50, _("Reading page:register: ") + strBuf );

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);

                    reg->m_value = m_userRegisters.getValue( reg->m_nPage, reg->m_nOffset );
                    m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                    2,
                                                    getFormattedValue( reg->m_value ) );
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);

                    str.Printf(reg->m_strName + _("\n") + reg->m_strDescription);
                    m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                    3,
                                                    str);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);

                    wxString strAccess;
                    if (reg->m_nAccess & MDF_ACCESS_READ) strAccess = _("r");
                    if (reg->m_nAccess & MDF_ACCESS_WRITE) {
                        strAccess += _("w");
                    } 
                    else {
                        strAccess += _("-");
                    }

                    m_gridRegisters->SetCellValue( m_gridRegisters->GetNumberRows() - 1,
                                                    1,
                                                    strAccess );
                    // Protect cell if readonly
                    if (wxNOT_FOUND == strAccess.Find(_("w"))) {
                        m_gridRegisters->SetReadOnly( m_gridRegisters->GetNumberRows() - 1, 2 );
                    }
                    m_gridRegisters->SetCellAlignment( m_gridRegisters->GetNumberRows() - 1, 1, wxALIGN_CENTRE, wxALIGN_CENTRE );

                    // Make all parts of the row visible
                    m_gridRegisters->AutoSizeRow( m_gridRegisters->GetNumberRows() - 1);

                }
            }

            // Fill grid with standard registers
            progressDlg.Update( 50, _("Fill standard register information 5/8.") );
            fillStandardRegisters();

            // Write status
            progressDlg.Update( 60, _("Fill standard status information 6/8.") );
            writeStatusInfo();

            m_bFirstRead = false;
        }

        // Update the DM grid   
        progressDlg.Update( 70, _("Update DM grid 7/8.") );
        updateDmGrid();

        // Update abstractions
        progressDlg.Update( 90, _("Update Abstraction grid 8/8.") );
        updateAbstractionGrid();

        // Enable load defaults buttons
        m_ctrlButtonLoadMDF->Enable(true);

        progressDlg.Update( 100, _("Done.") );

    }            
    // update
    else {

        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel1Registers( nodeid );
        } 
        else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel2Registers();
        }

    }

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonLoadDefaultsClick
//

void frmDeviceConfig::OnButtonLoadDefaultsClick( wxCommandEvent& event ) 
{
    wxString strBuf;
    wxString str;
    uint8_t val;
    uint8_t nodeid = 0;
    uint32_t row = 0;

    wxBusyCursor wait;

    // Get nickname
    nodeid = vscp_readStringValue(m_comboNodeID->GetValue());

    wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
    wxFont fontBold = defaultFont;
    fontBold.SetStyle(wxFONTSTYLE_NORMAL);
    fontBold.SetWeight(wxFONTWEIGHT_BOLD);

    // Fill in register descriptions
    uint8_t progress = 0;
    uint8_t progress_count;
    if (m_mdf.m_list_register.GetCount()) progress_count = 256 / m_mdf.m_list_register.GetCount();
    MDF_REGISTER_LIST::iterator iter;
    for (iter = m_mdf.m_list_register.begin(); iter != m_mdf.m_list_register.end(); ++iter) {

        CMDF_Register *reg = *iter;
        int cnt = 0;

            if ( wxNOT_FOUND == reg->m_strDefault.Find(_("UNDEF") ) ) {

                val = vscp_readStringValue(reg->m_strDefault);
                strBuf = getFormattedValue(val);
                m_gridRegisters->SelectRow(row);
                m_gridRegisters->SetCellValue(row, 2, strBuf);
                m_gridRegisters->MakeCellVisible(row, 2);
                m_gridRegisters->SetCellTextColour(row, 2, *wxRED);
                m_gridRegisters->Update();

            // Make all parts of the row visible
            m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

            row++;
        }

    }

    OnButtonUpdateClick( event );

    // Update the DM grid    
    updateDmGrid();

    // Update abstractions
    updateAbstractionGrid();


    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonWizardClick
//

void frmDeviceConfig::OnButtonWizardClick(wxCommandEvent& event) 
{
    wxMessageBox(_("The wizard is not implemented yet!"));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCellRightClick
//

void frmDeviceConfig::OnCellRightClick(wxGridEvent& event) 
{
    wxMenu menu;

    wxPoint pos = ClientToScreen(event.GetPosition());

    if (ID_GRID_REGISTERS == event.GetId()) {

        menu.Append(Menu_Popup_Update, _T("Update"));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_Read_Value, _T("Read value(s) for selected row(s)"));
        menu.Append(Menu_Popup_Write_Value, _T("Write value(s) for selected row(s)"));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_Undo, _T("Undo value(s) for selected row(s)"));
        menu.Append(Menu_Popup_Default, _T("Write default for selected row(s)"));
        menu.AppendSeparator();
        menu.Append(/*Menu_Popup_Read_Value*/ Menu_Popup_go_VSCP, _T("Be Hungry - Stay Foolish!"));

        PopupMenu(&menu);

    } 
    else if ( ID_GRID_ABSTRACTIONS == event.GetId() ) {



    } 
    else if ( ID_GRID_DM == event.GetId() ) {

        menu.Append(Menu_Popup_dm_enable_row, _T("Enable row."));
        menu.Append(Menu_Popup_dm_disable_row, _T("Disable row."));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_dm_row_wizard, _T("Run DM setup wizard for this row..."));

        PopupMenu(&menu);

    }

    event.Skip(false);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// readValueSelectedRow
//

void frmDeviceConfig::readValueSelectedRow( wxCommandEvent& WXUNUSED( event ) ) 
{
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        // Get Interface id
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());
    }

    if ( m_gridRegisters->GetNumberRows() ) {

        wxArrayInt selrows = m_gridRegisters->GetSelectedRows();

        if (selrows.GetCount()) {

            for (int i = selrows.GetCount() - 1; i >= 0; i--) {

                uint8_t val;

                uint16_t page;
                page = getPageFromCell(selrows[i]);

                uint32_t reg;
                reg = getRegFromCell(selrows[i]);

                if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

                    if ( CANAL_ERROR_SUCCESS == m_csw.getDllInterface()->readLevel1Register( nodeid, page, reg, &val)) {

                        // Set value
                        m_userRegisters.setValue( page, reg, val );

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);
                    } 
                    else {
                        wxMessageBox( _("Failed to read register value.") );
                    }
                } 
                else if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {

                    // Get the destination GUID
                    cguid destGUID;
                    destGUID.getFromString(m_comboNodeID->GetValue());

                    if ( VSCP_ERROR_SUCCESS == 
                        m_csw.getTcpIpInterface()->readLevel2Registers( reg,
                                                                            page,
	                                                                        1,
                                                                            &val,
                                                                            m_ifguid,
	                                                                        &destGUID ) ) {

                        // Set value
                        m_userRegisters.setValue( page, reg, val );
                
                        // Update display
                        strBuf.Printf(_("0x%02lx"), val);
                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } 
                    else {
                        wxMessageBox( _("Failed to read register value.") );
                    }

                } // Interface
            }

        } 
        else {
            wxMessageBox(_("No row(s) selected!"));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// writeValueSelectedRow
//

void frmDeviceConfig::writeValueSelectedRow(wxCommandEvent& WXUNUSED(event)) 
{
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());

    }
    else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface GUID
        destGUID.getFromString(m_comboNodeID->GetValue());

    }


    if (m_gridRegisters->GetNumberRows()) {

        wxArrayInt selrows = m_gridRegisters->GetSelectedRows();

        if (selrows.GetCount()) {

            for (int i = selrows.GetCount() - 1; i >= 0; i--) {

                // Must be writable
                wxString permissions =
                        m_gridRegisters->GetCellValue(selrows[ i ], 1);
                if (wxNOT_FOUND == permissions.Find(_("w"))) {
                    wxMessageBox(_("This register is not writable!"));
                } 
                else {
                    uint8_t val =
                            vscp_readStringValue(m_gridRegisters->GetCellValue(selrows[i], 2));

                    uint16_t page;
                    page = getPageFromCell(selrows[i]);

                    uint32_t reg;
                    reg = getRegFromCell(selrows[i]);


                    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        if ( CANAL_ERROR_SUCCESS ==
                            m_csw.getDllInterface()->writeLevel1Register(nodeid, page, reg, &val) ) {

                            // Set value
                            m_userRegisters.setValue( page, reg, val );

                            // Update display
                            strBuf = getFormattedValue(val);

                            m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);

                            m_gridRegisters->SelectRow(selrows[i]);
                            m_gridRegisters->MakeCellVisible(selrows[i], 2);
                            m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxBLUE);
                            m_gridRegisters->Update();

                            updateDmGridConditional(reg, page);
                            updateAbstractionGridConditional(reg, page);
                        }
                        else {
                            wxMessageBox(_("Failed to write value."));
                        }

                    } 
                    else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        if ( VSCP_ERROR_SUCCESS == 
                                m_csw.getTcpIpInterface()->writeLevel2Register( reg,
                                                                            page,
                                                                            &val,
                                                                            m_ifguid,
                                                                            &destGUID,
                                                                            m_bLevel2->GetValue() ) ) {

                            // Set value
                            m_userRegisters.setValue( page, reg, val );

                            // Update display
                            strBuf.Printf(_("0x%02lx"), val);
                            m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);

                            m_gridRegisters->SelectRow(selrows[i]);
                            m_gridRegisters->MakeCellVisible(selrows[i], 2);
                            m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxBLUE);
                            m_gridRegisters->Update();

                            updateDmGridConditional(reg, page);
                            updateAbstractionGridConditional(reg, page);

                        }
                        else {
                            wxMessageBox( _("Failed to write register value.") );
                        }

                    }

                }

            }

        } else {
            wxMessageBox(_("No rows selected!"));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// undoValueSelectedRow
//

void frmDeviceConfig::undoValueSelectedRow(wxCommandEvent& WXUNUSED(event)) 
{
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;
    destGUID.getFromString( m_comboNodeID->GetValue() );

    // Select the row
    m_gridRegisters->SelectRow( m_lastLeftClickRow );

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        // Get Interface id
        nodeid = vscp_readStringValue( m_comboNodeID->GetValue() );
    }

    if ( m_gridRegisters->GetNumberRows() ) {

        wxArrayInt selrows = m_gridRegisters->GetSelectedRows();

        if ( selrows.GetCount() ) {

            for ( int i = selrows.GetCount() - 1; i >= 0; i-- ) {

                // Must be writable
                wxString permissions =
                        m_gridRegisters->GetCellValue( selrows[ i ], 1 );
                if ( wxNOT_FOUND == permissions.Find(_("w") ) ) {
                    wxMessageBox(_("This register is not writable!"));
                } 
                else {

                    uint8_t val;

                    uint8_t reg = getRegFromCell( selrows[ i ] );
                    uint32_t page = getPageFromCell( selrows[ i ] );

                    CMDF_Register *mdfRegister = m_mdf.getMDFRegister(reg, page);
                    if (NULL == mdfRegister) {
                        wxMessageBox(_("Register is unkown. Report possible iternal problem."));
                        continue;
                    }

                    val = mdfRegister->m_value;
                    //val = m_userRegisters.getValue( page, reg );

                    if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.getDllInterface()->writeLevel1Register( nodeid, page, reg, &val );

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxRED);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } 
                    else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        if ( VSCP_ERROR_SUCCESS == 
                                m_csw.getTcpIpInterface()->writeLevel2Register( reg,
                                                                                    page,
                                                                                    &val,
                                                                                    m_ifguid,
                                                                                    &destGUID,
                                                                                    m_bLevel2->GetValue() ) ) {
                            // Update display
                            strBuf.Printf(_("0x%02lx"), val);
                            m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                            m_gridRegisters->SelectRow(selrows[i]);
                            m_gridRegisters->MakeCellVisible(selrows[i], 2);
                            m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxRED);
                            m_gridRegisters->Update();

                            updateDmGridConditional(reg, page);
                            updateAbstractionGridConditional(reg, page);
                        }
                        else {
                            wxMessageBox( _("Failed to write register.") );
                        }
                        
                    }

                }

            }

        } 
        else {
            wxMessageBox( _("No rows selected!") );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defaultValueSelectedRow
//

void frmDeviceConfig::defaultValueSelectedRow(wxCommandEvent& WXUNUSED(event)) 
{
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {
        // Get Interface id
        nodeid = vscp_readStringValue(m_comboNodeID->GetValue());
    }

    if ( m_gridRegisters->GetNumberRows() ) {

        wxArrayInt selrows = m_gridRegisters->GetSelectedRows();

        if (selrows.GetCount()) {

            for (int i = selrows.GetCount() - 1; i >= 0; i--) {

                // Must be writable
                wxString permissions =
                        m_gridRegisters->GetCellValue( selrows[i], 1 );
                if (wxNOT_FOUND == permissions.Find(_("w"))) {
                    wxMessageBox(_("This register is not writable!"));
                } 
                else {

                    uint8_t val;

                    uint8_t reg = getRegFromCell( selrows[i] );
                    uint32_t page = getPageFromCell( selrows[i] );

                    CMDF_Register *mdfRegister = m_mdf.getMDFRegister(reg, page);
                    if (NULL == mdfRegister) {
                        wxMessageBox(_("Register is unkown. Report possible iternal problem."));
                        continue;
                    }

                    val = vscp_readStringValue( mdfRegister->m_strDefault );

                    if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        if ( CANAL_ERROR_SUCCESS == m_csw.getDllInterface()->writeLevel1Register( nodeid, page, reg, &val ) ) {

                            // Set value
                            m_userRegisters.setValue( page, reg, val );

                            // Update display
                            strBuf = getFormattedValue(val);

                            m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                            m_gridRegisters->SelectRow(selrows[i]);
                            m_gridRegisters->MakeCellVisible(selrows[i], 2);
                            m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxBLUE);
                            m_gridRegisters->Update();

                            updateDmGridConditional(reg, page);
                            updateAbstractionGridConditional(reg, page);

                        }
                        else {
                            wxMessageBox(_("Failed to write default value."));    
                        }

                    } 
                    else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        if ( VSCP_ERROR_SUCCESS == 
                            m_csw.getTcpIpInterface()->writeLevel2Register( reg,
                                                                            page,
                                                                            &val,
                                                                            m_ifguid,
                                                                            &destGUID,
                                                                            m_bLevel2->GetValue() ) ) {
                            // Set value
                            m_userRegisters.setValue( page, reg, val );

                            // Update display
                            strBuf.Printf( _("0x%02lx"), val );
                            m_gridRegisters->SetCellValue( selrows[i], 2, strBuf );
                            m_gridRegisters->SelectRow(selrows[i]);
                            m_gridRegisters->MakeCellVisible(selrows[i], 2);
                            m_gridRegisters->SetCellTextColour( selrows[i], 2, *wxBLUE );
                            m_gridRegisters->Update();

                            updateDmGridConditional(reg, page);
                            updateAbstractionGridConditional(reg, page);

                        }
                        else {
                             wxMessageBox( _("Failed to write register.") );
                        }

                    }

                }

            }

        } 
        else {
            wxMessageBox(_("No rows selected!"));
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnLeftDClick
//

void frmDeviceConfig::OnLeftDClick( wxGridEvent& event ) 
{
    uint8_t reg;

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    if ( ID_GRID_REGISTERS == event.GetId() ) {
        m_gridRegisters->SelectRow( event.GetRow() );
    } 
    else if ( ID_GRID_ABSTRACTIONS == event.GetId() ) {

        uint8_t pos;
        wxString strValue;
        wxString strBuf;
        uint8_t val = 0;

        DialogAbstractionEdit dlg(this);
        strValue = m_gridAbstractions->GetCellValue( event.GetRow(), 3 );
        dlg.TransferDataToWindow( m_mdf.m_list_abstraction[ event.GetRow() ],
                strValue);

        m_gridAbstractions->SelectRow( event.GetRow() );

        int row;
        if (wxID_OK == dlg.ShowModal()) {

            wxString newValue;
            dlg.TransferDataFromWindow(newValue);
			
            // If value is not changed do nothing.
            if (newValue == strValue) goto error;
            strValue = newValue;
            m_gridAbstractions->SetCellValue(event.GetRow(), 3, newValue);

            if (-1 == (row =
                    getRegisterGridRow(m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nPage))) {
                wxMessageBox(_("Register corresponding to abstraction not found. Error in mdf-file."));
            }

            switch (m_mdf.m_list_abstraction[ event.GetRow() ]->m_nType) {

                case type_string:

                    // Do visual part
                    if (-1 != row) {

                        // Handle indexed storage
                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            strBuf = getFormattedValue(0);
                            m_gridRegisters->SetCellValue(
                                    row,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    row,
                                    2, *wxRED);

                            strBuf = getFormattedValue(strValue[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    row + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    row + 1,
                                    2, *wxRED);

                        }
                            // Handle linear storage
                        else {

                            for (pos = 0; pos < strValue.Length(); pos++) {

                                strBuf = getFormattedValue(strValue[ pos ]);
                                m_gridRegisters->SetCellValue(
                                        row + pos,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        row + pos,
                                        2, *wxRED);

                                // Cant write more then allocated space
                                if (pos > m_mdf.m_list_abstraction[ event.GetRow() ]->m_nWidth) break;

                            }

                        }
                    }

                    // Update registers
                    m_csw.writeAbstractionString(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            strValue,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                    break;

                case type_boolval:
                {
                    long lv;
                    bool bval;
                    strValue.ToLong(&lv);
                    strValue = strValue.Upper();
                    if (wxNOT_FOUND != strValue.Find(_("TRUE"))) {
                        bval = true;
                    } 
                    else if (wxNOT_FOUND != strValue.Find(_("FALSE"))) {
                        bval = false;
                    } 
                    else if (lv) {
                        bval = true;
                    } 
                    else {
                        bval = false;
                    }

                    // Do visual part
                    if (-1 != row) {

                        strBuf = bval ? _("1") : _("0");
                        m_gridRegisters->SetCellValue(
                                row,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row,
                                2, *wxRED);

                    }

                    // Update registers
                    m_csw.writeAbstractionBool(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            bval,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                }
                    break;

                case type_bitfield:
                {
                    val = 0;

                    // Octet width is the number of bytes needed to store the bits
                    uint8_t octetwidth =
                            m_mdf.m_list_abstraction[ event.GetRow() ]->m_nWidth / 8 +
                            (m_mdf.m_list_abstraction[ event.GetRow() ]->m_nWidth % 8) ? 1 : 0;

                    uint8_t *p;
                    p = new uint8_t[ octetwidth ];
                    memset(p, 0, octetwidth);

                    // Build byte array
                    wxString str = strValue;
                    for (int i = 0; i < m_mdf.m_list_abstraction[ event.GetRow() ]->m_nWidth; i++) {
                        for (int j = 7; j > 0; j--) {
                            if (!str.Length()) break; // Must be digits left
                            if (_("1") == str.Left(1)) {
                                *(p + 1) += (1 << j);
                            }
                            str = str.Right(str.Length() - 1);
                        }
                    }

                    // Do visual part
                    if (-1 != row) {

                        // Handle indexed storage
                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // Index
                            strBuf = getFormattedValue(0);
                            m_gridRegisters->SetCellValue(
                                    row,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    row,
                                    2, *wxRED);

                            // Value
                            strBuf = getFormattedValue(*p);
                            m_gridRegisters->SetCellValue(
                                    row + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    row + 1,
                                    2, *wxRED);
                        }
                            // Handle linear storage
                        else {

                            for (pos = 0; pos < strValue.Length(); pos++) {

                                strBuf = getFormattedValue(*(p + pos));
                                m_gridRegisters->SetCellValue(
                                        row + pos,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        row + pos,
                                        2, *wxRED);

                                // Cant write more then allocated space
                                if (pos > octetwidth) break;
                            }
                        }
                    }

                    // Update registers
                    m_csw.writeAbstractionBitField(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            strValue,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                }
                    break;

                case type_int8_t:
                case type_uint8_t:
                {
                    uint8_t val;
                    val = vscp_readStringValue(strValue);

                    // Do visual part
                    if (-1 != row) {

                        strBuf = getFormattedValue(val);
                        m_gridRegisters->SetCellValue(
                                row,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row,
                                2, *wxRED);

                    }

                    // Update registers
                    m_csw.writeAbstraction8bitinteger(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            val,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                }
                    break;


                case type_int16_t:
                case type_uint16_t:
                {
                    uint16_t val;
                    val = vscp_readStringValue(strValue);

                    // Do visual part
                    if (-1 != row) {

                        strBuf = getFormattedValue(((val >> 8) & 0xff));
                        m_gridRegisters->SetCellValue(
                                row,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row,
                                2, *wxRED);

                        strBuf = getFormattedValue(val & 0xff);
                        m_gridRegisters->SetCellValue(
                                row + 1,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row + 1,
                                2, *wxRED);
                    }

                    // Update registers
                    m_csw.writeAbstraction16bitinteger(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            val,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                }
                    break;

                case type_int32_t:
                case type_uint32_t:
                {
                    long longVal;
                    strValue.ToLong(&longVal);

                    // Do visual part
                    if (-1 != row) {

                        uint8_t *pVal = (uint8_t *) & longVal;

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(pVal[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {


                            for (int i = 0; i < 4; i++) {

                                strBuf = getFormattedValue(pVal[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update registers
                    uint32_t val32 = longVal;
                    m_csw.writeAbstraction32bitinteger(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            val32,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());
                }
                    break;

                case type_int64_t:
                case type_uint64_t:
                {
                    wxLongLong_t longlongVal;
                    strValue.ToLongLong(&longlongVal);

                    // Do visual part
                    if (-1 != row) {

                        uint8_t *pVal = (uint8_t *) & longlongVal;

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(pVal[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {

                            for (int i = 0; i < 8; i++) {

                                strBuf = getFormattedValue(pVal[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update register
                    uint64_t val64 = longlongVal;
                    m_csw.writeAbstraction64bitinteger(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            val64,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());

                }
                    break;


                case type_float:
                {
                    double doubleVal;
                    strValue.ToDouble(&doubleVal);
                    float floatVal = doubleVal;

                    uint8_t *pVal = (uint8_t *) & floatVal;
                    // Do visual part
                    if (-1 != row) {

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(pVal[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {


                            for (int i = 0; i < 4; i++) {

                                strBuf = getFormattedValue(pVal[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update register
                    m_csw.writeAbstractionFloat(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            floatVal,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());

                }
                break;

                case type_double:
                {
                    double doubleVal;
                    strValue.ToDouble(&doubleVal);

                    uint8_t *pVal = (uint8_t *) & doubleVal;
                    // Do visual part
                    if (-1 != row) {

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(pVal[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {


                            for (int i = 0; i < 8; i++) {

                                strBuf = getFormattedValue(pVal[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update register
                    m_csw.writetAbstractionDouble(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            doubleVal,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());

                }
                    break;

                case type_date:
                {
                    // Dates are stored as YYYY-MM-DD
                    // byte 0 - MSB of year
                    // byte 1 - LSB of year
                    // byte 2 - Month (1-12)
                    // byte 3 - Date (0-31)

                    uint8_t buf[ 4 ];
                    wxDateTime date;

                    date.ParseDate(strValue);

                    uint16_t year = date.GetYear();
                    buf[ 0 ] = ((year >> 8) & 0xff);
                    buf[ 1 ] = (year & 0xff);
                    buf[ 2 ] = date.GetMonth();
                    buf[ 3 ] = date.GetDay();

                    // Do visual part
                    if (-1 != row) {

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(buf[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {

                            for (int i = 0; i < 4; i++) {

                                strBuf = getFormattedValue(buf[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update register
                    m_csw.writeAbstractionDate(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            date,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());

                }
                break;

                case type_time:
                {
                    // Dates are stored as HH:MM:SS
                    // byte 0 - Hours
                    // byte 1 - Minutes
                    // byte 2 - seconds
                    uint8_t buf[ 3 ];
                    wxDateTime time;
                    time.ParseTime(strValue);

                    buf[ 0 ] = time.GetHour();
                    buf[ 1 ] = time.GetMinute();
                    buf[ 2 ] = time.GetSecond();

                    // Do visual part
                    if (-1 != row) {

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(buf[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {


                            for (int i = 0; i < 3; i++) {

                                strBuf = getFormattedValue(buf[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }
                        }
                    }

                    // Update register
                    m_csw.writeAbstractionTime(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            time,
                            &m_ifguid,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue());

                }
                break;

                case type_guid:
                {
                    cguid guid;
                    guid.getFromString(strValue);

                    const uint8_t *p = guid.getGUID();

                    // Do visual part
                    if (-1 != row) {

                        if (m_mdf.m_list_abstraction[ event.GetRow() ]->m_bIndexed) {

                            // index
                            strBuf = _("0");
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset,
                                    2, *wxRED);

                            // value
                            strBuf = getFormattedValue(p[ 0 ]);
                            m_gridRegisters->SetCellValue(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + 1,
                                    2, *wxRED);

                        } 
                        else {

                            for (int i = 0; i < 3; i++) {

                                strBuf = getFormattedValue(p[ i ]);
                                m_gridRegisters->SetCellValue(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2,
                                        strBuf);

                                m_gridRegisters->SetCellTextColour(
                                        m_mdf.m_list_abstraction[ event.GetRow() ]->m_nOffset + i,
                                        2, *wxRED);
                            }

                        }
                    }

                    // Update register
                    m_csw.writeAbstractionGUID(this,
                            vscp_readStringValue(m_comboNodeID->GetValue()),
                            m_mdf.m_list_abstraction[ event.GetRow() ],
                            guid);
                }
                break;

                case type_unknown:
                default:
                    break;
            } // case

            // Update registers
            OnButtonUpdateClick(event);

            m_gridAbstractions->SelectRow( event.GetRow() );

        }

    } 
    else if (ID_GRID_DM == event.GetId()) {

        wxString str;
        DialogEditLevelIDMrow dlg(this);

        m_gridDM->SelectRow( event.GetRow() );

        // Originating address
        str = m_gridDM->GetCellValue(event.GetRow(), 0);
        dlg.m_oaddr->ChangeValue(str);

        // flags
        uint8_t flags = vscp_readStringValue(m_gridDM->GetCellValue(event.GetRow(), 1));
        (flags & 0x80) ? dlg.m_chkEnableDMRow->SetValue(true) : dlg.m_chkEnableDMRow->SetValue(false);
        (flags & 0x40) ? dlg.m_chkCheckOAddr->SetValue(true) : dlg.m_chkCheckOAddr->SetValue(false);
        (flags & 0x20) ? dlg.m_chkHardOAddr->SetValue(true) : dlg.m_chkHardOAddr->SetValue(false);
        (flags & 0x10) ? dlg.m_chkMatchZone->SetValue(true) : dlg.m_chkMatchZone->SetValue(false);
        (flags & 0x08) ? dlg.m_chkMatchSubzone->SetValue(true) : dlg.m_chkMatchSubzone->SetValue(false);

        // Class Mask
        reg = vscp_readStringValue(m_gridDM->GetCellValue(event.GetRow(), 2)) +
                ((flags & 0x02) << 9);
        str = wxString::Format(_("%d"), reg);
        dlg.m_classMask->ChangeValue(str);

        // Class Filter
        reg = vscp_readStringValue(m_gridDM->GetCellValue(event.GetRow(), 3)) +
                ((flags & 0x01) << 9);
        str = wxString::Format(_("%d"), reg);
        dlg.m_classFilter->ChangeValue(str);

        // Type Mask
        str = m_gridDM->GetCellValue(event.GetRow(), 4);
        dlg.m_typeMask->ChangeValue(str);

        // Type Filter
        str = m_gridDM->GetCellValue(event.GetRow(), 5);
        dlg.m_typeFilter->ChangeValue(str);

        // Action
        dlg.m_comboAction->Clear();
        dlg.m_comboAction->Append(_("No operation"));
        dlg.m_comboAction->SetClientData(0, 0);
        dlg.m_comboAction->SetSelection(0);

        reg = vscp_readStringValue(m_gridDM->GetCellValue(event.GetRow(), 6));

        MDF_ACTION_LIST::iterator iter;
        for (iter = m_mdf.m_dmInfo.m_list_action.begin();
                iter != m_mdf.m_dmInfo.m_list_action.end(); ++iter) {
            CMDF_Action *action = *iter;
            int idx = dlg.m_comboAction->Append(action->m_strName);
            dlg.m_comboAction->SetClientData(idx, (void *)action->m_nCode); // Yes - pointer conversion
            if (reg == action->m_nCode) {
                dlg.m_comboAction->SetSelection(idx);
            }
        }

        // Action parameter
        str = m_gridDM->GetCellValue(event.GetRow(), 7);
        dlg.m_actionParam->ChangeValue(str);

        // Show the dialog
        if (wxID_OK == dlg.ShowModal()) {

            // OK Clicked 

            wxString strBuf;

            // O-addr
            strBuf.Printf(_("0x%02x"), (uint8_t)vscp_readStringValue(dlg.m_oaddr->GetValue()));
            m_gridRegisters->SetCellValue(m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // class mask
            uint16_t class_mask = vscp_readStringValue(dlg.m_classMask->GetValue());
            strBuf.Printf(_("0x%02x"), class_mask & 0xff);
            m_gridRegisters->SetCellValue(2 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // class filter
            uint16_t class_filter = vscp_readStringValue(dlg.m_classFilter->GetValue());
            strBuf.Printf(_("0x%02x"), class_filter & 0xff);
            m_gridRegisters->SetCellValue(3 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // type mask
            strBuf.Printf(_("0x%02x"), vscp_readStringValue(dlg.m_typeMask->GetValue()));
            m_gridRegisters->SetCellValue(4 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // type filter
            strBuf.Printf(_("0x%02x"), vscp_readStringValue(dlg.m_typeFilter->GetValue()));
            m_gridRegisters->SetCellValue(5 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // flags
            uint8_t flags = 0;

            if (0x100 & class_mask) flags |= 0x02;
            if (0x100 & class_filter) flags |= 0x01;
            if (dlg.m_chkEnableDMRow->GetValue()) flags |= 0x80;
            if (dlg.m_chkCheckOAddr->GetValue()) flags |= 0x40;
            if (dlg.m_chkHardOAddr->GetValue()) flags |= 0x20;
            if (dlg.m_chkMatchZone->GetValue()) flags |= 0x10;
            if (dlg.m_chkMatchSubzone->GetValue()) flags |= 0x08;
            strBuf.Printf(_("0x%02x"), flags);
            m_gridRegisters->SetCellValue(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // Action
            int idx = 0;
            if (wxNOT_FOUND != (idx = dlg.m_comboAction->GetSelection())) {
                uint32_t data = (uintptr_t) dlg.m_comboAction->GetClientData(idx);
                strBuf.Printf(_("0x%02x"), data);
            } else {
                strBuf.Printf(_("0x%02x"), 0);
            }
            m_gridRegisters->SetCellValue(6 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // Action Parameter
            strBuf.Printf(_("0x%02x"), (uint8_t)vscp_readStringValue(dlg.m_actionParam->GetValue()));
            m_gridRegisters->SetCellValue(7 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            int row = getRegisterGridRow(m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    m_mdf.m_dmInfo.m_nStartPage);

            // Visual indication
            if (-1 != row) {

                for (int i = 0; i < m_mdf.m_dmInfo.m_nRowSize; i++) {

                    if (m_mdf.m_dmInfo.m_bIndexed) {

                        // Index
                        strBuf = getFormattedValue(0);
                        m_gridRegisters->SetCellValue(
                                row,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row,
                                2, *wxRED);

                        // Value
                        strBuf = getFormattedValue(
                                vscp_readStringValue(dlg.m_oaddr->GetValue()));
                        m_gridRegisters->SetCellValue(
                                row + 1,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row + 1,
                                2, *wxRED);

                    } 
                    else {

                        for (int i = 0; i < m_mdf.m_dmInfo.m_nRowSize; i++) {

                            // Index
                            strBuf = getFormattedValue(vscp_readStringValue(
                                    m_gridRegisters->GetCellValue(m_mdf.m_dmInfo.m_nStartOffset +
                                    m_mdf.m_dmInfo.m_nRowSize *
                                    event.GetRow() + i, 2)));
                            m_gridRegisters->SetCellValue(
                                    row + i,
                                    2,
                                    strBuf);

                            m_gridRegisters->SetCellTextColour(
                                    row + i,
                                    2, *wxRED);

                        }

                    }
                }
            } 
            else {
                wxMessageBox(_("It's a problem with the MDF for this device. DM rows not in register definition."));
            }

            // Update registers
            OnButtonUpdateClick(event);

            m_gridDM->SelectRow( event.GetRow() );
        }

    } 
    else if (ID_GRID_ABSTRACTIONS == event.GetId()) {

        // Update the DM grid    
        updateDmGrid();

        // Update abstractions
        updateAbstractionGrid();

    }

error:

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateAbstractionGrid
//

void frmDeviceConfig::updateAbstractionGrid(void) 
{
    wxString strBuf;

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
    wxFont fontBold = defaultFont;
    fontBold.SetStyle(wxFONTSTYLE_NORMAL);
    fontBold.SetWeight(wxFONTWEIGHT_BOLD);

    // Delete grid rows if there are any
    m_gridAbstractions->ClearGrid();
    if (m_gridAbstractions->GetNumberRows()) {
        m_gridAbstractions->DeleteRows(0, m_gridAbstractions->GetNumberRows());
    }

    MDF_ABSTRACTION_LIST::iterator iter;
    for (iter = m_mdf.m_list_abstraction.begin();
            iter != m_mdf.m_list_abstraction.end(); ++iter) {

        CMDF_Abstraction *abstraction = *iter;

        // Add a row
        m_gridAbstractions->AppendRows(1);

        // Name
        m_gridAbstractions->SetCellValue( m_gridAbstractions->GetNumberRows() - 1,
                                            0,
                                            _(" ") + abstraction->m_strName );
        m_gridAbstractions->SetCellAlignment( wxALIGN_LEFT,
                                                m_gridAbstractions->GetNumberRows() - 1,
                                                0 );
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 0);


        // Type
        m_gridAbstractions->SetCellValue( m_gridAbstractions->GetNumberRows() - 1,
                                            1,
                                            abstraction->m_strName );
        m_gridAbstractions->SetCellAlignment( wxALIGN_CENTER,
                                                m_gridAbstractions->GetNumberRows() - 1,
                                                1 );
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 1);


        // Access
        strBuf = _("");
        if (abstraction->m_nAccess & MDF_ACCESS_READ) {
            strBuf = _("r");
        }

        if (abstraction->m_nAccess & MDF_ACCESS_WRITE) {
            strBuf += _("w");
        }

        m_gridAbstractions->SetCellValue( m_gridAbstractions->GetNumberRows() - 1,
                                            2,
                                            strBuf );
        m_gridAbstractions->SetCellAlignment( wxALIGN_CENTER,
                                                m_gridAbstractions->GetNumberRows() - 1,
                                                2 );
        m_gridAbstractions->SetReadOnly( m_gridAbstractions->GetNumberRows() - 1, 2);

        
        // Description
        m_gridAbstractions->SetCellValue( m_gridAbstractions->GetNumberRows() - 1,
                                            4,
                                            _(" ") + abstraction->m_strDescription);

        m_gridAbstractions->SetCellAlignment( wxALIGN_LEFT,
                                                m_gridAbstractions->GetNumberRows() - 1,
                                                4 );
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 4);
        m_gridRegisters->AutoSizeRow(m_gridAbstractions->GetNumberRows() - 1);

        wxString strValue;
        wxString strType;
        int pos = 0; // Current character

        strType = abstraction->getAbstractionValueType();
        m_userRegisters.getAbstractionValueAsString( abstraction, strValue );
        /*strValue = m_csw.getAbstractionValueAsString( this,
                                                        m_stdRegisters.getNickname(),
                                                        abstraction,
                                                        &m_ifguid,
                                                        &destGUID,
                                                        NULL,
                                                        m_bLevel2->GetValue());*/
        // Value
        m_gridAbstractions->SetCellValue( m_gridAbstractions->GetNumberRows() - 1,
                                            3,
                                            strValue );

        m_gridAbstractions->SetCellAlignment(wxALIGN_CENTER,
                m_gridAbstractions->GetNumberRows() - 1,
                3);
        m_gridAbstractions->SetCellFont(m_gridAbstractions->GetNumberRows() - 1, 3, fontBold);
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 3);

        // Type
        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                1,
                strType);

        // Make all parts of the row visible
        m_gridAbstractions->AutoSizeRow(m_gridAbstractions->GetNumberRows() - 1);

    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateDmGrid
//

void frmDeviceConfig::updateDmGrid(void) 
{
    wxString strBuf;
    uint8_t buf[ 512 ];
    uint8_t *prow = buf;

    cguid destGUID;
    destGUID.getFromString( m_comboNodeID->GetValue() );

    m_gridDM->ClearGrid();
    if ( m_gridDM->GetNumberRows() ) m_gridDM->DeleteRows( 0, m_gridDM->GetNumberRows() );

    for ( int i=0; i < m_mdf.m_dmInfo.m_nRowCount; i++ ) {

        // Add a row
        if ( !m_gridDM->AppendRows(1) ) continue;

        // O-addr
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 0, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 0);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 0);

        // Flags
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                1 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 1, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 1);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 1);

        // Class Mask
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                2 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 2, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 2);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 2);

        // Class Filter
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                3 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 3, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 3);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 3);

        // Type Mask
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                4 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 4, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 4);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 4);

        // Type Filter
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                5 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                 m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 5, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 5);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 5);

        // Action
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                6 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                 m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 6, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 6);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 6);

        // Action Parameter
        strBuf = getFormattedValue( m_userRegisters.getValue( m_mdf.m_dmInfo.m_nStartPage, 
                                                                7 + m_mdf.m_dmInfo.m_nStartOffset + 
                                                                m_mdf.m_dmInfo.m_nRowSize * i ) );
        m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 7, strBuf);
        m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 7);
        m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 7);

        m_gridDM->Update();

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateDmGridConditional
//

void frmDeviceConfig::updateDmGridConditional(uint16_t reg, uint32_t page) 
{
    int row = getRegisterGridRow(reg, page);
    if (-1 == row) return;

    // Must write to data part for a change
    // to take place
    if ( ( reg >= ( m_mdf.m_dmInfo.m_nStartOffset ) ) &&
                ( reg < ( m_mdf.m_dmInfo.m_nStartOffset + ( m_mdf.m_dmInfo.m_nRowCount * m_mdf.m_dmInfo.m_nRowSize ) ) ) &&
                ( page == m_mdf.m_dmInfo.m_nStartPage ) ) {

        updateDmGrid();

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateAbstractionGridConditional
//

void frmDeviceConfig::updateAbstractionGridConditional(uint16_t reg, uint32_t page) 
{
    uint16_t rowAbstraction = 0;

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    int row = getRegisterGridRow(reg, page);
    if (-1 == row) return;

    // Iterate through abstractions to se if this register is used
    MDF_ABSTRACTION_LIST::iterator iter;
    for (iter = m_mdf.m_list_abstraction.begin();
            iter != m_mdf.m_list_abstraction.end(); ++iter) {

        CMDF_Abstraction *abstraction = *iter;
        bool bIndexed = abstraction->m_bIndexed;
        uint32_t regAbstraction = abstraction->m_nOffset;
        uint16_t pageAbstraction = abstraction->m_nPage;

        uint32_t width;

        if (abstraction->m_nType == type_string) {
            width = abstraction->m_nWidth;
        } 
        else if (abstraction->m_nType == type_bitfield) {
            width = abstraction->m_nWidth;
        } 
        else if (abstraction->m_nType == type_boolval) {
            width = 1;
        } 
        else if (abstraction->m_nType == type_int8_t) {
            width = 1;
        } 
        else if (abstraction->m_nType == type_uint8_t) {
            width = 1;
        } 
        else if (abstraction->m_nType == type_int16_t) {
            width = 2;
        } 
        else if (abstraction->m_nType == type_uint16_t) {
            width = 2;
        } 
        else if (abstraction->m_nType == type_int32_t) {
            width = 4;
        } 
        else if (abstraction->m_nType == type_uint32_t) {
            width = 4;
        } 
        else if (abstraction->m_nType == type_int64_t) {
            width = 8;
        } 
        else if (abstraction->m_nType == type_uint64_t) {
            width = 8;
        } 
        else if (abstraction->m_nType == type_double) {
            width = 8;
        } 
        else if (abstraction->m_nType == type_float) {
            width = 4;
        } 
        else if (abstraction->m_nType == type_date) {
            width = 4;
        } 
        else if (abstraction->m_nType == type_time) {
            width = 3;
        } 
        else if (abstraction->m_nType == type_guid) {
            width = 16;
        } 
        else {
            width = 0;
        }

        if ((reg >= regAbstraction) &&
                    (reg < (regAbstraction + width)) &&
                    (page == pageAbstraction)) {

            updateAbstractionGrid();
        }

        rowAbstraction++;

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dmEnableSelectedRow
//

void frmDeviceConfig::dmEnableSelectedRow(wxCommandEvent& event) 
{
    wxString strBuf;

    // Select the row
    m_gridDM->SelectRow(m_lastLeftClickRow);

    wxArrayInt selrows = m_gridDM->GetSelectedRows();

    if (selrows.GetCount()) {

        for (int i = selrows.GetCount() - 1; i >= 0; i--) {

            uint8_t flags = vscp_readStringValue(m_gridDM->GetCellValue(selrows[i], 2));
            flags |= 0x80; // Set enable bit
            strBuf = getFormattedValue(flags);
            m_gridRegisters->SetCellValue(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * selrows[i],
                    2,
                    strBuf);

            m_gridRegisters->SetCellTextColour(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * selrows[i],
                    2,
                    *wxRED);

            // Update registers
            OnButtonUpdateClick(event);


        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dmDisableSelectedRow
//

void frmDeviceConfig::dmDisableSelectedRow(wxCommandEvent& event) 
{
    wxString strBuf;

    // Select the row
    m_gridDM->SelectRow(m_lastLeftClickRow);

    wxArrayInt selrows = m_gridDM->GetSelectedRows();

    if (selrows.GetCount()) {

        for (int i = selrows.GetCount() - 1; i >= 0; i--) {

            uint8_t flags = vscp_readStringValue(m_gridDM->GetCellValue(selrows[i], 2));
            flags &= 0x7f; // Reset enable bit
            strBuf = getFormattedValue(flags);
            m_gridRegisters->SetCellValue(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * selrows[i],
                    2,
                    strBuf);

            m_gridRegisters->SetCellTextColour(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * selrows[i],
                    2,
                    *wxRED);

            // Update registers
            OnButtonUpdateClick(event);

        }

    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dmRowWizard
//

void frmDeviceConfig::dmRowWizard(wxCommandEvent& event) 
{
    wxMessageBox(_("Still needs to be coded..."));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveRegistersClick
//

void frmDeviceConfig::OnMenuitemSaveRegistersClick( wxCommandEvent& event ) 
{
    wxString str;
    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg( this,
                        _("Choose file to save register set to "),
                        wxStandardPaths::Get().GetUserDataDir(),
                        _("registerset"),
                        _("Register set (*.reg)|*.reg|XML Files (*.xml)|*.xml|All files (*.*)|*.*"),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

    if ( wxID_OK == dlg.ShowModal() ) {

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file for configuration."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // Transmission set start
        pFileStream->Write("<registerset>\n", strlen("<registerset>\n"));

        // We write the full register range including standard registers
        // so we can use the dump for debug purposes. No need to read
        // back standard registersor a standard user.
        for (int i = 0; i < 256; i++) {

            pFileStream->Write("<register id='", strlen("<register id='"));
            str.Printf(_("%d"), i);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("'>\n", strlen("'>\n"));

            pFileStream->Write("<value>", strlen("<value>"));
            str = m_gridRegisters->GetCellValue(i, 2);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</value>\n", strlen("</value>\n"));

            pFileStream->Write("<description>", strlen("<description>"));
            str = m_gridRegisters->GetCellValue(i, 3);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</description>\n", strlen("</description>\n"));

            pFileStream->Write("</register>\n", strlen("</register>\n"));

        }

        // Transmission set end
        pFileStream->Write("</registerset>\n", strlen("</registerset>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveSelectedRegistersClick
//

void frmDeviceConfig::OnMenuitemSaveSelectedRegistersClick(wxCommandEvent& event) 
{
    wxString str;
    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save register set to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("registerset"),
            _("Register set (*.reg)|*.reg|XML Files (*.xml)|*.xml|All files (*.*)|*.*"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (wxID_OK == dlg.ShowModal()) {

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file for configuration."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // Transmission set start
        pFileStream->Write("<registerset>\n", strlen("<registerset>\n"));

        // We write the full register range including standard registers
        // so we can use the dump for debug purposes. No need to read
        // back standard registersor a standard user.
        for (int i = 0; i < 256; i++) {

            // If row is not selected ignore it
            if (!m_gridRegisters->IsInSelection(i, 2)) continue;

            pFileStream->Write("<register id='", strlen("<register id='"));
            str.Printf(_("%d"), i);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("'>\n", strlen("'>\n"));

            pFileStream->Write("<value>", strlen("<value>"));
            str = m_gridRegisters->GetCellValue(i, 2);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</value>\n", strlen("</value>\n"));

            pFileStream->Write("<description>", strlen("<description>"));
            str = m_gridRegisters->GetCellValue(i, 3);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</description>\n", strlen("</description>\n"));

            pFileStream->Write("</register>\n", strlen("</register>\n"));

        }

        // Transmission set end
        pFileStream->Write("</registerset>\n", strlen("</registerset>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemLoadRegistersClick
//

void frmDeviceConfig::OnMenuitemLoadRegistersClick(wxCommandEvent& event) 
{
    //wxStandardPaths stdpaths;
    wxXmlDocument doc;

    wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
    wxFont fontBold = defaultFont;
    fontBold.SetStyle(wxFONTSTYLE_NORMAL);
    fontBold.SetWeight(wxFONTWEIGHT_BOLD);

    // First find a path to save register set data to
    wxFileDialog dlg( this,
                        _("Choose file to load register set from "),
                        wxStandardPaths::Get().GetUserDataDir(),
                        _("registerset"),
                        _("Register set Files (*.reg)|*.reg|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        if (!doc.Load(dlg.GetPath())) {
            wxMessageDialog(this,
                    _("Unable to load file!"),
                    _("Load error"),
                    wxICON_ERROR);
            return;
        }

        bool bEmpty = false;
        int nRows = m_gridRegisters->GetNumberRows();
        if ( 0 == nRows ) {
            bEmpty=true;
            //fillStandardRegisters();
        }

        // start processing the XML file
        if (doc.GetRoot()->GetName() != wxT("registerset")) {
            return;
        }

        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if (child->GetName() == wxT("register")) {

                long reg;
                uint8_t value;
                wxString strid;
                wxString strValue;
                wxString strSaveValue;
                wxString strDescription;

                strid = child->GetAttribute(wxT("id"), wxT("-1"));
                     
                reg = vscp_readStringValue(strid);
                if (-1 == reg) continue;

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {

                    if (subchild->GetName() == wxT("value")) {
                        wxString str = subchild->GetNodeContent();
                        value = vscp_readStringValue(str);
                        strValue.Printf(_("0x%02lx"), value);
                    }
                    else if (subchild->GetName() == wxT("description")) {
                        strDescription = subchild->GetNodeContent();
                    }

                    subchild = subchild->GetNext();
                }

                
                if ( !bEmpty ) {
                    strSaveValue = m_gridRegisters->GetCellValue(reg, 2);
                }
                else {
                    strSaveValue = _("");
                }
                       
                if ( !bEmpty ) {
                    m_gridRegisters->SetCellValue(reg, 2, strValue);
                    if (strValue != strSaveValue) {
                        m_gridRegisters->SetCellTextColour(reg, 2, *wxRED);
                    }
                }
                else {
                    wxString strBuf;

                    m_gridRegisters->AppendRows(1);
                                
                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strValue);
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 1, _("rw"));
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 1);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 1);

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                                                                2,
                                                                strValue);
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 2);

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, strDescription );
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 3, fontBold);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 3);
    
                    // Make all parts of the row visible
                    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

                    for (int i = 0; i < 4; i++) {
                        m_gridRegisters->SetCellBackgroundColour(m_gridRegisters->GetNumberRows() - 1, i, wxColour(0xff, 0xff, 0xd2));
                    }

                    // Yes value is changed as it is new
                    m_gridRegisters->SetCellTextColour(reg, 2, *wxRED);
                }
            }

            child = child->GetNext();

        }

    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getRegFromCell
//

uint32_t frmDeviceConfig::getRegFromCell(int row) 
{
    wxString token1, token2;
    wxString str = m_gridRegisters->GetCellValue(row, 0);

    wxStringTokenizer tkz(str, _(":"));
    if (tkz.HasMoreTokens()) {
        token1 = tkz.GetNextToken(); // Page
    } 
    else {
        token1 = str;
    }

    // For a standard register this one will fail
    if (tkz.HasMoreTokens()) {
        token2 = tkz.GetNextToken(); // Register
    } 
    else {
        token2 = token1;
    }

    return vscp_readStringValue(_("0x") + token2.Trim());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getPageFromCell
//

uint16_t frmDeviceConfig::getPageFromCell(int row) 
{
    wxString str = m_gridRegisters->GetCellValue(row, 0);
    if (wxNOT_FOUND != str.Find(_(":"))) {
        return vscp_readStringValue(_("0x") + m_gridRegisters->GetCellValue(row, 0).Trim());
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getFormattedValue
//

wxString frmDeviceConfig::getFormattedValue(uint8_t val) 
{
    if (VSCP_DEVCONFIG_NUMBERBASE_HEX ==
            g_Config.m_Numberbase) {
        return wxString::Format(_("0x%02X"), val);
    } 
    else {
        return wxString::Format(_("%d"), val);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getRegisterGridRow
//

int frmDeviceConfig::getRegisterGridRow(uint32_t reg, uint16_t page) 
{
    for (int i = 0; i < m_gridRegisters->GetRows(); i++) {
        if ((page == getPageFromCell(i)) &&
                (reg == getRegFromCell(i))) {
            return i;
        }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

bool frmDeviceConfig::fetchIterfaceGUID(void) 
{
    wxString str;
    wxArrayString ifarray;

    if (!m_csw.isOpen()) {
        wxMessageBox(_("TCP/IP connection to daemon must be open."));
        return false;
    }

    if (USE_TCPIP_INTERFACE != m_csw.getDeviceType()) {
        wxMessageBox(_("Interfaces can only be fetched from the VSCP daemon."));
        return false;
    }

    // Get the interface list
    while ( true ) {

        if ( VSCP_ERROR_SUCCESS ==
             m_csw.getTcpIpInterface()->doCmdInterfaceList( ifarray ) ) {

            if ( ifarray.Count() ) {

                for ( unsigned int i = 0; i < ifarray.Count(); i++ ) {

                    wxStringTokenizer tkz( ifarray[ i ], _( "," ) );
                    wxString strOrdinal = tkz.GetNextToken();
                    wxString strType = tkz.GetNextToken();
                    wxString strIfGUID = tkz.GetNextToken();
                    wxString strDescription = tkz.GetNextToken();

                    int pos;
                    wxString strName;
                    if ( wxNOT_FOUND != ( pos = strDescription.Find( _( " " ) ) ) ) {
                        strName = strDescription.Left( pos );
                        strName.Trim();
                    }

                    if ( strName.Upper() == m_vscpif.m_strInterfaceName.Upper() ) {

                        // Save the name
                        //m_vscpif.m_strInterfaceName. = strName;

                        // Save interface GUID;
                        m_ifguid.getFromString( strIfGUID );

                        return true;
                    }

                }

            }
            else {
                if ( wxYES != wxMessageBox( _( "No interfaces found. Try to find again?" ), 
                                                _( "Fetching interfaces" ), 
                                                wxYES_NO ) ) {
                    break;
                }
            }
        }
        else {
            if ( wxYES != wxMessageBox( _( "Unable to get interface list from VSCP daemon. Try to get again?" ), 
                                            _( "Fetching interfaces" ), 
                                            wxYES_NO ) ) {
                break;
            }
        }

    }

    return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAddGuidsClick
//

void frmDeviceConfig::OnMenuitemAddGuidsClick(wxCommandEvent& event) 
{
    wxString str = ::wxGetTextFromUser(_("Enter full GUID to add :"),
            _("VSCP Works"));
    m_guidarray.Add(str);
    m_comboNodeID->Append(str);

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveGuidsClick
//

void frmDeviceConfig::OnMenuitemSaveGuidsClick(wxCommandEvent& event) 
{
    wxString str;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save GUID's to"),
            wxStandardPaths::Get().GetUserDataDir(),
            _("guidset"),
            _("GUID set Files (*.guid)|*.guid|All files (*.*)|*.*"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (wxID_OK == dlg.ShowModal()) {

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file for configuration."));
            return;
        }

        for (unsigned int i = 0; i < m_comboNodeID->GetCount(); i++) {
            wxString str = m_comboNodeID->GetString(i) + _("\r\n");
            pFileStream->Write(str.mb_str(wxConvUTF8), str.Length());
        }

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemLoadGuidsClick
//

void frmDeviceConfig::OnMenuitemLoadGuidsClick(wxCommandEvent& event) 
{
    wxXmlDocument doc;

    // First find a path to save register set data to
    wxFileDialog dlg( this,
                        _("Choose file to load register set from "),
                        wxStandardPaths::Get().GetUserDataDir(),
                        _("guidset"),
                        _("GUID set Files (*.guid)|*.guid|All files (*.*)|*.*"));

    if (wxID_OK == dlg.ShowModal()) {

        m_comboNodeID->Clear();

        wxFFileInputStream *pFileStream = new wxFFileInputStream(dlg.GetPath());
        if ( NULL == pFileStream ) {
            wxMessageBox(_("Failed to open GUID set file."));
            return;
        }

        char buf[ 2048 ];
        wxString str;

        while ( !pFileStream->Eof() ) {

            memset(buf, 0, sizeof ( buf));
            pFileStream->Read(buf, sizeof ( buf) - 1);
            str += wxString::FromAscii(buf);

        }

        wxStringTokenizer tkz(str, _("\r\n"));
        wxString strToken;
        while (tkz.HasMoreTokens()) {
            strToken = tkz.GetNextToken();
            m_comboNodeID->Append(strToken);
        }

        if (m_comboNodeID->GetCount()) m_comboNodeID->Select(0);
    }

    event.Skip(false);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnHtmlwindowCellClicked
//

void frmDeviceConfig::OnHtmlwindowCellClicked(wxHtmlLinkEvent& event) 
{
    if (event.GetLinkInfo().GetHref().StartsWith(_("http://"))) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventStatusChange
//

void frmDeviceConfig::eventStatusChange( wxCommandEvent &evt )
{
    switch (evt.GetInt() ) { 

        case STATUSBAR_STATUS_RIGHT:
            m_pitemStatusBar->SetStatusText( evt.GetString(), STATUSBAR_STATUS_RIGHT );   
            break;

        case STATUSBAR_STATUS_LEFT:
        default:
            m_pitemStatusBar->SetStatusText( evt.GetString(), STATUSBAR_STATUS_LEFT );   
            break;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventStatusChange
//

void frmDeviceConfig::OnResizeWindow( wxSizeEvent& event ) 
{
    wxSize sz = wxWindow::GetClientSize();
    //m_StatusWnd->SetPosition( wxPoint(0,0) );
    event.Skip();
}

