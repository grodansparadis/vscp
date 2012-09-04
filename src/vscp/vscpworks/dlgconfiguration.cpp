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

    GetBookCtrl()->AddPage(itemPanel2, _("General"), false, 0);

    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create( GetBookCtrl(), ID_PANEL_COMMUNICATION, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    GetBookCtrl()->AddPage(itemPanel4, _("Communication"), false, 1);

    wxPanel* itemPanel6 = new wxPanel;
    itemPanel6->Create( GetBookCtrl(), ID_PANEL_COLORS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel6->SetSizer(itemBoxSizer7);

    GetBookCtrl()->AddPage(itemPanel6, _("Colours"), false, 2);

    wxPanel* itemPanel8 = new wxPanel;
    itemPanel8->Create( GetBookCtrl(), ID_PANEL_RECEIVE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemPanel8->SetSizer(itemBoxSizer9);

    wxGridSizer* itemGridSizer10 = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizer9->Add(itemGridSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
    itemGridSizer10->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_ChkAutoScroll = new wxCheckBox;
    m_ChkAutoScroll->Create( itemPanel8, ID_ChkAutoScroll, _("Autoscroll"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ChkAutoScroll->SetValue(true);
    itemGridSizer10->Add(m_ChkAutoScroll, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizer10->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_chkPyjamasLook = new wxCheckBox;
    m_chkPyjamasLook->Create( itemPanel8, ID_ChkPyjamasLook, _("Pyjamas Look"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkPyjamasLook->SetValue(true);
    itemGridSizer10->Add(m_chkPyjamasLook, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizer10->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_chkUseSymbols = new wxCheckBox;
    m_chkUseSymbols->Create( itemPanel8, ID_CHECKBOX, _("Use symbols for class and type"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkUseSymbols->SetValue(true);
    itemGridSizer10->Add(m_chkUseSymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizer10->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    itemGridSizer10->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText19 = new wxStaticText;
    itemStaticText19->Create( itemPanel8, wxID_STATIC, _("Text colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer10->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer10->Add(itemBoxSizer20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editRxForeGroundColour = new wxTextCtrl;
    m_editRxForeGroundColour->Create( itemPanel8, ID_EditRxForeGroundColour, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer20->Add(m_editRxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton22 = new wxButton;
    itemButton22->Create( itemPanel8, ID_BUTTON4, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer20->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemPanel8, wxID_STATIC, _("Background colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer10->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer10->Add(itemBoxSizer24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editRxBackGroundColour = new wxTextCtrl;
    m_editRxBackGroundColour->Create( itemPanel8, ID_EditRxBackGroundColour, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer24->Add(m_editRxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton26 = new wxButton;
    itemButton26->Create( itemPanel8, ID_BUTTON5, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer24->Add(itemButton26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText27 = new wxStaticText;
    itemStaticText27->Create( itemPanel8, wxID_STATIC, _("Text colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer10->Add(itemStaticText27, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer10->Add(itemBoxSizer28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editTxForeGroundColour = new wxTextCtrl;
    m_editTxForeGroundColour->Create( itemPanel8, ID_EditTxForeGroundColour, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer28->Add(m_editTxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton30 = new wxButton;
    itemButton30->Create( itemPanel8, ID_BUTTON6, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer28->Add(itemButton30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create( itemPanel8, wxID_STATIC, _("Background colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer10->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer10->Add(itemBoxSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    m_editTxBackGroundColour = new wxTextCtrl;
    m_editTxBackGroundColour->Create( itemPanel8, ID_EditTxBackGroundColour, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
    itemBoxSizer32->Add(m_editTxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton34 = new wxButton;
    itemButton34->Create( itemPanel8, ID_BUTTON7, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer32->Add(itemButton34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(itemBoxSizer35, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer35->Add(itemBoxSizer36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton37 = new wxButton;
    itemButton37->Create( itemPanel8, ID_BUTTON, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer36->Add(itemButton37, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton38 = new wxButton;
    itemButton38->Create( itemPanel8, ID_BUTTON1, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer36->Add(itemButton38, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer35->Add(itemBoxSizer39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText40 = new wxStaticText;
    itemStaticText40->Create( itemPanel8, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer39->Add(itemStaticText40, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox41Strings;
    itemCheckListBox41Strings.Add(_("Dir"));
    itemCheckListBox41Strings.Add(_("Time"));
    itemCheckListBox41Strings.Add(_("GUID"));
    itemCheckListBox41Strings.Add(_("Class"));
    itemCheckListBox41Strings.Add(_("Type"));
    itemCheckListBox41Strings.Add(_("Head"));
    itemCheckListBox41Strings.Add(_("Data Count"));
    itemCheckListBox41Strings.Add(_("Data"));
    itemCheckListBox41Strings.Add(_("Timestamp"));
    itemCheckListBox41Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox41 = new wxCheckListBox;
    itemCheckListBox41->Create( itemPanel8, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, itemCheckListBox41Strings, wxLB_SINGLE );
    itemBoxSizer39->Add(itemCheckListBox41, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer35->Add(itemBoxSizer42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton43 = new wxButton;
    itemButton43->Create( itemPanel8, ID_BUTTON3, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer42->Add(itemButton43, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton44 = new wxButton;
    itemButton44->Create( itemPanel8, ID_BUTTON8, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer42->Add(itemButton44, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer35->Add(itemBoxSizer45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemPanel8, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer45->Add(itemStaticText46, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox47Strings;
    itemCheckListBox47Strings.Add(_("Dir"));
    itemCheckListBox47Strings.Add(_("Time"));
    itemCheckListBox47Strings.Add(_("GUID"));
    itemCheckListBox47Strings.Add(_("Class"));
    itemCheckListBox47Strings.Add(_("Type"));
    itemCheckListBox47Strings.Add(_("Head"));
    itemCheckListBox47Strings.Add(_("Data Count"));
    itemCheckListBox47Strings.Add(_("Data"));
    itemCheckListBox47Strings.Add(_("Timestamp"));
    itemCheckListBox47Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox47 = new wxCheckListBox;
    itemCheckListBox47->Create( itemPanel8, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, itemCheckListBox47Strings, wxLB_SINGLE );
    itemBoxSizer45->Add(itemCheckListBox47, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanel8, _("VSCP Receive View"), false, 3);

    wxPanel* itemPanel48 = new wxPanel;
    itemPanel48->Create( GetBookCtrl(), ID_PANEL_TRANSMIT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxVERTICAL);
    itemPanel48->SetSizer(itemBoxSizer49);

    wxGridSizer* itemGridSizer50 = new wxGridSizer(24, 2, 0, 0);
    itemBoxSizer49->Add(itemGridSizer50, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemPanel48, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer50->Add(itemStaticText51, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer52 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer50->Add(itemBoxSizer52, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl53 = new wxTextCtrl;
    itemTextCtrl53->Create( itemPanel48, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer52->Add(itemTextCtrl53, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton54 = new wxButton;
    itemButton54->Create( itemPanel48, ID_BUTTON2, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer52->Add(itemButton54, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText55 = new wxStaticText;
    itemStaticText55->Create( itemPanel48, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer50->Add(itemStaticText55, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer56 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer50->Add(itemBoxSizer56, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl57 = new wxTextCtrl;
    itemTextCtrl57->Create( itemPanel48, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer56->Add(itemTextCtrl57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton58 = new wxButton;
    itemButton58->Create( itemPanel48, ID_BUTTON9, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer56->Add(itemButton58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText59 = new wxStaticText;
    itemStaticText59->Create( itemPanel48, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer50->Add(itemStaticText59, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer60 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer50->Add(itemBoxSizer60, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl61 = new wxTextCtrl;
    itemTextCtrl61->Create( itemPanel48, ID_TEXTCTRL3, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer60->Add(itemTextCtrl61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton62 = new wxButton;
    itemButton62->Create( itemPanel48, ID_BUTTON10, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer60->Add(itemButton62, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText63 = new wxStaticText;
    itemStaticText63->Create( itemPanel48, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer50->Add(itemStaticText63, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer50->Add(itemBoxSizer64, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    wxTextCtrl* itemTextCtrl65 = new wxTextCtrl;
    itemTextCtrl65->Create( itemPanel48, ID_TEXTCTRL4, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer64->Add(itemTextCtrl65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxButton* itemButton66 = new wxButton;
    itemButton66->Create( itemPanel48, ID_BUTTON11, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer64->Add(itemButton66, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer49->Add(itemBoxSizer67, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer68 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer67->Add(itemBoxSizer68, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton69 = new wxButton;
    itemButton69->Create( itemPanel48, ID_BUTTON12, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer68->Add(itemButton69, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton70 = new wxButton;
    itemButton70->Create( itemPanel48, ID_BUTTON13, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer68->Add(itemButton70, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer67->Add(itemBoxSizer71, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText72 = new wxStaticText;
    itemStaticText72->Create( itemPanel48, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer71->Add(itemStaticText72, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox73Strings;
    itemCheckListBox73Strings.Add(_("Dir"));
    itemCheckListBox73Strings.Add(_("Time"));
    itemCheckListBox73Strings.Add(_("GUID"));
    itemCheckListBox73Strings.Add(_("Class"));
    itemCheckListBox73Strings.Add(_("Type"));
    itemCheckListBox73Strings.Add(_("Head"));
    itemCheckListBox73Strings.Add(_("Data Count"));
    itemCheckListBox73Strings.Add(_("Data"));
    itemCheckListBox73Strings.Add(_("Timestamp"));
    itemCheckListBox73Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox73 = new wxCheckListBox;
    itemCheckListBox73->Create( itemPanel48, ID_CHECKLISTBOX2, wxDefaultPosition, wxDefaultSize, itemCheckListBox73Strings, wxLB_SINGLE );
    itemBoxSizer71->Add(itemCheckListBox73, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer74 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer67->Add(itemBoxSizer74, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton75 = new wxButton;
    itemButton75->Create( itemPanel48, ID_BUTTON14, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer74->Add(itemButton75, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton76 = new wxButton;
    itemButton76->Create( itemPanel48, ID_BUTTON15, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer74->Add(itemButton76, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer77 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer67->Add(itemBoxSizer77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText78 = new wxStaticText;
    itemStaticText78->Create( itemPanel48, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer77->Add(itemStaticText78, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemCheckListBox79Strings;
    itemCheckListBox79Strings.Add(_("Dir"));
    itemCheckListBox79Strings.Add(_("Time"));
    itemCheckListBox79Strings.Add(_("GUID"));
    itemCheckListBox79Strings.Add(_("Class"));
    itemCheckListBox79Strings.Add(_("Type"));
    itemCheckListBox79Strings.Add(_("Head"));
    itemCheckListBox79Strings.Add(_("Data Count"));
    itemCheckListBox79Strings.Add(_("Data"));
    itemCheckListBox79Strings.Add(_("Timestamp"));
    itemCheckListBox79Strings.Add(_("Note"));
    wxCheckListBox* itemCheckListBox79 = new wxCheckListBox;
    itemCheckListBox79->Create( itemPanel48, ID_CHECKLISTBOX3, wxDefaultPosition, wxDefaultSize, itemCheckListBox79Strings, wxLB_SINGLE );
    itemBoxSizer77->Add(itemCheckListBox79, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanel48, _("VSCP Transmission View"), false, 4);

    // Connect events and objects
    itemPanel8->Connect(ID_PANEL_RECEIVE, wxEVT_INIT_DIALOG, wxInitDialogEventHandler(dlgConfiguration::OnInitDialog), NULL, this);
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
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getDialogData
//

bool dlgConfiguration::getDialogData( bool bWriteToConfigFile )
{
    g_Config.m_bAutoscollRcv = m_ChkAutoScroll->GetValue();
    g_Config.m_VscpRcvFrameRxbPyamas = m_chkPyjamasLook->GetValue();	
    g_Config.m_UseSymbolicNames = m_chkUseSymbols->GetValue();

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
    if (name == _T("copy.xpm")) {
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

