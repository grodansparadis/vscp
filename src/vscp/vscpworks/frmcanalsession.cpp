/////////////////////////////////////////////////////////////////////////////
// Name:        frmcanalsession.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     30/05/2007 17:40:31
// RCS-ID:      
// Copyright:   (C) 2007-2017 
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
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmcanalsessionnomidi.h"
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
////@end includes

#ifndef WIN32
#include "unistd.h"
#endif

#include "vscpworks.h"
#include "frmcanalsession.h"


extern appConfiguration g_Config;

////@begin XPM images
/* XPM */
static char *wxwin32x32_xpm[] = {
"32 32 6 1",
"   c None",
".  c #000000",
"X  c #000084",
"o  c #FFFFFF",
"O  c #FFFF00",
"+  c #FF0000",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"        ..............          ",
"        .XXXXXXXXXXXX.          ",
"        .XXXXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXX..............",
"        .XooXXXXXX.OOOOOOOOOOOO.",
".........XooXXXXXX.OOOOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++...........OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".++++++++++++.    ..............",
".++++++++++++.                  ",
"..............                  ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "};

/* XPM */
static char *open_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 5 1",
"  c None",
". c Black",
"X c Yellow",
"o c Gray100",
"O c #bfbf00",
/* pixels */
"                ",
"          ...   ",
"         .   . .",
"              ..",
"  ...        ...",
" .XoX.......    ",
" .oXoXoXoXo.    ",
" .XoXoXoXoX.    ",
" .oXoX..........",
" .XoX.OOOOOOOOO.",
" .oo.OOOOOOOOO. ",
" .X.OOOOOOOOO.  ",
" ..OOOOOOOOO.   ",
" ...........    ",
"                "
};

/* XPM */
static char *cut_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 3 1",
"  c None",
". c Black",
"X c #000080",
/* pixels */
"                ",
"     .   .      ",
"     .   .      ",
"     .   .      ",
"     .. ..      ",
"      . .       ",
"      ...       ",
"       .        ",
"      X.X       ",
"      X XXX     ",
"    XXX X  X    ",
"   X  X X  X    ",
"   X  X X  X    ",
"   X  X  XX     ",
"    XX          "
};

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

/* XPM */
static char *filesaveas_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 23 1",
"X c Black",
"+ c #FFFFFF",
"< c #D5D6D8",
"> c #446A8C",
"3 c #CAD2DC",
", c #C0C7D1",
"  c #5F666D",
"* c #A5B0BA",
"O c #65839D",
"1 c #DCE2EA",
"2 c #C3C5C8",
": c #E1E6EE",
". c #FFFF00",
"- c #C6CCD3",
"@ c None",
"& c #305F81",
"4 c #D6DFE7",
"; c #D2D9E0",
"= c #B7BFC7",
"o c #1B4467",
"$ c #BCBDBE",
"# c #7A90AC",
"% c #5D7C93",
/* pixels */
"         .X .XX.",
" oO+++++++.X.X.@",
" #O+$$$$$XX...XX",
" #O++++++.......",
" #O+$$$$$XX...XX",
" #O+++++++.X.X.@",
" ##%%%%%%.X%.X .",
" ############# @",
" ###&&&&&&&&## @",
" ##&***=-;:+&# @",
" ##&*o>>-;:+&# @",
" ##&*o>>,<1+&# @",
" ##&*o>>234+&# @",
" ##&224+++++&# @",
"@             @@"
};

/* XPM */
static char *print_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 35 1",
"y c #EDF2FB",
"9 c #4E7FD0",
"> c #B9CDED",
"# c #3365B7",
"r c #FFFFFF",
"8 c #2C58A0",
"X c #779DDB",
"7 c #7FA2DD",
"t c #3263B4",
"q c #6E96D8",
"= c #6992D7",
"* c #D9E4F5",
"0 c #356AC1",
"& c #CBD9F1",
"; c #97B4E3",
"e c #3161B1",
"  c None",
"1 c #B5C9EB",
", c #7CA0DC",
"4 c #487BCE",
"- c #4377CD",
". c #5584D1",
"6 c #3A70CA",
"2 c #305FAC",
"< c #5685D2",
"w c #4075CC",
"5 c #638ED5",
"3 c #3467BC",
"% c #2F5DA9",
"o c #ECF1FA",
": c #D6E1F4",
"@ c #376EC9",
"$ c #2D5AA5",
"+ c #A2BCE6",
"O c #CFDDF3",
/* pixels */
"   .XXXXXXXX    ",
"   .oooooooX    ",
"   .OOOOOOOX    ",
"   .+++++++X    ",
" @@#$%%%%%$%@@  ",
"#&*=-=;:>,<#*&# ",
".11.2345.63#11. ",
"4;;;;;;;;;;;;;4 ",
"677588888889776 ",
"0qq60wwwwwweqq0 ",
"3553rrrrrrr$553 ",
"3<<tyrrrrrr$<<3 ",
"XXX%rrrrrrr$XXX ",
"   3rrrrrrr$    ",
"   #XXXXXXX@    "
};

/* XPM */
static char *save_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 4 1",
"  c None",
". c Black",
"X c #808000",
"o c #808080",
/* pixels */
"                ",
" .............. ",
" .X.        . . ",
" .X.        ... ",
" .X.        .X. ",
" .X.        .X. ",
" .X.        .X. ",
" .X.        .X. ",
" .XX........oX. ",
" .XXXXXXXXXXXX. ",
" .XX.........X. ",
" .XX......  .X. ",
" .XX......  .X. ",
" .XX......  .X. ",
"  ............. "
};

/* XPM */
static char *paste_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 6 1",
"  c None",
". c Black",
"X c Yellow",
"o c #808080",
"O c #000080",
"+ c Gray100",
/* pixels */
"                ",
"     ....       ",
" .....XX.....   ",
".ooo.X..X.ooo.  ",
".oo.      .oo.  ",
".oo........oo.  ",
".oooooooooooo.  ",
".oooooOOOOOOO.  ",
".oooooO+++++OO  ",
".oooooO+++++O+O ",
".oooooO+OOO+OOO ",
".oooooO+++++++O ",
".oooooO+OOOOO+O ",
" .....O+++++++O ",
"      OOOOOOOOO "
};

/* XPM */
static char *preview_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 30 1",
"1 c Black",
"= c #97C4E7",
"< c #FFFFFF",
", c #69A1CF",
". c #5A89A6",
"# c #3A749C",
"+ c #538BB1",
"> c #D1E5F5",
"o c #6792AE",
"% c #C3DDF1",
"8 c #749BB5",
"; c #619BC4",
"X c #6B97B6",
"@ c #4B82A8",
"  c None",
"O c #5F8FB0",
"q c #84A5BB",
": c #71A8D1",
"- c #85BBE2",
"9 c #EFF6FC",
"7 c #F7FBFD",
"6 c #FAFCFE",
"2 c #7FA6C0",
"5 c #C00000",
"0 c #FDFDFE",
"3 c #E2EFF8",
"$ c #8EA9BC",
"& c #B6D5EE",
"* c #A5CCEA",
"4 c #F4F9FD",
/* pixels */
"  ..XoO+@#$     ",
"  .%%&*=-;:;    ",
"  .>>%&*=,<=X   ",
"  $%%%1112<<=X  ",
"  $3-1<<<1oXO+  ",
"  $314<<<<1=-+  ",
"  $31<<<<<1-=+  ",
"  $31<<<<<1&*O  ",
"  $3-1<<<51>%X  ",
"  $367111551>8  ",
"  $3-----95518  ",
"  $3<067466551  ",
"  $3--------551 ",
"  $3<<<0666<<55 ",
"  $$$$$$$$$$q$  "
};

/* XPM */
static char *new_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 31 1",
". c #7198D9",
"2 c #DCE6F6",
", c #FFFFFF",
"= c #9AB6E4",
"6 c #EAF0FA",
"w c #6992D7",
"0 c #5886D2",
"7 c #F7F9FD",
"5 c #F0F5FC",
"* c #A8C0E8",
"  c None",
"8 c #FDFEFF",
"% c #C4D5F0",
"3 c #E2EAF8",
"+ c #4377CD",
"O c #487BCE",
"; c #6B94D7",
"- c #89A9DF",
": c #5584D1",
"# c #3569BF",
"@ c #3A70CA",
"1 c #D2DFF4",
"> c #3366BB",
"$ c #2E5CA8",
"9 c #FAFCFE",
"4 c #F5F8FD",
"q c #638ED5",
"o c #5282D0",
"& c #B8CCEC",
"X c #376EC9",
"< c #ACE95B",
/* pixels */
"   .XoO+@#$.    ",
"   .%%&*=-O;:   ",
"  >>>>%&*=O,=o  ",
"  ><<>%%&*O,,=o ",
">>><<>>>%&OOo+@ ",
"><<<<<<>1%&*=-@ ",
"><<<<<<>21%&*=@ ",
">>><<>>>321%&*+ ",
"  ><<>456321%&O ",
"  >>>>7456321%o ",
"   .,8974563210 ",
"   .,,897456320 ",
"   .,,,8974563q ",
"   .,,,,897456w ",
"   ............ "
};

////@end XPM images


/*!
 * frmCANALSession type definition
 */

IMPLEMENT_CLASS( frmCANALSession, wxFrame )


/*!
 * frmCANALSession event table definition
 */

BEGIN_EVENT_TABLE( frmCANALSession, wxFrame )

////@begin frmCANALSession event table entries
  EVT_CLOSE( frmCANALSession::OnCloseWindow )

  EVT_GRID_CELL_RIGHT_CLICK( frmCANALSession::OnGridCellRightClick )
  EVT_GRID_LABEL_LEFT_DCLICK( frmCANALSession::OnGridLabelLeftDClick )

  EVT_MENU( ID_MENUITEM_CANAL_LOAD_MSG_LIST, frmCANALSession::OnMenuitemCanalLoadMsgListClick )

  EVT_MENU( ID_MENUITEM_CANAL_SAVE_MSG_LIST, frmCANALSession::OnMenuitemCanalSaveMsgListClick )

  EVT_MENU( ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET, frmCANALSession::OnMenuitemCanalLoadTransmissionSetClick )

  EVT_MENU( ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET, frmCANALSession::OnMenuitemCanalSaveTransmissionSetClick )

  EVT_MENU( ID_MENUITEM_CANAL_EXIT, frmCANALSession::OnMenuitemCanalExittClick )

  EVT_MENU( ID_MENUITEM_CANAL_ABOUT, frmCANALSession::OnMenuitemCanalAboutClick )

////@end frmCANALSession event table entries

END_EVENT_TABLE()


/*!
 * frmCANALSession constructors
 */

frmCANALSession::frmCANALSession()
{
    Init();
}

frmCANALSession::frmCANALSession( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * frmCANALSessionNoMIDI creator
 */

bool frmCANALSession::Create( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style )
{
////@begin frmCANALSession creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  this->SetBackgroundColour(wxColour(239, 235, 231));
  CreateControls();
  SetIcon(GetIconResource(wxT("wxwin32x32.xpm")));
////@end frmCANALSession creation
    return true;
}


/*!
 * frmCANALSession destructor
 */

frmCANALSession::~frmCANALSession()
{
////@begin frmCANALSession destruction
////@end frmCANALSession destruction
}


/*!
 * Member initialisation
 */

void frmCANALSession::Init()
{
////@begin frmCANALSession member initialisation
  m_ctrlGridReceive = NULL;
  m_btnEdit = NULL;
  m_btnDelete = NULL;
  m_btnLoadSet = NULL;
  m_btnSaveSet = NULL;
  m_ctrlGridTransmission = NULL;
  m_btnSend = NULL;
  m_btnActivate = NULL;
  m_btnClear = NULL;
  m_comboSelectView = NULL;
  m_ctrlBtnActivateInterface = NULL;
////@end frmCANALSession member initialisation
}


/*!
 * Control creation for frmCANALSessionNoMIDI
 */

void frmCANALSession::CreateControls()
{    
////@begin frmCANALSession content construction
  frmCANALSession* itemFrame1 = this;

  wxMenuBar* menuBar = new wxMenuBar;
  wxMenu* itemMenu35 = new wxMenu;
  itemMenu35->Append(ID_MENUITEM_CANAL_LOAD_MSG_LIST, _("Load CAN message list from file..."), _T(""), wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_CANAL_SAVE_MSG_LIST, _("Save CAN message list to file..."), _T(""), wxITEM_NORMAL);
  itemMenu35->AppendSeparator();
  itemMenu35->Append(ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET, _("Load transmission set from file..."), _T(""), wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET, _("Save transmission set to file..."), _T(""), wxITEM_NORMAL);
  itemMenu35->AppendSeparator();
  itemMenu35->Append(ID_MENUITEM_CANAL_EXIT, _("Exit"), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu35, _("File"));
  wxMenu* itemMenu43 = new wxMenu;
  itemMenu43->Append(ID_MENUITEM_CANAL_CUT, _("Cut"), _T(""), wxITEM_NORMAL);
  itemMenu43->Append(ID_MENUITEM_CANAL_COPY, _("Copy"), _T(""), wxITEM_NORMAL);
  itemMenu43->Append(ID_MENUITEM_CANAL_PASTE, _("Paste"), _T(""), wxITEM_NORMAL);
  itemMenu43->AppendSeparator();
  menuBar->Append(itemMenu43, _("Edit"));
  wxMenu* itemMenu48 = new wxMenu;
  itemMenu48->Append(ID_MENUITEM_CANAL_LOG, _("Message Log"), _T(""), wxITEM_RADIO);
  itemMenu48->Check(ID_MENUITEM_CANAL_LOG, true);
  itemMenu48->Append(ID_MENUITEM_CANAL_COUNT, _("Message Count"), _T(""), wxITEM_RADIO);
  menuBar->Append(itemMenu48, _("View"));
  wxMenu* itemMenu51 = new wxMenu;
  itemMenu51->Append(ID_MENUITEM_CANAL_AUTO_REPLY, _("Auto Reply Settings..."), _T(""), wxITEM_NORMAL);
  itemMenu51->Append(ID_MENUITEM_CANAL_BURST_COUNT, _("Set Burst Count..."), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu51, _("Tools"));
  wxMenu* itemMenu54 = new wxMenu;
  itemMenu54->Append(ID_MENUITEM_CANAL_HELP, _("VSCP-Works Help"), _T(""), wxITEM_RADIO);
  itemMenu54->Append(ID_MENUITEM_CANAL_FAQ, _("Frequently Asked Questions"), _T(""), wxITEM_NORMAL);
  itemMenu54->Append(ID_MENUITEM_CANAL_SHORTCUTS, _("Keyboard shortcuts"), _T(""), wxITEM_NORMAL);
  itemMenu54->AppendSeparator();
  itemMenu54->Append(ID_MENUITEM_CANAL_THANKS, _("Thanks..."), _T(""), wxITEM_NORMAL);
  itemMenu54->Append(ID_MENUITEM_CANAL_CREDITS, _("Credits..."), _T(""), wxITEM_NORMAL);
  itemMenu54->AppendSeparator();
  itemMenu54->Append(ID_MENUITEM_CANAL_VSCP_SITE, _("Go to VSCP site"), _T(""), wxITEM_NORMAL);
  itemMenu54->AppendSeparator();
  itemMenu54->Append(ID_MENUITEM_CANAL_ABOUT, _("About"), _T(""), wxITEM_NORMAL);
  menuBar->Append(itemMenu54, _("Help"));
  itemFrame1->SetMenuBar(menuBar);

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemFrame1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 2);

  m_ctrlGridReceive = new wxGrid( itemFrame1, ID_GRID_RECEIVE, wxDefaultPosition, wxSize(800, 300), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_ctrlGridReceive->SetBackgroundColour(wxColour(251, 252, 170));
  m_ctrlGridReceive->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
  m_ctrlGridReceive->SetDefaultColSize(100);
  m_ctrlGridReceive->SetDefaultRowSize(22);
  m_ctrlGridReceive->SetColLabelSize(22);
  m_ctrlGridReceive->SetRowLabelSize(30);
  m_ctrlGridReceive->CreateGrid(1, 8, wxGrid::wxGridSelectRows);
  itemBoxSizer3->Add(m_ctrlGridReceive, 0, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer6, 0, wxGROW|wxALL, 0);

  wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer6->Add(itemBoxSizer7, 0, wxGROW|wxALL, 5);

  m_btnEdit = new wxBitmapButton;
  m_btnEdit->Create( itemFrame1, ID_BITMAPBUTTON_CANAL_EDIT, itemFrame1->GetBitmapResource(wxT("open.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmCANALSession::ShowToolTips())
    m_btnEdit->SetToolTip(_("Edit selected line"));
  itemBoxSizer7->Add(m_btnEdit, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnDelete = new wxBitmapButton;
  m_btnDelete->Create( itemFrame1, ID_BITMAPBUTTONID_MENUITEM_CANAL_DELETE, itemFrame1->GetBitmapResource(wxT("cut.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmCANALSession::ShowToolTips())
    m_btnDelete->SetToolTip(_("Delete selected line"));
  itemBoxSizer7->Add(m_btnDelete, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  itemBoxSizer7->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_btnLoadSet = new wxBitmapButton;
  m_btnLoadSet->Create( itemFrame1, ID_BITMAPBUTTONID_MENUITEM_CANAL_LOAD, itemFrame1->GetBitmapResource(wxT("copy.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmCANALSession::ShowToolTips())
    m_btnLoadSet->SetToolTip(_("Load transmission set from file..."));
  itemBoxSizer7->Add(m_btnLoadSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnSaveSet = new wxBitmapButton;
  m_btnSaveSet->Create( itemFrame1, ID_BITMAPBUTTONID_MENUITEM_CANAL_SAVE, itemFrame1->GetBitmapResource(wxT("filesaveas.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmCANALSession::ShowToolTips())
    m_btnSaveSet->SetToolTip(_("Save transmission set to file..."));
  itemBoxSizer7->Add(m_btnSaveSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer6->Add(itemBoxSizer13, 0, wxGROW|wxALL, 5);

  m_ctrlGridTransmission = new wxGrid( itemFrame1, ID_GRID_TRANSMISSION, wxDefaultPosition, wxSize(750, 100), wxHSCROLL|wxVSCROLL );
  m_ctrlGridTransmission->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
  m_ctrlGridTransmission->SetDefaultColSize(100);
  m_ctrlGridTransmission->SetDefaultRowSize(22);
  m_ctrlGridTransmission->SetColLabelSize(22);
  m_ctrlGridTransmission->SetRowLabelSize(30);
  m_ctrlGridTransmission->CreateGrid(100, 5, wxGrid::wxGridSelectRows);
  itemBoxSizer13->Add(m_ctrlGridTransmission, 400, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer6->Add(itemBoxSizer15, 0, wxGROW|wxALL, 5);

  m_btnSend = new wxBitmapButton;
  m_btnSend->Create( itemFrame1, ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND, itemFrame1->GetBitmapResource(wxT("copy.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmCANALSession::ShowToolTips())
    m_btnSend->SetToolTip(_("Send message from selected row(s)"));
  itemBoxSizer15->Add(m_btnSend, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_btnActivate = new wxBitmapButton;
  m_btnActivate->Create( itemFrame1, ID_BITMAPBUTTON12, itemFrame1->GetBitmapResource(wxT("Print.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  m_btnActivate->Show(false);
  itemBoxSizer15->Add(m_btnActivate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnClear = new wxBitmapButton;
  m_btnClear->Create( itemFrame1, ID_BITMAPBUTTON13, itemFrame1->GetBitmapResource(wxT("open.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  m_btnClear->Show(false);
  itemBoxSizer15->Add(m_btnClear, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxToolBar* itemToolBar20 = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR_CANAL );
  wxBitmap itemtool21Bitmap(itemFrame1->GetBitmapResource(wxT("open.xpm")));
  wxBitmap itemtool21BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_OPEN, _T(""), itemtool21Bitmap, itemtool21BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  wxBitmap itemtool22Bitmap(itemFrame1->GetBitmapResource(wxT("save.xpm")));
  wxBitmap itemtool22BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_SAVE, _T(""), itemtool22Bitmap, itemtool22BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  itemToolBar20->AddSeparator();
  wxBitmap itemtool24Bitmap(itemFrame1->GetBitmapResource(wxT("cut.xpm")));
  wxBitmap itemtool24BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_CUT, _T(""), itemtool24Bitmap, itemtool24BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  wxBitmap itemtool25Bitmap(itemFrame1->GetBitmapResource(wxT("copy.xpm")));
  wxBitmap itemtool25BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_COPY, _T(""), itemtool25Bitmap, itemtool25BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  wxBitmap itemtool26Bitmap(itemFrame1->GetBitmapResource(wxT("paste.xpm")));
  wxBitmap itemtool26BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_PASTE, _T(""), itemtool26Bitmap, itemtool26BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  itemToolBar20->AddSeparator();
  wxBitmap itemtool28Bitmap(itemFrame1->GetBitmapResource(wxT("Preview.xpm")));
  wxBitmap itemtool28BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_PREVIEW, _T(""), itemtool28Bitmap, itemtool28BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  wxBitmap itemtool29Bitmap(itemFrame1->GetBitmapResource(wxT("Print.xpm")));
  wxBitmap itemtool29BitmapDisabled;
  itemToolBar20->AddTool(ID_TOOL_CANAL_PRINT, _T(""), itemtool29Bitmap, itemtool29BitmapDisabled, wxITEM_NORMAL, _T(""), wxEmptyString);
  itemToolBar20->AddSeparator();
  wxArrayString m_comboSelectViewStrings;
  m_comboSelectViewStrings.Add(_("Message Log"));
  m_comboSelectViewStrings.Add(_("Message Count"));
  m_comboSelectView = new wxComboBox;
  m_comboSelectView->Create( itemToolBar20, ID_COMBOBOX, _("Message Log"), wxDefaultPosition, wxDefaultSize, m_comboSelectViewStrings, wxCB_DROPDOWN );
  m_comboSelectView->SetStringSelection(_("Message Log"));
  if (frmCANALSession::ShowToolTips())
    m_comboSelectView->SetToolTip(_("Select the way messages are shown"));
  m_comboSelectView->SetBackgroundColour(wxColour(255, 255, 185));
  itemToolBar20->AddControl(m_comboSelectView);
  itemToolBar20->AddSeparator();
  m_ctrlBtnActivateInterface = new wxBitmapButton;
  m_ctrlBtnActivateInterface->Create( itemToolBar20, ID_BITMAPBUTTON, itemFrame1->GetBitmapResource(wxT("New1.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemToolBar20->AddControl(m_ctrlBtnActivateInterface);
  itemToolBar20->Realize();
  itemFrame1->SetToolBar(itemToolBar20);

////@end frmCANALSession content construction

  // Receive Grid
  m_ctrlGridReceive->SetCellBackgroundColour( g_Config.m_CanalRcvFrameBgColour );
	m_ctrlGridReceive->SetGridLineColour( g_Config.m_CanalRcvFrameLineColour );
  m_ctrlGridReceive->SetCellTextColour( g_Config.m_CanalRcvFrameTextColour );
  
  m_ctrlGridReceive->SetRowLabelSize( 50 );
  
  m_ctrlGridReceive->SetColSize( 0, g_Config.m_CanalRcvFieldWidth[0] );
	m_ctrlGridReceive->SetColLabelValue( 0, g_Config.m_CanalRcvFieldText[0] );

	m_ctrlGridReceive->SetColSize( 1, g_Config.m_CanalRcvFieldWidth[1] );
	m_ctrlGridReceive->SetColLabelValue( 1, g_Config.m_CanalRcvFieldText[1] );
	
	m_ctrlGridReceive->SetColSize( 2, g_Config.m_CanalRcvFieldWidth[2] );
	m_ctrlGridReceive->SetColLabelValue( 2, g_Config.m_CanalRcvFieldText[2] );
	
	m_ctrlGridReceive->SetColSize( 3, g_Config.m_CanalRcvFieldWidth[3] );
	m_ctrlGridReceive->SetColLabelValue( 3, g_Config.m_CanalRcvFieldText[3] );
	
	m_ctrlGridReceive->SetColSize( 4,g_Config.m_CanalRcvFieldWidth[4] );
	m_ctrlGridReceive->SetColLabelValue( 4, g_Config.m_CanalRcvFieldText[4] );

	m_ctrlGridReceive->SetColSize( 5, g_Config.m_CanalRcvFieldWidth[5] );
	m_ctrlGridReceive->SetColLabelValue( 5, g_Config.m_CanalRcvFieldText[5] );
	
	m_ctrlGridReceive->SetColSize( 6, g_Config.m_CanalRcvFieldWidth[6] );
	m_ctrlGridReceive->SetColLabelValue( 6, g_Config.m_CanalRcvFieldText[6] );
	
	m_ctrlGridReceive->SetColSize( 7, g_Config.m_CanalRcvFieldWidth[7] );
	m_ctrlGridReceive->SetColLabelValue( 7, g_Config.m_CanalRcvFieldText[7] );

  // Transmit Grid
  m_ctrlGridTransmission->SetCellBackgroundColour( g_Config.m_CanalTrmitFrameBgColour );
	m_ctrlGridTransmission->SetGridLineColour( g_Config.m_CanalTrmitFrameLineColour );
  m_ctrlGridTransmission->SetCellTextColour( g_Config.m_CanalTrmitFrameTextColour );
  
  m_ctrlGridTransmission->SetRowLabelSize( 50 );
  
  m_ctrlGridTransmission->SetColSize( 0, g_Config.m_CanalTrmitFieldWidth[0] );
	m_ctrlGridTransmission->SetColLabelValue( 0, g_Config.m_CanalTrmitFieldText[0] );

	m_ctrlGridTransmission->SetColSize( 1, g_Config.m_CanalTrmitFieldWidth[1] );
	m_ctrlGridTransmission->SetColLabelValue( 1, g_Config.m_CanalTrmitFieldText[1] );
	
	m_ctrlGridTransmission->SetColSize( 2, g_Config.m_CanalTrmitFieldWidth[2] );
	m_ctrlGridTransmission->SetColLabelValue( 2, g_Config.m_CanalTrmitFieldText[2] );
	
	m_ctrlGridTransmission->SetColSize( 3, g_Config.m_CanalTrmitFieldWidth[3] );
	m_ctrlGridTransmission->SetColLabelValue( 3, g_Config.m_CanalTrmitFieldText[3] );
	
	m_ctrlGridTransmission->SetColSize( 4, g_Config.m_CanalTrmitFieldWidth[4] );
	m_ctrlGridTransmission->SetColLabelValue( 4, g_Config.m_CanalTrmitFieldText[4] );
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_RECEIVE
 */

void frmCANALSession::OnGridCellRightClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_RECEIVE in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_RECEIVE in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID_RECEIVE
 */

void frmCANALSession::OnGridLabelLeftDClick( wxGridEvent& event )
{
////@begin wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID_RECEIVE in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_GRID_RECEIVE in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_MSG_LIST
 */

void frmCANALSession::OnMenuitemCanalLoadMsgListClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_MSG_LIST in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_MSG_LIST in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_MSG_LIST
 */

void frmCANALSession::OnMenuitemCanalSaveMsgListClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_MSG_LIST in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_MSG_LIST in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET
 */

void frmCANALSession::OnMenuitemCanalLoadTransmissionSetClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_LOAD_TRANSMISSION_SET in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET
 */

void frmCANALSession::OnMenuitemCanalSaveTransmissionSetClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_SAVE_TRANSMISSION_SET in frmCANALSessionNoMIDI. 
}



//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_ABOUT
 */

void frmCANALSession::OnMenuitemCanalAboutClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_ABOUT in frmCANALSessionNoMIDI.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_ABOUT in frmCANALSessionNoMIDI. 
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * Should we show tooltips?
 */

bool frmCANALSession::ShowToolTips()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * Get bitmap resources
 */

wxBitmap frmCANALSession::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin frmCANALSession bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("open.xpm"))
  {
    wxBitmap bitmap(open_xpm);
    return bitmap;
  }
  else if (name == _T("cut.xpm"))
  {
    wxBitmap bitmap(cut_xpm);
    return bitmap;
  }
  else if (name == _T("copy.xpm"))
  {
    wxBitmap bitmap(copy_xpm);
    return bitmap;
  }
  else if (name == _T("filesaveas.xpm"))
  {
    wxBitmap bitmap(filesaveas_xpm);
    return bitmap;
  }
  else if (name == _T("Print.xpm"))
  {
    wxBitmap bitmap(print_xpm);
    return bitmap;
  }
  else if (name == _T("save.xpm"))
  {
    wxBitmap bitmap(save_xpm);
    return bitmap;
  }
  else if (name == _T("paste.xpm"))
  {
    wxBitmap bitmap(paste_xpm);
    return bitmap;
  }
  else if (name == _T("Preview.xpm"))
  {
    wxBitmap bitmap(preview_xpm);
    return bitmap;
  }
  else if (name == _T("New1.xpm"))
  {
    wxBitmap bitmap(new_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end frmCANALSession bitmap retrieval
}

//////////////////////////////////////////////////////////////////////////////////////////
//  
//

/*!
 * Get icon resources
 */

wxIcon frmCANALSession::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin frmCANALSession icon retrieval
  wxUnusedVar(name);
  if (name == _T("wxwin32x32.xpm"))
  {
    wxIcon icon(wxwin32x32_xpm);
    return icon;
  }
  return wxNullIcon;
////@end frmCANALSession icon retrieval
}

//////////////////////////////////////////////////////////////////////////////////////////
//  wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CANAL_EXIT
//

void frmCANALSession::OnMenuitemCanalExittClick( wxCommandEvent& event )
{
  Close();
  event.Skip(); 
}

//////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_CLOSE_WINDOW event handler for ID_FRMCANALSESSION
//

void frmCANALSession::OnCloseWindow( wxCloseEvent& event )
{

  event.Skip();
}

