/////////////////////////////////////////////////////////////////////////////
// Name:        frmScanfordevices.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     23/05/2009 17:40:41
// RCS-ID:      
// Copyright:   (C) 2009-2014 
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmscanfordevices.h"
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

#include <list>

////@begin includes
#include "frmdeviceconfig.h"
#include "wx/imaglist.h"
////@end includes

#include <wx/tokenzr.h>
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/listimpl.cpp>

#include "vscpworks.h"
#include "../common/canal.h"
#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "dlgabout.h"
#include "frmscanfordevices.h"

extern appConfiguration g_Config;

////@begin XPM images
////@end XPM images

static const char *fatbee_v2_xpm[] = {
    /* columns rows colors chars-per-pixel */
    "32 32 253 2",
    "c. c #75790B",
    "c  c Black",
    "dX c #A8AE0F",
    "3. c #3C3C3C",
    "l  c #73E4E4",
    "d  c #595B12",
    "~  c #4A4C08",
    "y. c #292A04",
    "8X c #6E4F0B",
    ".  c #828817",
    "I  c #B0B60C",
    "(. c #E8EF0F",
    "H  c #5C5F0A",
    "x  c #5C5F0B",
    "L. c #C2C90E",
    "k. c #5C5F12",
    "K  c #2A463A",
    "xX c #211C13",
    "w. c #7E8062",
    "s. c #D9E00D",
    "8. c #D9E00E",
    "r  c #566060",
    "(  c #B8BE0D",
    "HX c #533B09",
    "{. c #0B0C04",
    "C. c #EBF30E",
    "*  c #6ED1D1",
    "}. c #454213",
    "T  c #3E4107",
    "p  c #427F7F",
    "t  c #6CD7D7",
    "W  c #010100",
    "mX c #996D0C",
    "4. c #767A0C",
    "D  c #010101",
    "M  c #3D7878",
    "lX c #101010",
    "OX c #767A0F",
    "+  c #7CFCFC",
    "4X c #4C4021",
    ">. c #343607",
    "$  c #52A1A1",
    "9  c #7CF9F9",
    "V. c #BBC20F",
    "<X c #CBA455",
    "a  c #458686",
    "1  c #6FDEDD",
    "1. c #5C7171",
    "_  c #D2D90E",
    "6  c #4D9797",
    "$. c #7E5C10",
    "r. c #E4EC0D",
    "O. c #131410",
    "N. c #90950B",
    "KX c #8B910C",
    "^. c #90950D",
    "J  c #53580C",
    "0. c #6F7309",
    "f. c #707252",
    "7X c #1D1C1B",
    "FX c #363636",
    "=X c #6F730D",
    "T. c #DAE10D",
    "M. c #4E5108",
    "-. c #4E5109",
    "bX c #312306",
    "*X c #393413",
    "G. c #81860D",
    "B  c #ECF40C",
    ";X c #656809",
    "E. c #ECF40D",
    "e. c #0C0D04",
    "nX c #72510A",
    "zX c #77550C",
    "w  c #75ECEC",
    "j. c #777B0B",
    "n. c #3F420C",
    "Q  c #020202",
    "[. c #070604",
    "P  c #3F4212",
    "I. c #353707",
    "b. c #C1C70D",
    "#  c #598787",
    "9X c #BD994F",
    ":. c #686C09",
    "JX c #BCC30F",
    "5  c #8E9090",
    "LX c #898E1B",
    " X c #0A0A04",
    "v  c #E5ED0D",
    "0  c #5D6F6F",
    "N  c #68C8C8",
    "). c #7F693E",
    ",  c #498E8D",
    "!. c #DBE20D",
    "x. c #2B2C17",
    "u. c #DBE20E",
    "<. c #D6DE0D",
    "ZX c #D6DE0E",
    "a. c #A3A90F",
    ",X c #322A1B",
    "g  c #7BF7F7",
    "#. c #2E3008",
    "z  c #59B0AF",
    "k  c #274444",
    "^  c #CCD30E",
    "2X c #030301",
    "&. c #030302",
    "5. c #ABB10D",
    "S. c #E3EA0E",
    "j  c #151C12",
    "-  c #4C9292",
    "%. c #302409",
    "_. c #BE9A51",
    "u  c #4F9C9B",
    "F. c #4D4F08",
    ">X c #D4DB0D",
    "VX c #7D5D2A",
    "jX c #D4DB0F",
    "=  c #949595",
    "|. c #B3B90C",
    "[  c #A37714",
    ";. c #C5CC0F",
    "   c None",
    "<  c #52A3A3",
    "Y. c #71750C",
    "R  c #A9AE0D",
    "3  c #7CFBFB",
    "h. c #D7DF0D",
    " . c #DCE30E",
    "g. c #0E0F02",
    "vX c #D0920D",
    "6. c #BBC10E",
    "iX c #513A09",
    "t. c #9A9F0C",
    "sX c #464808",
    "tX c #DFE70D",
    ".X c #DFE70E",
    "B. c #373908",
    "h  c #488C8C",
    "o  c #686969",
    "oX c #494C0A",
    "i  c #7AF5F5",
    "~. c #282A07",
    "eX c #0C0C04",
    "G  c #E7EF0D",
    "/. c #60630B",
    "n  c #020404",
    "BX c #815C0C",
    "1X c #725C31",
    "!  c #C6CD0D",
    "kX c #0C0C0C",
    "7  c #68D0D0",
    "qX c #515408",
    "+X c #9A7C41",
    "C  c #393939",
    "4  c #6DCEC0",
    "`  c #696A4F",
    "DX c #141403",
    ".. c #84890D",
    "$X c #7A6434",
    "m  c #4E9796",
    "&X c #262706",
    "AX c #050501",
    ",. c #42450C",
    "F  c #686B16",
    "MX c #595C0A",
    "wX c #987C42",
    "X. c #595C0D",
    "q. c #605A10",
    "Z  c #595C0F",
    "z. c #C4CA0F",
    ";  c #607878",
    "e  c #63BDBD",
    "SX c #9EA410",
    "uX c #D4940C",
    "9. c #7D820B",
    "2. c #55514C",
    "cX c #D4940D",
    "L  c #D6DD12",
    "/  c #080902",
    "A  c #B5BB0D",
    "q  c #6EDBDB",
    "hX c #94990D",
    "'  c #A57914",
    "&  c #585E5E",
    "GX c #553F1A",
    "}  c #73770A",
    "o. c #0D0D0D",
    "+. c #A6AC0C",
    "b  c #DEE50D",
    "J. c #6E7312",
    "Q. c #101102",
    "-X c #BDC30E",
    "7. c #101103",
    "@X c #4E4D4D",
    "v. c #676767",
    "{  c #64680A",
    "E  c #696C0C",
    "%  c #6ED2D2",
    "Y  c #484A0B",
    "P. c #767B0A",
    "0X c #7B7F0C",
    "fX c #5A5D08",
    "m. c #393B07",
    "`. c #C0C70D",
    "U. c #343708",
    "y  c #67CECE",
    "6X c #6C700C",
    "D. c #5A5D14",
    "]. c #C6A155",
    "H. c #6C700F",
    "5X c #6C7010",
    "d. c #4B4E0A",
    ">  c #838585",
    "=. c #2F3006",
    "Z. c #7E7E7D",
    "V  c #2A2C07",
    "#X c #B1B80D",
    "l. c #E9F10D",
    "@. c #E9F110",
    "8  c #55A9A8",
    "rX c #3C3F07",
    "R. c #2F300F",
    "O  c #458180",
    "NX c #3E3F1E",
    "S  c #202106",
    "@  c #7AF7F7",
    "X  c #434C4C",
    "pX c #4F390A",
    ":X c #65690D",
    "K. c #D0D70F",
    "*. c #D0D710",
    "2  c #A4A4A4",
    ")  c #AFB50D",
    "gX c #161613",
    "p. c #070707",
    "CX c #1B1404",
    "]  c #CC9414",
    "aX c #1B1406",
    "f  c #4B8F8E",
    "'. c #A0A60B",
    "A. c #191A07",
    "%X c #342B18",
    ":  c #468888",
    "|  c #303105",
    "W. c #646550",
    "i. c #EAF20D",
    "yX c #63660A",
    "3X c #614F2A",
    "XX c #EAF20E",
    "U  c #EAF20F",
    "s  c #9D9D9D",
    /* pixels */
    "                            . .             . X o               ",
    "                          O + @ #         $ % + % .             ",
    "                        & * + + % .     . % + + + =             ",
    "                        - + + + + ;     : + + + + >             ",
    "                        , + + + + <   . 1 + + + + 2             ",
    "                    .   . 3 + 4 + + 5 6 + 4 + + 7 .   .         ",
    "                          8 + 9 4 + 0 q w + e @ r   .           ",
    "                          . + t y + u i p a + s     .           ",
    "                        s d : + f g h j k l z                   ",
    "              . x c c c v b c n m M N B B V c C .               ",
    "            Z A S c D F G H c c c J K B L c c c P I             ",
    "          c B U c c Y B T c c c R B B E W c Q ! B B I .         ",
    "      . c W B ~ c c ^ B c c c / ( B B ) c c c _ B B B `         ",
    ". ' ] [ c { } c c | B  .c c c ..B B B X.c o.O.B B B +.@.#.      ",
    "  . $.%.&.*.=.c c -.B ;.c c c :.B B B c c c >.B B B ,.B <.1.2.. ",
    "      3.4.B c c c 5.B 6.c c 7.8.B B B c c c 9.B I 0.B B q.w..   ",
    "      c _ B c c e.r.B t.c c y.u.B B i.c c p.a.B s.d.B B B f.    ",
    "    . c B B c c g.h.B j.c c k.l.B B z.c c x.+.B B I I B +.c.v.  ",
    "    . c B b.c c n.B B m.c c M.B B B N.c c B.V.B B B I I I C.Z.  ",
    "    . A.B S.c c D.B B F.c c G.B B B H.c c J.K.B B B B B B L..   ",
    "    . c B P.c c I.B B U.c c Y.B B T.4.c c R.E.B B B B B B W.    ",
    "      Q.B !.c c ~.r.B ^.c c H B B C./.c c ,.(.B B B B B +..     ",
    "    )._.`.'.].[.{.E.}.|.c c  X.XB XXoXc c OXB B B B B B /.      ",
    "  ,.+X@X#X$X%Xc &XY.*X=Xc c ,.B B -X;Xc c :X>XB B B B K..       ",
    ". ,X,.,.<X1X2Xc 3X4XB B c c c 5XB B `.c D c 6XB B B B /.        ",
    "7X8X.   9X0XqXc wXeXB B c c c rXtXB B c c c yXB B +./.          ",
    ",.uXiXpXaXsXdXc c c B B fXc c gXhXB jXkXc c lXl.+./.            ",
    ". zXxXcXvXbXnXmXc c B B MXc c c NXB B c c c c /..               ",
    "      BXuXVXuXuXvXCXB B ZXAXc c c B B SXDXFX.                   ",
    "      . GX. ,.uXuXHXB B B JXc c c KXB LX.                       ",
    "            . ,.,..   . . . . . . .                             ",
    "                                                                "
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices type definition
//

IMPLEMENT_CLASS(frmScanforDevices, wxFrame)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices event table definition
//

BEGIN_EVENT_TABLE(frmScanforDevices, wxFrame)

////@begin frmScanforDevices event table entries
  EVT_CLOSE( frmScanforDevices::OnCloseWindow )
  EVT_MENU( ID_MENU_VSCPWORKS_EXIT, frmScanforDevices::OnMenuitemExitClick )
  EVT_MENU( ID_MENUITEM_HELP, frmScanforDevices::OnMenuitemHelpClick )
  EVT_MENU( ID_MENUITEM_HELP_FAQ, frmScanforDevices::OnMenuitemHelpFaqClick )
  EVT_MENU( ID_MENUITEM_HELP_SC, frmScanforDevices::OnMenuitemHelpScClick )
  EVT_MENU( ID_MENUITEM_HELP_THANKS, frmScanforDevices::OnMenuitemHelpThanksClick )
  EVT_MENU( ID_MENUITEM_HELP_CREDITS, frmScanforDevices::OnMenuitemHelpCreditsClick )
  EVT_MENU( ID_MENUITEM_HELP_VSCP_SITE, frmScanforDevices::OnMenuitemHelpVscpSiteClick )
  EVT_MENU( ID_MENUITEM_HELP_ABOUT, frmScanforDevices::OnMenuitemHelpAboutClick )
  EVT_TREE_SEL_CHANGED( ID_TREE_DEVICE, frmScanforDevices::OnTreeDeviceSelChanged )
  EVT_TREE_ITEM_RIGHT_CLICK( ID_TREE_DEVICE, frmScanforDevices::OnTreeDeviceItemRightClick )
  EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW3, frmScanforDevices::OnHtmlwindow3LinkClicked )
  EVT_BUTTON( ID_BUTTON_SCAN, frmScanforDevices::OnButtonScanClick )
////@end frmScanforDevices event table entries

EVT_MENU(Menu_Popup_GetNodeInfo, frmScanforDevices::getNodeInfo)
EVT_MENU(Menu_Popup_OpenConfig, frmScanforDevices::openConfiguration)

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices constructors
//

frmScanforDevices::frmScanforDevices()
{
    Init();
}

frmScanforDevices::frmScanforDevices(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices creator
//

bool frmScanforDevices::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxFrame::Create(parent, id, caption, pos, size, style);

    CreateControls();
    SetIcon(GetIconResource(wxT("fatbee_v2.ico")));
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices destructor
//

frmScanforDevices::~frmScanforDevices()
{
    ////@begin frmScanforDevices destruction
    ////@end frmScanforDevices destruction

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmScanforDevices::Init()
{
    ////@begin frmScanforDevices member initialisation
  m_pPanel = NULL;
  m_labelInterface = NULL;
  m_DeviceTree = NULL;
  m_htmlWnd = NULL;
  m_slowAlgorithm = NULL;
  m_ctrlEditFrom = NULL;
  m_ctrlEditTo = NULL;
    ////@end frmScanforDevices member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for frmScanforDevices
//

void frmScanforDevices::CreateControls()
{
    ////@begin frmScanforDevices content construction
  frmScanforDevices* itemFrame1 = this;

  wxMenuBar* menuBar = new wxMenuBar;
  wxMenu* itemMenu3 = new wxMenu;
  itemMenu3->AppendSeparator();
  itemMenu3->Append(ID_MENU_VSCPWORKS_EXIT, _("Exit"), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu3, _("File"));
  wxMenu* itemMenu6 = new wxMenu;
  itemMenu6->Append(ID_MENUITEM14, _("Scan for nodes..."), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu6, _("Tools"));
  wxMenu* itemMenu8 = new wxMenu;
  itemMenu8->Append(ID_MENUITEM_HELP, _("VSCP-Works Help"), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->Append(ID_MENUITEM_HELP_FAQ, _("Frequently Asked Questions"), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->Append(ID_MENUITEM_HELP_SC, _("Keyboard shortcuts"), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->AppendSeparator();
  itemMenu8->Append(ID_MENUITEM_HELP_THANKS, _("Thanks..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->Append(ID_MENUITEM_HELP_CREDITS, _("Credits..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->AppendSeparator();
  itemMenu8->Append(ID_MENUITEM_HELP_VSCP_SITE, _("Go to VSCP site"), wxEmptyString, wxITEM_NORMAL);
  itemMenu8->AppendSeparator();
  itemMenu8->Append(ID_MENUITEM_HELP_ABOUT, _("About"), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu8, _("Help"));
  itemFrame1->SetMenuBar(menuBar);

  m_pPanel = new wxPanel;
  m_pPanel->Create( itemFrame1, ID_PANEL_DEVICE_SCAN, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxVERTICAL);
  m_pPanel->SetSizer(itemBoxSizer20);

  wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer20->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_labelInterface = new wxStaticText;
  m_labelInterface->Create( m_pPanel, wxID_STATIC, _("Searching on interface: "), wxDefaultPosition, wxDefaultSize, 0 );
  m_labelInterface->SetName(wxT("m_staticInterfaceName"));
  itemBoxSizer21->Add(m_labelInterface, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer20->Add(itemBoxSizer23, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer23->Add(itemBoxSizer24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxStaticText* itemStaticText25 = new wxStaticText;
  itemStaticText25->Create( m_pPanel, wxID_STATIC, _("Found devices"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_DeviceTree = new wxTreeCtrl;
  m_DeviceTree->Create( m_pPanel, ID_TREE_DEVICE, wxDefaultPosition, wxSize(250, 340), wxTR_SINGLE|wxSIMPLE_BORDER );
  m_DeviceTree->SetBackgroundColour(wxColour(231, 235, 184));
  itemBoxSizer24->Add(m_DeviceTree, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer23->Add(itemBoxSizer27, 0, wxGROW|wxALL, 5);

  wxStaticText* itemStaticText28 = new wxStaticText;
  itemStaticText28->Create( m_pPanel, wxID_STATIC, _("Device information"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer27->Add(itemStaticText28, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_htmlWnd = new wxHtmlWindow;
  m_htmlWnd->Create( m_pPanel, ID_HTMLWINDOW3, wxDefaultPosition, wxSize(400, 340), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemBoxSizer27->Add(m_htmlWnd, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer23->Add(itemBoxSizer30, 0, wxALIGN_BOTTOM|wxALL, 5);

  m_slowAlgorithm = new wxCheckBox;
  m_slowAlgorithm->Create( m_pPanel, ID_CHECKBOX4, _("Slow"), wxDefaultPosition, wxDefaultSize, 0 );
  m_slowAlgorithm->SetValue(false);
  itemBoxSizer30->Add(m_slowAlgorithm, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText32 = new wxStaticText;
  itemStaticText32->Create( m_pPanel, wxID_STATIC, _("Scan from"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemStaticText32, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_ctrlEditFrom = new wxTextCtrl;
  m_ctrlEditFrom->Create( m_pPanel, ID_TEXTCTRL40, _("1"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(m_ctrlEditFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxStaticText* itemStaticText34 = new wxStaticText;
  itemStaticText34->Create( m_pPanel, wxID_STATIC, _("Scan to"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemStaticText34, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_ctrlEditTo = new wxTextCtrl;
  m_ctrlEditTo->Create( m_pPanel, ID_TEXTCTRL, _("255"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(m_ctrlEditTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  itemBoxSizer30->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 20);

  wxButton* itemButton37 = new wxButton;
  itemButton37->Create( m_pPanel, ID_BUTTON_SCAN, _("Scan"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer30->Add(itemButton37, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStatusBar* itemStatusBar38 = new wxStatusBar;
  itemStatusBar38->Create( m_pPanel, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
  itemStatusBar38->SetFieldsCount(2);
  itemBoxSizer20->Add(itemStatusBar38, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // Connect events and objects
  m_DeviceTree->Connect(ID_TREE_DEVICE, wxEVT_LEFT_DCLICK, wxMouseEventHandler(frmScanforDevices::OnLeftDClick), NULL, this);
    ////@end frmScanforDevices content construction
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmScanforDevices::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmScanforDevices::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    ////@begin frmScanforDevices bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end frmScanforDevices bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmScanforDevices::GetIconResource(const wxString& name)
{
    // Icon retrieval
    wxUnusedVar(name);
    if (name == _T("fatbee_v2.ico")) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }
    return wxNullIcon;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enableInterface 
//

bool frmScanforDevices::enableInterface(void)
{
    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Opening interface..."),
            100,
            this,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

    progressDlg.Pulse(_("opening interface..."));

    if (0 != m_csw.doCmdOpen()) {

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

            }

        } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

            //m_staticComboText->SetLabel(_("Interface"));

            progressDlg.Pulse(_("Checking if interface is working..."));

            // Test
            if (CANAL_ERROR_SUCCESS != m_csw.doCmdNOOP()) {
                wxMessageBox(_("Interface test command failed. May not work correctly!"));
            }

            // TCP/IP interface
            progressDlg.Pulse(_("TCP/IP Interface Open"));

            ///////////////////////////////////////////
            // Fill listbox with available interfaces
            ///////////////////////////////////////////

            // Get the interface list
            wxString wxstr;
            wxArrayString strarray;

            // Get VSCP interface list
            progressDlg.Pulse(_("TCP/IP Interface Open"));

            m_csw.getTcpIpInterface()->doCmdInterfaceList(strarray);

            if (strarray.Count()) {

                //m_comboNodeID->Clear();
                for (unsigned int i = 0; i < strarray.Count(); i++) {
                    wxStringTokenizer tkz(strarray[i], _(","));
                    wxString strOrdinal = tkz.GetNextToken();
                    wxString strType = tkz.GetNextToken();
                    wxString strGUID = tkz.GetNextToken();
                    wxString strDescription = tkz.GetNextToken();
                    wxString strLine = strGUID;
                    strLine += _(" ");
                    strLine += _(" Type = ");
                    strLine += strType;
                    strLine += _(" - ");
                    strLine += strDescription;

                    //m_comboNodeID->Append(strLine);
                }

                // Select the first item
                //m_comboNodeID->SetSelection(0);

            } else {
                wxMessageBox(_("No interfaces found!"));
            }

        } // TCP/IP interface

    } else {
        progressDlg.Pulse(_("Failed to open Interface."));
        wxMessageBox(_("Failed to Open Interface."));

        //m_BtnActivateInterface->SetValue(false);
        //m_BtnActivateInterface->SetLabel(_("Disconnected"));
        return false;
    }

    // Move to top of zorder
    Raise();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disableInterface
//

bool frmScanforDevices::disableInterface(void)
{
    m_csw.doCmdClose();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemExitClick
//

void frmScanforDevices::OnMenuitemExitClick(wxCommandEvent& event)
{
    Close();
    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmScanforDevices::OnCloseWindow(wxCloseEvent& event)
{

    event.Skip();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpClick
//

void frmScanforDevices::OnMenuitemHelpClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpFaqClick
//

void frmScanforDevices::OnMenuitemHelpFaqClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpScClick
//

void frmScanforDevices::OnMenuitemHelpScClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpThanksClick
//

void frmScanforDevices::OnMenuitemHelpThanksClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpCreditsClick
//

void frmScanforDevices::OnMenuitemHelpCreditsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpVscpSiteClick
//

void frmScanforDevices::OnMenuitemHelpVscpSiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpAboutClick
//

void frmScanforDevices::OnMenuitemHelpAboutClick(wxCommandEvent& event)
{
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonScanClick
//

void frmScanforDevices::OnButtonScanClick(wxCommandEvent& event)
{
    bool bSlowAlgorithm = false;
    uint8_t val;
    uint8_t reg[256];
    CMDF mdf;
    wxString url;
    wxTreeItemId newitem;

    ::wxBeginBusyCursor();
    
    bSlowAlgorithm = m_slowAlgorithm->GetValue();
    
    uint8_t scanFrom = vscp_readStringValue(m_ctrlEditFrom->GetValue());
    uint8_t scanTo = vscp_readStringValue(m_ctrlEditTo->GetValue());
    
    if ( scanFrom >=  scanTo ) {
        wxMessageBox(_("Node to scan from must be less then to"));
        return;
    }

    m_DeviceTree->DeleteAllItems();

    wxProgressDialog progressDlg(_("Scanning for VSCP devices"),
            _("Reading Registers"),
            scanTo-scanFrom+1,
            this,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE  | wxPD_CAN_ABORT);

    wxTreeItemId rootItem = m_DeviceTree->AddRoot(_("Found device(s)"));
    m_DeviceTree->ExpandAll();

    // Fetch GUID for the interface
	if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {
		fetchIterfaceGUID();
	}

    if ( bSlowAlgorithm ) {

        for ( uint8_t i = scanFrom; i < scanTo; i++ ) {

            if (!progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i))) {
                if (m_DeviceTree->GetCount()) {
                    wxTreeItemIdValue cookie;
                    wxTreeItemId item = m_DeviceTree->GetFirstChild(m_DeviceTree->GetRootItem(), cookie);
                    if ( item.IsOk() ) m_DeviceTree->SelectItem( item );
                }
                ::wxEndBusyCursor();
                break;
            }

            if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

                if (m_csw.readLevel1Register(i, 0xd0, &val, &progressDlg)) {

                    newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), i));
                    m_DeviceTree->ExpandAll();
                    memset(reg, 0, sizeof(reg));
                    //m_csw.readLevel2Registers(this,
                    //                            reg,
                    //                            sizeof(reg) );
                    //::wxGetApp().readAllLevel1Registers(this, &m_csw, reg, i);
                    //::wxGetApp().loadMDF( this, &m_csw, &mdf, url, &i );
                    //wxString str = ::wxGetApp().getHtmlStatusInfo(&m_csw, reg);
                    //m_htmlWnd->SetPage(str);

                    scanElement *pElement = new scanElement;
                    if (NULL != pElement) {
                        pElement->m_bLoaded = false;
                        pElement->m_nodeid = i;
                        //pElement->m_html = str; 
                        memset(pElement->m_reg, 0, 256);
                        m_DeviceTree->SetItemData(newitem, pElement);
                    }
                }

            } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

                cguid destguid;
                destguid.setLSB(i);
                if (m_csw.readLevel2Register(m_ifguid,
                        0xd0,
                        &val,
                        &destguid,
                        &progressDlg)) {

                    newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), i));
                    m_DeviceTree->ExpandAll();
                    //::wxGetApp().readAllLevel2Registers(this, &m_csw, reg, interfaceGUID);
                    //::wxGetApp().loadMDF( this, &m_csw, &mdf, url, &i );
                    //wxString str = ::wxGetApp().getHtmlStatusInfo(&m_csw, reg);
                    //m_htmlWnd->SetPage(str);
                    
                    scanElement *pElement = new scanElement;
                    if (NULL != pElement) {
                        pElement->m_bLoaded = false;
                        pElement->m_nodeid = i;
                        //pElement->m_html = str; 
                        memset(pElement->m_reg, 0, 256);
                        m_DeviceTree->SetItemData(newitem, pElement);
                    }
                     
                }

            }

            ::wxSafeYield();

        } // for
    }
    else { // Fast Algorithm

        vscpEventEx eventex;

		if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

			// Send read register to all nodes.
			for ( uint8_t i = scanFrom; i < scanTo; i++ ) {

#ifdef WIN32				
				progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i));
#endif				

				eventex.vscp_class = VSCP_CLASS1_PROTOCOL;
				eventex.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;
				eventex.sizeData = 2;		// nodeid + register to read
				eventex.data[ 0 ] = i;		// nodeid
				eventex.data[ 1 ] = 0xd0;	// Register to read
				m_csw.doCmdSend( &eventex );
			}


			// Check for replies
			wxLongLong resendTime = ::wxGetLocalTimeMillis();
       
			std::list<int> found_list;
			bool bLevel2 = false;
			uint8_t cnt = 0; 
			while (true) {
            
				progressDlg.Pulse( wxString::Format(_("Found %d"), found_list.size()));

				if (m_csw.doCmdDataAvailable()) { // Message available

					if (CANAL_ERROR_SUCCESS == m_csw.doCmdReceive(&eventex)) { // Valid event
                    
#if 0                    
							{
							wxString str;
								str = wxString::Format(_("Received Event: class=%d type=%d size=%d data=%d %d"), 
									eventex.vscp_class, eventex.vscp_type, eventex.sizeData, eventex.data[15], eventex.data[16] );
								wxLogDebug(str);
							}
#endif                    
							// Level I Read reply?
							if ((VSCP_CLASS1_PROTOCOL == eventex.vscp_class) &&
									(VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type)) {
								if ( 0xd0 == eventex.data[ 0 ] ) { // Requested register?
									// Add nickname to list 
									found_list.push_back( eventex.GUID[15] );
								}	// Check for correct node
							}		// Level II 512 Read reply?

						} // valid event

					} //Event is available

					if ((::wxGetLocalTimeMillis() - resendTime) > 1000 ) {

						// Take away duplicates
						found_list.unique();
                
						wxTreeItemId newitem;
						for( std::list<int>::iterator list_iter = found_list.begin(); 
							    list_iter != found_list.end(); list_iter++) {
                    
							newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), *list_iter));
							m_DeviceTree->ExpandAll();
                    
							scanElement *pElement = new scanElement;
							if (NULL != pElement) {
								pElement->m_bLoaded = false;
								pElement->m_nodeid = *list_iter;
								pElement->m_html = _("Right click on item to load info about node."); 
								memset(pElement->m_reg, 0, 256);
								m_DeviceTree->SetItemData(newitem, pElement);
							}
						}
						break;

					}

			} // while

		
		} // CANAL
		else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
            
			// Read register at all nodes.
			for ( uint8_t i = scanFrom; i < scanTo; i++ ) {
            
				cguid destguid;
				destguid.setLSB(i);
                
				eventex.head = VSCP_PRIORITY_NORMAL;
				eventex.timestamp = 0;
				eventex.obid = 0;
                
				// Check if a specific interface is used
				if ( !m_ifguid.isNULL() ) {

					progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i));

					eventex.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
					eventex.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

					memset(eventex.GUID, 0, 16);// We use GUID for interface 
					eventex.sizeData = 16 + 2;  // Interface GUID + nodeid + register to read

					m_ifguid.writeGUID(eventex.data);

					eventex.data[ 16 ] = i; // nodeid
					eventex.data[ 17 ] = 0xd0; // Register to read

					m_csw.doCmdSend( &eventex );

				}
			
			} // for
                

			// Check for replies
			wxLongLong resendTime = ::wxGetLocalTimeMillis();
       
			std::list<int> found_list;
			bool bLevel2 = false;
			uint8_t cnt = 0; 
			while (true) {
            
				progressDlg.Pulse( wxString::Format(_("Found %d"), found_list.size()));

				if (m_csw.doCmdDataAvailable()) { // Message available

					if (CANAL_ERROR_SUCCESS == m_csw.doCmdReceive(&eventex)) { // Valid event
                    
#if 0                    
							{
							wxString str;
								str = wxString::Format(_("Received Event: class=%d type=%d size=%d data=%d %d"), 
									eventex.vscp_class, eventex.vscp_type, eventex.sizeData, eventex.data[15], eventex.data[16] );
								wxLogDebug(str);
							}
#endif                    

						// Level I Read reply?
						if (/*ifGUID.isNULL() &&*/ (VSCP_CLASS1_PROTOCOL == eventex.vscp_class) &&
								(VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type)) {
							if (eventex.data[ 0 ] == 0xd0) { // Requested register?
								// Add nickname to list 
								found_list.push_back( eventex.GUID[ 15 ] );
							} // Check for correct node
						}                        // Level II 512 Read reply?
						else if (/*!m_ifguid.isNULL() && !bLevel2 &&*/
								(VSCP_CLASS2_LEVEL1_PROTOCOL == eventex.vscp_class) &&
							(VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type)) {

							//if ( pdestGUID->isSameGUID( event.GUID ) ) {
							// Reg we requested?
							if (0xd0 == eventex.data[ 16 ] ) {
								// Add nickname to list 
								found_list.push_back( eventex.GUID[ 15 ] );
							}
							//}

						}                        // Level II Read reply?
						else if (m_ifguid.isNULL() && bLevel2 &&
								(VSCP_CLASS2_PROTOCOL == eventex.vscp_class) &&
								(VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == 
							eventex.vscp_type)) {

							// from us
							uint32_t retreg = (eventex.data[ 0 ] << 24) +
												(eventex.data[ 1 ] << 16) +
												(eventex.data[ 2 ] << 8) +
												eventex.data[ 3 ];

							// Register we requested?
							if (retreg == 0xffffffd0) {
								// Add nickname to list 
								found_list.push_back( eventex.data[ 15 ] );
							}
						}

					} // valid event

				} //Event is available

				if ((::wxGetLocalTimeMillis() - resendTime) > 1000 ) {

					// Take away duplicates
					found_list.unique();
                
					wxTreeItemId newitem;
					for ( std::list<int>::iterator list_iter = found_list.begin(); 
							list_iter != found_list.end(); list_iter++) {
                    
						newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), *list_iter));
						m_DeviceTree->ExpandAll();
                    
						scanElement *pElement = new scanElement;
						if (NULL != pElement) {
							pElement->m_bLoaded = false;
							pElement->m_nodeid = *list_iter;
							pElement->m_html = _("Right click on item to load info about node."); 
							memset(pElement->m_reg, 0, 256);
							m_DeviceTree->SetItemData(newitem, pElement);
						}
					}
					break;
            
				} // while
        
			}
        
		} // TCP/IP i/f     
		    
        
    }  // fast

    if (m_DeviceTree->GetCount()) {
        m_DeviceTree->SelectItem(m_DeviceTree->GetRootItem());
    }

    ::wxEndBusyCursor();

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTreeDeviceItemRightClick
//

void frmScanforDevices::OnTreeDeviceItemRightClick(wxTreeEvent& event)
{
    // Ask if we should display info
    wxMenu menu;

    menu.Append(Menu_Popup_GetNodeInfo, _T("Update node info from MDF..."));
    menu.Append(Menu_Popup_OpenConfig, _T("Open configuration window..."));
    PopupMenu(&menu);

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnLeftDClick
//

void frmScanforDevices::OnLeftDClick(wxMouseEvent& event)
{

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getNodeInfo
//

void frmScanforDevices::getNodeInfo(wxCommandEvent& event)
{
    CMDF mdf;
    wxString url;

    scanElement *pElement =
            (scanElement *) m_DeviceTree->GetItemData(m_DeviceTree->GetSelection());
    
    if (NULL != pElement) {
        
        wxProgressDialog progressDlg(_("Getting node info."),
                        _("Reading Registers"),
                        256,
                        this,
                        wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE  | wxPD_CAN_ABORT);

		if (INTERFACE_CANAL == m_interfaceType) {
		
			bool bregs = m_csw.readLevel1Registers( this,
														pElement->m_reg,
														pElement->m_nodeid,
														0,
														256,
														&progressDlg );
		
			uint8_t preg_url[33];
		    memset( preg_url, 0, sizeof(preg_url));
			memcpy( preg_url, pElement->m_reg + 0xe0, 32 );
			bool bmdf = m_csw.loadMDF( this,
				                        preg_url,
					                    url,
						                &mdf );
                
			pElement->m_html = vscp_getDeviceHtmlStatusInfo( pElement->m_reg, bmdf ? &mdf : NULL );
			m_htmlWnd->SetPage(pElement->m_html);
        
			// Mark as loaded
			if ( bregs ) pElement->m_bLoaded = true;

		} 
        else if (INTERFACE_VSCP == m_interfaceType) {
        
			cguid destguid;
			destguid.setLSB( pElement->m_nodeid );
        
			bool bregs = m_csw.readLevel2Registers( this,
				                                        pElement->m_reg,
					                                    m_ifguid,
						                                0,
							                            256,
								                        &destguid,
									                    &progressDlg,
										                false );
            // If read fails abort
            if (!bregs) return;

	        uint8_t preg_url[33];
		    memset( preg_url, 0, sizeof(preg_url));
			memcpy( preg_url, pElement->m_reg + 0xe0, 32 );
			bool bmdf = m_csw.loadMDF( this,
				                        preg_url,
					                    url,
						                &mdf );
                
			pElement->m_html = vscp_getDeviceHtmlStatusInfo( pElement->m_reg, bmdf ? &mdf : NULL );
			m_htmlWnd->SetPage(pElement->m_html);
        
			// Mark as loaded
			if ( bregs ) pElement->m_bLoaded = true;

		}

	}

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// openConfiguration
//

void frmScanforDevices::openConfiguration(wxCommandEvent& event)
{
    frmDeviceConfig *subframe = new frmDeviceConfig(this/*->GetParent()*/, 2000, _("VSCP Configuration window"));

    if (INTERFACE_CANAL == m_interfaceType) {

        // Hide the Level II checkbox
        subframe->m_bLevel2->Show(false);

        // Init node id combo
        wxRect rc = subframe->m_comboNodeID->GetRect();
        rc.SetWidth(60);
        subframe->m_comboNodeID->SetSize(rc);

        for (int i = 1; i < 256; i++) {
            subframe->m_comboNodeID->Append(wxString::Format(_("0x%02x"), i));
        }

        subframe->m_comboNodeID->SetValue(_("0x01"));
        subframe->SetTitle(_("VSCP Registers (CANAL) - ") +
                m_canalif.m_strDescription);

        subframe->m_csw.setInterface(m_canalif.m_strDescription,
                m_canalif.m_strPath,
                m_canalif.m_strConfig,
                m_canalif.m_flags, 0, 0);

        scanElement *pElement =
                (scanElement *) m_DeviceTree->GetItemData(m_DeviceTree->GetSelection());
        if (NULL != pElement) {
            subframe->m_comboNodeID->SetSelection(pElement->m_nodeid - 1);
        }

        // Close our interface
        m_csw.doCmdClose();

        // Connect to device bus
        subframe->enableInterface();

        //subframe->OnInterfaceActivate( ev );

        wxCommandEvent ev;
        subframe->OnButtonUpdateClick(ev);

        // Move window on top
        subframe->Raise();

        // Show the VSCP configuration windows
        subframe->Show(true);

        // Close the scan window
        Show(false);

    } else if (INTERFACE_VSCP == m_interfaceType) {

        wxString str;
        unsigned char GUID[16];
        memcpy(GUID, m_vscpif.m_GUID, 16);

        // Fill the combo
        for (int i = 1; i < 256; i++) {
            GUID[0] = i;
            vscp_writeGuidArrayToString(GUID, str);
            subframe->m_comboNodeID->Append(str);
        }

        GUID[0] = 0x01;
        vscp_writeGuidArrayToString(GUID, str);
        subframe->m_comboNodeID->SetValue(str);

        subframe->SetTitle(_("VSCP Registers (TCP/IP)- ") +
                m_vscpif.m_strDescription);

        // If server username is given and no password is entered we ask for it.
        if (m_vscpif.m_strPassword.IsEmpty() &&
                !m_vscpif.m_strUser.IsEmpty()) {
            m_vscpif.m_strPassword =
                    ::wxGetTextFromUser(_("Please enter password"),
                    _("Connection Test"));
        }

        //subframe->m_csw = m_interfaceType;
        subframe->m_csw.setInterface(m_vscpif.m_strHost,
                m_vscpif.m_strUser,
                m_vscpif.m_strPassword);

        // Connect to host
        subframe->enableInterface();

        // Show the VSCP configuration windows
        subframe->Show(true);

        // Move window on top
        subframe->Raise();

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTreeDeviceSelChanged
//

void frmScanforDevices::OnTreeDeviceSelChanged(wxTreeEvent& event)
{
    scanElement *pElement = (scanElement *) m_DeviceTree->GetItemData(m_DeviceTree->GetSelection());
    if (NULL != pElement) m_htmlWnd->SetPage(pElement->m_html);
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW3
//

void frmScanforDevices::OnHtmlwindow3LinkClicked(wxHtmlLinkEvent& event)
{
    if (event.GetLinkInfo().GetHref().StartsWith(_("http://"))) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

bool frmScanforDevices::fetchIterfaceGUID(void) {
    wxString str;

    if (!m_csw.isOpen()) {
        wxMessageBox(_("TCP/IP connection to daemon must be open."));
        return false;
    }

    if (USE_TCPIP_INTERFACE != m_csw.getDeviceType()) {
        wxMessageBox(_("Interfaces can only be fetched from the VSCP daemon."));
        return false;
    }

    // Get the interface list
    wxArrayString ifarray;
    if (CANAL_ERROR_SUCCESS ==
            m_csw.getTcpIpInterface()->doCmdInterfaceList(ifarray)) {

        if (ifarray.Count()) {

            for (unsigned int i = 0; i < ifarray.Count(); i++) {

                wxStringTokenizer tkz(ifarray[ i ], _(","));
                wxString strOrdinal = tkz.GetNextToken();
                wxString strType = tkz.GetNextToken();
                wxString strIfGUID = tkz.GetNextToken();
                wxString strDescription = tkz.GetNextToken();

                int pos;
                wxString strName;
                if (wxNOT_FOUND != (pos = strDescription.Find(_(" ")))) {
                    strName = strDescription.Left(pos);
                    strName.Trim();
                }

                if (strName.Upper() == m_vscpif.m_strInterfaceName.Upper()) {

                    // Save interface GUID;
                    m_ifguid.getFromString(strIfGUID);

                    return true;
                }

            }

        } else {
            wxMessageBox(_("No interfaces found."));
        }
    } else {
        wxMessageBox(_("Unable to get interface list from VSCP daemon."));
    }

    return false;
}
