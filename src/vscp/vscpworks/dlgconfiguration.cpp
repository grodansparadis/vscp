/////////////////////////////////////////////////////////////////////////////
// Name:        dlgconfiguration.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 18:30:18 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2012 Ake Hedman, Grodnas PAradis AB, <ake@grodansparadis.com>
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
//  Grodans Paradis AB at info@Grodans Paradis AB.se, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgconfiguration.h"
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
#include "wx/bookctrl.h"
////@end includes

#include <wx/colordlg.h>
#include <wx/imaglist.h>

#include "vscpworks.h"
#include "dlgconfiguration.h"

////@begin XPM images
/* XPM */
static char *copy_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 4 1",
"  c None",
". c Black",
"X c Gray100",
"o c #000080",
/* pixels */
"                ",
" ......         ",
" .XXXX..        ",
" .XXXX.X.       ",
" .X..X.oooooo   ",
" .XXXXXoXXXXoo  ",
" .X....oXXXXoXo ",
" .XXXXXoX..Xoooo",
" .X....oXXXXXXXo",
" .XXXXXoX.....Xo",
" ......oXXXXXXXo",
"       oX.....Xo",
"       oXXXXXXXo",
"       ooooooooo",
"                "
};

////@end XPM images

extern appConfiguration g_Config;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgConfiguration, wxPropertySheetDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration event table definition
//

BEGIN_EVENT_TABLE( dlgConfiguration, wxPropertySheetDialog )

////@begin dlgConfiguration event table entries
  EVT_BUTTON( ID_BUTTON4, dlgConfiguration::OnButtonSetRxGridTextColourClick )

  EVT_BUTTON( ID_BUTTON5, dlgConfiguration::SetRxGridBackgroundColourClick )

  EVT_BUTTON( ID_BUTTON6, dlgConfiguration::OnButtonSetTxGridTextColourClick )

  EVT_BUTTON( ID_BUTTON7, dlgConfiguration::OnButtonSetTxGridBackgroundColourClick )

////@end dlgConfiguration event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration constructors
//

dlgConfiguration::dlgConfiguration()
{
    Init();
}

dlgConfiguration::dlgConfiguration( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration creator
//

bool dlgConfiguration::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    ////@begin dlgConfiguration creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  SetSheetStyle(wxPROPSHEET_DEFAULT);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  LayoutDialog();
  Centre();
    ////@end dlgConfiguration creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration destructor
//

dlgConfiguration::~dlgConfiguration()
{
    ////@begin dlgConfiguration destruction
    ////@end dlgConfiguration destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgConfiguration::Init()
{
    ////@begin dlgConfiguration member initialisation
  m_labelLogFile = NULL;
  m_checkEnableLogging = NULL;
  m_comboLogLevel = NULL;
  m_checkConfirmDeletes = NULL;
  m_comboNumericalBase = NULL;
  m_maxRetries = NULL;
  m_readTimeout = NULL;
  m_ChkAutoScroll = NULL;
  m_chkPyjamasLook = NULL;
  m_chkUseSymbols = NULL;
  m_editRxForeGroundColour = NULL;
  m_editRxBackGroundColour = NULL;
  m_editTxForeGroundColour = NULL;
  m_editTxBackGroundColour = NULL;
    ////@end dlgConfiguration member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgConfiguration
//

void dlgConfiguration::CreateControls()
{    
    ////@begin dlgConfiguration content construction
  dlgConfiguration* itemPropertySheetDialog1 = this;

  wxImageList* itemPropertySheetDialog1ImageList = new wxImageList(16, 16, true, 5);
  {
    wxIcon itemPropertySheetDialog1Icon0(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon0);
    wxIcon itemPropertySheetDialog1Icon1(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon1);
    wxIcon itemPropertySheetDialog1Icon2(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon2);
    wxIcon itemPropertySheetDialog1Icon3(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon3);
    wxIcon itemPropertySheetDialog1Icon4(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon4);
  }
  GetBookCtrl()->AssignImageList(itemPropertySheetDialog1ImageList);

  wxPanel* itemPanel2 = new wxPanel;
  itemPanel2->Create( GetBookCtrl(), ID_PANEL_GENERAL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  itemPanel2->SetName(_T("general"));
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer3->Add(itemGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText5 = new wxStaticText;
  itemStaticText5->Create( itemPanel2, wxID_STATIC, _("Logfile :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_labelLogFile = new wxStaticText;
  m_labelLogFile->Create( itemPanel2, wxID_STATIC, _("---"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer6->Add(m_labelLogFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemPanel2, wxID_STATIC, _("Enable logging :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_checkEnableLogging = new wxCheckBox;
  m_checkEnableLogging->Create( itemPanel2, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_checkEnableLogging->SetValue(false);
  itemBoxSizer9->Add(m_checkEnableLogging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemPanel2, wxID_STATIC, _("Log Level :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
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

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemPanel2, wxID_STATIC, _("Confirm deletes :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_checkConfirmDeletes = new wxCheckBox;
  m_checkConfirmDeletes->Create( itemPanel2, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_checkConfirmDeletes->SetValue(false);
  itemBoxSizer17->Add(m_checkConfirmDeletes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText;
  itemStaticText19->Create( itemPanel2, wxID_STATIC, _("Base to use for numbers :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxArrayString m_comboNumericalBaseStrings;
  m_comboNumericalBaseStrings.Add(_("Hexadecimal"));
  m_comboNumericalBaseStrings.Add(_("Decimal"));
  m_comboNumericalBase = new wxChoice;
  m_comboNumericalBase->Create( itemPanel2, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_comboNumericalBaseStrings, 0 );
  itemBoxSizer20->Add(m_comboNumericalBase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  GetBookCtrl()->AddPage(itemPanel2, _("General"), false, 0);

  wxPanel* itemPanel22 = new wxPanel;
  itemPanel22->Create( GetBookCtrl(), ID_PANEL_COMMUNICATION, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
  itemPanel22->SetSizer(itemBoxSizer23);

  wxGridSizer* itemGridSizer24 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer23->Add(itemGridSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText25 = new wxStaticText;
  itemStaticText25->Create( itemPanel22, wxID_STATIC, _("Max number of register read/write retries :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer24->Add(itemStaticText25, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer24->Add(itemBoxSizer26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_maxRetries = new wxSpinCtrl;
  m_maxRetries->Create( itemPanel22, ID_SPINCTRL, _T("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 5, 1 );
  itemBoxSizer26->Add(m_maxRetries, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText28 = new wxStaticText;
  itemStaticText28->Create( itemPanel22, wxID_STATIC, _("Register read/write timeout in seconds :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer24->Add(itemStaticText28, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer24->Add(itemBoxSizer29, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_readTimeout = new wxSpinCtrl;
  m_readTimeout->Create( itemPanel22, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 1 );
  itemBoxSizer29->Add(m_readTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel22, _("Communication"), false, 1);

  wxPanel* itemPanel31 = new wxPanel;
  itemPanel31->Create( GetBookCtrl(), ID_PANEL_COLORS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
  itemPanel31->SetSizer(itemBoxSizer32);

  GetBookCtrl()->AddPage(itemPanel31, _("Colours"), false, 2);

  wxPanel* itemPanel33 = new wxPanel;
  itemPanel33->Create( GetBookCtrl(), ID_PANEL_RECEIVE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
  itemPanel33->SetSizer(itemBoxSizer34);

  wxGridSizer* itemGridSizer35 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer34->Add(itemGridSizer35, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ChkAutoScroll = new wxCheckBox;
  m_ChkAutoScroll->Create( itemPanel33, ID_ChkAutoScroll, _("Autoscroll"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ChkAutoScroll->SetValue(true);
  itemGridSizer35->Add(m_ChkAutoScroll, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_chkPyjamasLook = new wxCheckBox;
  m_chkPyjamasLook->Create( itemPanel33, ID_ChkPyjamasLook, _("Pyjamas Look"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkPyjamasLook->SetValue(true);
  itemGridSizer35->Add(m_chkPyjamasLook, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_chkUseSymbols = new wxCheckBox;
  m_chkUseSymbols->Create( itemPanel33, ID_CHECKBOX, _("Use symbols for class and type"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkUseSymbols->SetValue(true);
  itemGridSizer35->Add(m_chkUseSymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText44 = new wxStaticText;
  itemStaticText44->Create( itemPanel33, wxID_STATIC, _("Text colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText44, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer45, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editRxForeGroundColour = new wxTextCtrl;
  m_editRxForeGroundColour->Create( itemPanel33, ID_EditRxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer45->Add(m_editRxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton47 = new wxButton;
  itemButton47->Create( itemPanel33, ID_BUTTON4, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer45->Add(itemButton47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText48 = new wxStaticText;
  itemStaticText48->Create( itemPanel33, wxID_STATIC, _("Background colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText48, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer49, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editRxBackGroundColour = new wxTextCtrl;
  m_editRxBackGroundColour->Create( itemPanel33, ID_EditRxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer49->Add(m_editRxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton51 = new wxButton;
  itemButton51->Create( itemPanel33, ID_BUTTON5, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer49->Add(itemButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText52 = new wxStaticText;
  itemStaticText52->Create( itemPanel33, wxID_STATIC, _("Text colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer53, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editTxForeGroundColour = new wxTextCtrl;
  m_editTxForeGroundColour->Create( itemPanel33, ID_EditTxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer53->Add(m_editTxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton55 = new wxButton;
  itemButton55->Create( itemPanel33, ID_BUTTON6, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer53->Add(itemButton55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText56 = new wxStaticText;
  itemStaticText56->Create( itemPanel33, wxID_STATIC, _("Background colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText56, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer57, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editTxBackGroundColour = new wxTextCtrl;
  m_editTxBackGroundColour->Create( itemPanel33, ID_EditTxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer57->Add(m_editTxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton59 = new wxButton;
  itemButton59->Create( itemPanel33, ID_BUTTON7, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer57->Add(itemButton59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer60 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer34->Add(itemBoxSizer60, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton62 = new wxButton;
  itemButton62->Create( itemPanel33, ID_BUTTON, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer61->Add(itemButton62, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton63 = new wxButton;
  itemButton63->Create( itemPanel33, ID_BUTTON1, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer61->Add(itemButton63, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText65 = new wxStaticText;
  itemStaticText65->Create( itemPanel33, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
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
  itemCheckListBox66->Create( itemPanel33, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, itemCheckListBox66Strings, wxLB_SINGLE );
  itemBoxSizer64->Add(itemCheckListBox66, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton68 = new wxButton;
  itemButton68->Create( itemPanel33, ID_BUTTON3, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer67->Add(itemButton68, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton69 = new wxButton;
  itemButton69->Create( itemPanel33, ID_BUTTON8, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer67->Add(itemButton69, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText71 = new wxStaticText;
  itemStaticText71->Create( itemPanel33, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
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
  itemCheckListBox72->Create( itemPanel33, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, itemCheckListBox72Strings, wxLB_SINGLE );
  itemBoxSizer70->Add(itemCheckListBox72, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel33, _("VSCP Receive View"), false, 3);

  wxPanel* itemPanel73 = new wxPanel;
  itemPanel73->Create( GetBookCtrl(), ID_PANEL_TRANSMIT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer74 = new wxBoxSizer(wxVERTICAL);
  itemPanel73->SetSizer(itemBoxSizer74);

  wxGridSizer* itemGridSizer75 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer74->Add(itemGridSizer75, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText76 = new wxStaticText;
  itemStaticText76->Create( itemPanel73, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText76, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer77 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer77, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl78 = new wxTextCtrl;
  itemTextCtrl78->Create( itemPanel73, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer77->Add(itemTextCtrl78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton79 = new wxButton;
  itemButton79->Create( itemPanel73, ID_BUTTON2, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer77->Add(itemButton79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText80 = new wxStaticText;
  itemStaticText80->Create( itemPanel73, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText80, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer81 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer81, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl82 = new wxTextCtrl;
  itemTextCtrl82->Create( itemPanel73, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer81->Add(itemTextCtrl82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton83 = new wxButton;
  itemButton83->Create( itemPanel73, ID_BUTTON9, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer81->Add(itemButton83, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText84 = new wxStaticText;
  itemStaticText84->Create( itemPanel73, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText84, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer85 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer85, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl86 = new wxTextCtrl;
  itemTextCtrl86->Create( itemPanel73, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer85->Add(itemTextCtrl86, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton87 = new wxButton;
  itemButton87->Create( itemPanel73, ID_BUTTON10, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer85->Add(itemButton87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText88 = new wxStaticText;
  itemStaticText88->Create( itemPanel73, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText88, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer89, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl90 = new wxTextCtrl;
  itemTextCtrl90->Create( itemPanel73, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer89->Add(itemTextCtrl90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton91 = new wxButton;
  itemButton91->Create( itemPanel73, ID_BUTTON11, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer89->Add(itemButton91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer92 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer74->Add(itemBoxSizer92, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxBoxSizer* itemBoxSizer93 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer93, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton94 = new wxButton;
  itemButton94->Create( itemPanel73, ID_BUTTON12, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer93->Add(itemButton94, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton95 = new wxButton;
  itemButton95->Create( itemPanel73, ID_BUTTON13, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer93->Add(itemButton95, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer96 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer96, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText97 = new wxStaticText;
  itemStaticText97->Create( itemPanel73, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
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
  itemCheckListBox98->Create( itemPanel73, ID_CHECKLISTBOX2, wxDefaultPosition, wxDefaultSize, itemCheckListBox98Strings, wxLB_SINGLE );
  itemBoxSizer96->Add(itemCheckListBox98, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer99 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer99, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton100 = new wxButton;
  itemButton100->Create( itemPanel73, ID_BUTTON14, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer99->Add(itemButton100, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton101 = new wxButton;
  itemButton101->Create( itemPanel73, ID_BUTTON15, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer99->Add(itemButton101, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer102, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText103 = new wxStaticText;
  itemStaticText103->Create( itemPanel73, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
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
  itemCheckListBox104->Create( itemPanel73, ID_CHECKLISTBOX3, wxDefaultPosition, wxDefaultSize, itemCheckListBox104Strings, wxLB_SINGLE );
  itemBoxSizer102->Add(itemCheckListBox104, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel73, _("VSCP Transmission View"), false, 4);

  // Connect events and objects
  itemPanel33->Connect(ID_PANEL_RECEIVE, wxEVT_INIT_DIALOG, wxInitDialogEventHandler(dlgConfiguration::OnInitDialog), NULL, this);
    ////@end dlgConfiguration content construction


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

	m_maxRetries->SetValue( g_Config.m_VscpRegisterReadMaxRetries );
	m_readTimeout->SetValue( g_Config.m_VscpRegisterReadErrorTimeout );

	m_labelLogFile->SetLabel( g_Config.m_strPathLogFile );
	m_checkEnableLogging->SetValue( g_Config.m_bEnableLog );
	m_comboLogLevel->Select( g_Config.m_logLevel );

	m_checkConfirmDeletes->SetValue( g_Config.m_bConfirmDelete );

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
	g_Config.m_VscpRegisterReadMaxRetries = m_maxRetries->GetValue();
	g_Config.m_VscpRegisterReadErrorTimeout = m_readTimeout->GetValue();
	g_Config.m_logLevel = m_comboLogLevel->GetSelection();
	g_Config.m_bEnableLog = m_checkEnableLogging->GetValue();
	g_Config.m_bConfirmDelete = m_checkConfirmDeletes->GetValue();
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
    ////@begin dlgConfiguration bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end dlgConfiguration bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgConfiguration::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin dlgConfiguration icon retrieval
  wxUnusedVar(name);
  if (name == _T("copy.xpm"))
  {
    wxIcon icon(copy_xpm);
    return icon;
  }
  return wxNullIcon;
    ////@end dlgConfiguration icon retrieval
}










/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
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
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
//

void dlgConfiguration::SetRxGridBackgroundColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameRxBgColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameRxBgColour = dlg.GetColourData().GetColour();
        m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip();  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
//

void dlgConfiguration::OnButtonSetTxGridTextColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameTxTextColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameTxTextColour = dlg.GetColourData().GetColour();
        m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
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
// wxEVT_INIT_DIALOG event handler for ID_PANEL4
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

