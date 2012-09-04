/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscptrmit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/11/2007 17:46:01
// RCS-ID:      
// Copyright:   (C) 2007-2011 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
//  Grodans Paradis AB at info@Grodans Paradis AB.se, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgvscptrmit.h"
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

#include <wx/tokenzr.h>
#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "vscptxobj.h"
#include "dlgvscptrmit.h"

////@begin XPM images
/* XPM */
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

////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgVSCPTrmitElement, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement event table definition
//

BEGIN_EVENT_TABLE( dlgVSCPTrmitElement, wxDialog )

////@begin dlgVSCPTrmitElement event table entries
  EVT_INIT_DIALOG( dlgVSCPTrmitElement::OnInitDialog )
  EVT_CLOSE( dlgVSCPTrmitElement::OnCloseWindow )

  EVT_COMBOBOX( ID_StrVscpClass, dlgVSCPTrmitElement::OnStrVscpClassSelected )

////@end dlgVSCPTrmitElement event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement constructors
//

dlgVSCPTrmitElement::dlgVSCPTrmitElement()
{
  Init();
}

dlgVSCPTrmitElement::dlgVSCPTrmitElement( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement creator
//

bool dlgVSCPTrmitElement::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgVSCPTrmitElement creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgVSCPTrmitElement creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement destructor
//

dlgVSCPTrmitElement::~dlgVSCPTrmitElement()
{
////@begin dlgVSCPTrmitElement destruction
////@end dlgVSCPTrmitElement destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgVSCPTrmitElement::Init()
{
////@begin dlgVSCPTrmitElement member initialisation
  m_wxChkActive = NULL;
  m_wxStrName = NULL;
  m_wxComboClass = NULL;
  m_wxComboType = NULL;
  m_wxChoicePriority = NULL;
  m_wxChkDefaultGUID = NULL;
  m_wxStrGUID15 = NULL;
  m_wxStrGUID14 = NULL;
  m_wxStrGUID13 = NULL;
  m_wxStrGUID12 = NULL;
  m_wxStrGUID11 = NULL;
  m_wxStrGUID10 = NULL;
  m_wxStrGUID9 = NULL;
  m_wxStrGUID8 = NULL;
  m_wxStrGUID7 = NULL;
  m_wxStrGUID6 = NULL;
  m_wxStrGUID5 = NULL;
  m_wxStrGUID4 = NULL;
  m_wxStrGUID3 = NULL;
  m_wxStrGUID2 = NULL;
  m_wxStrGUID1 = NULL;
  m_wxStrGUID0 = NULL;
  m_wxStrData = NULL;
  m_wxStrCount = NULL;
  m_wxStrPeriod = NULL;
  m_wxComboTrigger = NULL;
////@end dlgVSCPTrmitElement member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgVSCPTrmitElement
//

void dlgVSCPTrmitElement::CreateControls()
{    
////@begin dlgVSCPTrmitElement content construction
  dlgVSCPTrmitElement* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxALL, 5);

  itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_TOP|wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizer7 = new wxFlexGridSizer(2, 2, 0, 0);
  itemBoxSizer6->Add(itemFlexGridSizer7, 1, wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  m_wxChkActive = new wxCheckBox;
  m_wxChkActive->Create( itemDialog1, ID_CHECKBOX1, _("Active"), wxDefaultPosition, wxDefaultSize, 0 );
  m_wxChkActive->SetValue(true);
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxChkActive->SetToolTip(_("Tick to activate the transmission event"));
  m_wxChkActive->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemFlexGridSizer7->Add(m_wxChkActive, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( itemDialog1, wxID_STATIC, _("Name :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText10->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrName = new wxTextCtrl;
  m_wxStrName->Create( itemDialog1, ID_StrName, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrName->SetToolTip(_("All transmission event should have a \n\n descriptive name. \n\nSet this name here."));
  itemFlexGridSizer7->Add(m_wxStrName, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText14 = new wxStaticText;
  itemStaticText14->Create( itemDialog1, wxID_STATIC, _("Class :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText14->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_wxComboClassStrings;
  m_wxComboClass = new wxComboBox;
  m_wxComboClass->Create( itemDialog1, ID_StrVscpClass, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboClassStrings, wxCB_READONLY );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxComboClass->SetToolTip(_("Select the VSCP class for the \ntransmission event here."));
  itemFlexGridSizer7->Add(m_wxComboClass, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemDialog1, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText16->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_wxComboTypeStrings;
  m_wxComboType = new wxComboBox;
  m_wxComboType->Create( itemDialog1, ID_StrVscpType, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboTypeStrings, wxCB_READONLY );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxComboType->SetToolTip(_("Select the VSCP type for the \ntransmission event here."));
  itemFlexGridSizer7->Add(m_wxComboType, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Priority :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText18->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_wxChoicePriorityStrings;
  m_wxChoicePriorityStrings.Add(_("0 - Highest"));
  m_wxChoicePriorityStrings.Add(_("1 - Even higher"));
  m_wxChoicePriorityStrings.Add(_("2 - Higher"));
  m_wxChoicePriorityStrings.Add(_("3 - Normal high"));
  m_wxChoicePriorityStrings.Add(_("4 - Normal low"));
  m_wxChoicePriorityStrings.Add(_("5 - Lower"));
  m_wxChoicePriorityStrings.Add(_("6 - Even lower"));
  m_wxChoicePriorityStrings.Add(_("7 - Lowest"));
  m_wxChoicePriority = new wxChoice;
  m_wxChoicePriority->Create( itemDialog1, ID_WxChoicePriority, wxDefaultPosition, wxSize(120, -1), m_wxChoicePriorityStrings, 0 );
  m_wxChoicePriority->SetStringSelection(_("0"));
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxChoicePriority->SetToolTip(_("Select a priority for the \ntransmission event here. \nMost receiving applications will ignore this field."));
  itemFlexGridSizer7->Add(m_wxChoicePriority, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 20, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText23 = new wxStaticText;
  itemStaticText23->Create( itemDialog1, wxID_STATIC, _("Originating GUID"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText23->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText23, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  m_wxChkDefaultGUID = new wxCheckBox;
  m_wxChkDefaultGUID->Create( itemDialog1, ID_CHKBOX_USE_DEFAULT, _("Use default GUID"), wxDefaultPosition, wxDefaultSize, 0 );
  m_wxChkDefaultGUID->SetValue(true);
  m_wxChkDefaultGUID->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Sans")));
  itemFlexGridSizer7->Add(m_wxChkDefaultGUID, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText26 = new wxStaticText;
  itemStaticText26->Create( itemDialog1, wxID_STATIC, _("15 - 8 :"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  itemStaticText26->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer7->Add(itemBoxSizer27, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID15 = new wxTextCtrl;
  m_wxStrGUID15->Create( itemDialog1, ID_StrGUID15, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID15->SetToolTip(_("GUID byte 15"));
  itemBoxSizer27->Add(m_wxStrGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID14 = new wxTextCtrl;
  m_wxStrGUID14->Create( itemDialog1, ID_WxStrGUID14, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID14->SetToolTip(_("GUID byte 14"));
  itemBoxSizer27->Add(m_wxStrGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID13 = new wxTextCtrl;
  m_wxStrGUID13->Create( itemDialog1, ID_WxStrGUID13, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID13->SetToolTip(_("GUID byte 13"));
  itemBoxSizer27->Add(m_wxStrGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID12 = new wxTextCtrl;
  m_wxStrGUID12->Create( itemDialog1, ID_WxStrGUID12, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID12->SetToolTip(_("GUID byte 12"));
  itemBoxSizer27->Add(m_wxStrGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID11 = new wxTextCtrl;
  m_wxStrGUID11->Create( itemDialog1, ID_WxStrGUID11, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID11->SetToolTip(_("GUID byte 11"));
  itemBoxSizer27->Add(m_wxStrGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID10 = new wxTextCtrl;
  m_wxStrGUID10->Create( itemDialog1, ID_WxStrGUID10, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID10->SetToolTip(_("GUID byte 10"));
  itemBoxSizer27->Add(m_wxStrGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID9 = new wxTextCtrl;
  m_wxStrGUID9->Create( itemDialog1, ID_WxStrGUID9, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID9->SetToolTip(_("GUID byte 9"));
  itemBoxSizer27->Add(m_wxStrGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID8 = new wxTextCtrl;
  m_wxStrGUID8->Create( itemDialog1, ID_WxStrGUID8, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID8->SetToolTip(_("GUID byte 8"));
  itemBoxSizer27->Add(m_wxStrGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText36 = new wxStaticText;
  itemStaticText36->Create( itemDialog1, wxID_STATIC, _("7 - 0 :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText36->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer7->Add(itemBoxSizer37, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID7 = new wxTextCtrl;
  m_wxStrGUID7->Create( itemDialog1, ID_WxStrGUID7, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID7->SetToolTip(_("GUID byte 7"));
  itemBoxSizer37->Add(m_wxStrGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID6 = new wxTextCtrl;
  m_wxStrGUID6->Create( itemDialog1, ID_WxStrGUID6, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID6->SetToolTip(_("GUID byte 6"));
  itemBoxSizer37->Add(m_wxStrGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID5 = new wxTextCtrl;
  m_wxStrGUID5->Create( itemDialog1, ID_WxStrGUID5, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID5->SetToolTip(_("GUID byte 5"));
  itemBoxSizer37->Add(m_wxStrGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID4 = new wxTextCtrl;
  m_wxStrGUID4->Create( itemDialog1, ID_WxStrGUID4, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID4->SetToolTip(_("GUID byte 4"));
  itemBoxSizer37->Add(m_wxStrGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID3 = new wxTextCtrl;
  m_wxStrGUID3->Create( itemDialog1, ID_WxStrGUID3, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID3->SetToolTip(_("GUID byte 3"));
  itemBoxSizer37->Add(m_wxStrGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID2 = new wxTextCtrl;
  m_wxStrGUID2->Create( itemDialog1, ID_WxStrGUID2, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_wxStrGUID2->SetHelpText(_("GUID byte 2"));
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID2->SetToolTip(_("GUID byte 2"));
  itemBoxSizer37->Add(m_wxStrGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID1 = new wxTextCtrl;
  m_wxStrGUID1->Create( itemDialog1, ID_WxStrGUID1, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID1->SetToolTip(_("GUID byte 1"));
  itemBoxSizer37->Add(m_wxStrGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID0 = new wxTextCtrl;
  m_wxStrGUID0->Create( itemDialog1, ID_WxStrGUID0, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrGUID0->SetToolTip(_("GUID byte 0"));
  itemBoxSizer37->Add(m_wxStrGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText46 = new wxStaticText;
  itemStaticText46->Create( itemDialog1, wxID_STATIC, _("Data :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText46->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrData = new wxTextCtrl;
  m_wxStrData->Create( itemDialog1, ID_StrVscpData, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrData->SetToolTip(_("You enter data for the transmission event\n as a comma separated list. \nYou can arrange the data over several lines. \nHexadecimal values should be preceded with '0x'."));
  itemFlexGridSizer7->Add(m_wxStrData, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 20, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText50 = new wxStaticText;
  itemStaticText50->Create( itemDialog1, wxID_STATIC, _("Count :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText50->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText50, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrCount = new wxTextCtrl;
  m_wxStrCount->Create( itemDialog1, ID_StrCount, _("1"), wxDefaultPosition, wxSize(60, -1), 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrCount->SetToolTip(_("Set event transmission event here. \nThis value sets how many times \nthis transmission event will be sent \nwhen triggered or doubleclicked."));
  itemFlexGridSizer7->Add(m_wxStrCount, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

  wxStaticText* itemStaticText52 = new wxStaticText;
  itemStaticText52->Create( itemDialog1, wxID_STATIC, _("Period :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText52->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrPeriod = new wxTextCtrl;
  m_wxStrPeriod->Create( itemDialog1, ID_StrPeriod, _("0"), wxDefaultPosition, wxSize(60, -1), 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrPeriod->SetToolTip(_("Set transmission event period here expressed as milliseconds. \nThe transmission event period is  interval between automatically sent events."));
  itemFlexGridSizer7->Add(m_wxStrPeriod, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

  wxStaticText* itemStaticText54 = new wxStaticText;
  itemStaticText54->Create( itemDialog1, wxID_STATIC, _("Trigger :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText54->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText54, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_wxComboTriggerStrings;
  m_wxComboTrigger = new wxComboBox;
  m_wxComboTrigger->Create( itemDialog1, ID_ComboTrigger, wxEmptyString, wxDefaultPosition, wxSize(200, -1), m_wxComboTriggerStrings, wxCB_DROPDOWN );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxComboTrigger->SetToolTip(_("Select a predefined trigger here that will do send of this transmission event."));
  itemFlexGridSizer7->Add(m_wxComboTrigger, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer56 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer56, 0, wxALIGN_BOTTOM|wxALL, 1);

  wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer56->Add(itemBoxSizer57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton58 = new wxButton;
  itemButton58->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer57->Add(itemButton58, 0, wxALIGN_RIGHT|wxALL, 1);

  wxButton* itemButton59 = new wxButton;
  itemButton59->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer57->Add(itemButton59, 0, wxALIGN_RIGHT|wxALL, 1);

////@end dlgVSCPTrmitElement content construction


  fillClassCombo();

  // Select first item
  m_wxComboClass->SetSelection( 0 );

  fillTypeCombo( 0 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
//

void dlgVSCPTrmitElement::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT in dlgVSCPTrmitElement.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT in dlgVSCPTrmitElement. 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
//

void dlgVSCPTrmitElement::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT in dlgVSCPTrmitElement.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT in dlgVSCPTrmitElement. 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgVSCPTrmitElement::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgVSCPTrmitElement::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgVSCPTrmitElement bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgVSCPTrmitElement bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgVSCPTrmitElement::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgVSCPTrmitElement icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
////@end dlgVSCPTrmitElement icon retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getEvent
//

bool dlgVSCPTrmitElement::getEventData( VscpTXObj *pObj )
{
    wxString str;
  
    // Check pointer
    if ( NULL == pObj ) return false;
  
    pObj->m_bActive = m_wxChkActive->GetValue();
    pObj->m_wxStrName = m_wxStrName->GetValue(); 
    pObj->m_bUseDefaultGUID = m_wxChkDefaultGUID->GetValue();
    str = m_wxStrCount->GetValue();
    pObj->m_count = readStringValue( str );  
    str = m_wxStrPeriod->GetValue();
    pObj->m_period = readStringValue( str ); 
    str = m_wxComboTrigger->GetValue();
    pObj->m_trigger = readStringValue( str );  
  
	pObj->m_Event.head = ( m_wxChoicePriority->GetCurrentSelection() << 5 );

    pObj->m_Event.timestamp = 0;
    if ( wxNOT_FOUND == m_wxComboClass->GetSelection() ) {
        str = m_wxComboClass->GetValue();
        pObj->m_Event.vscp_class = readStringValue( str );
    }
    else {
        pObj->m_Event.vscp_class = 
        (uintptr_t)m_wxComboClass->GetClientData( m_wxComboClass->GetSelection() );
    }
  
    if ( wxNOT_FOUND == m_wxComboType->GetSelection() ) {
        str = m_wxComboType->GetValue();
        pObj->m_Event.vscp_type = readStringValue( str );
    }
    else {
        pObj->m_Event.vscp_type = 
        (uintptr_t)m_wxComboType->GetClientData( m_wxComboType->GetSelection() );
    }

	// Priority
	pObj->m_Event.head &= ~VSCP_HEADER_PRIORITY_MASK;
	pObj->m_Event.head |= ( m_wxChoicePriority->GetSelection() << 5 );
  
    pObj->m_bUseDefaultGUID = m_wxChkDefaultGUID->GetValue();
    if ( pObj->m_bUseDefaultGUID ) {
        for ( int i=0; i<16; i++ ) {
            pObj->m_Event.GUID[ i ] = 0;
        }
    }
    else {
        str = m_wxStrGUID0->GetValue();
        pObj->m_Event.GUID[ 0 ] = readStringValue( str );
        str = m_wxStrGUID1->GetValue();
        pObj->m_Event.GUID[ 1 ] = readStringValue( str );
        str = m_wxStrGUID2->GetValue();
        pObj->m_Event.GUID[ 2 ] = readStringValue( str );
        str = m_wxStrGUID3->GetValue();
        pObj->m_Event.GUID[ 3 ] = readStringValue( str );
        str = m_wxStrGUID4->GetValue();
        pObj->m_Event.GUID[ 4 ] = readStringValue( str );
        str = m_wxStrGUID5->GetValue();
        pObj->m_Event.GUID[ 5 ] = readStringValue( str );
        str = m_wxStrGUID6->GetValue();
        pObj->m_Event.GUID[ 6 ] = readStringValue( str );
        str = m_wxStrGUID7->GetValue();
        pObj->m_Event.GUID[ 7 ] = readStringValue( str );
        str = m_wxStrGUID8->GetValue();
        pObj->m_Event.GUID[ 8 ] = readStringValue( str );
        str = m_wxStrGUID9->GetValue();
        pObj->m_Event.GUID[ 9 ] = readStringValue( str );
        str = m_wxStrGUID10->GetValue();
        pObj->m_Event.GUID[ 10 ] = readStringValue( str );
        str = m_wxStrGUID11->GetValue();
        pObj->m_Event.GUID[ 11 ] = readStringValue( str );
        str = m_wxStrGUID12->GetValue();
        pObj->m_Event.GUID[ 12 ] = readStringValue( str );
        str = m_wxStrGUID13->GetValue();
        pObj->m_Event.GUID[ 13 ] = readStringValue( str );
        str = m_wxStrGUID14->GetValue();
        pObj->m_Event.GUID[ 14 ] = readStringValue( str );
        str = m_wxStrGUID15->GetValue();
        pObj->m_Event.GUID[ 15 ] = readStringValue( str );
    }
  
    // Data is allowed to scan multiple lines and to be in hex
    // or decimal form
    str = m_wxStrData->GetValue();
    getVscpDataFromString( &pObj->m_Event, str );
  
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// writeEventData
//

bool dlgVSCPTrmitElement::writeEventData( VscpTXObj *pObj )
{
	wxString str;
	VSCPInformation vscpinfo;
    unsigned int i;

	// Check pointer
	if ( NULL == pObj ) return false;
  
	m_wxChkActive->SetValue( pObj->m_bActive );
  
	m_wxStrName->SetValue( pObj->m_wxStrName ); 
  
	m_wxChkDefaultGUID->SetValue( pObj->m_bUseDefaultGUID );
  
	str.Printf(_("%d"), pObj->m_count );
	m_wxStrCount->SetValue( str );
  
	str.Printf(_("%d"), pObj->m_period );
	m_wxStrPeriod->SetValue( str );
  
	str.Printf(_("%d"), pObj->m_trigger );
	m_wxComboTrigger->SetValue( str );
   
    for ( i=0; i<m_wxComboClass->GetCount(); i++ ) {
        if ( pObj->m_Event.vscp_class == (unsigned long)m_wxComboClass->GetClientData( i ) ) {
           m_wxComboClass->SetSelection( i );
           fillTypeCombo( pObj->m_Event.vscp_class );
           break;
        }
    }

    for ( i=0; i<m_wxComboType->GetCount(); i++ ) {
        if ( MAKE_CLASSTYPE_LONG( pObj->m_Event.vscp_class, 
                                   pObj->m_Event.vscp_type ) == (unsigned long)m_wxComboType->GetClientData( i ) ) {
           m_wxComboType->SetSelection( i );
           break;
        }
    }


	// Priority
	m_wxChoicePriority->SetSelection( ( pObj->m_Event.head & VSCP_HEADER_PRIORITY_MASK ) >> 5 );
 
	m_wxChkDefaultGUID->SetValue( pObj->m_bUseDefaultGUID );

	if ( pObj->m_bUseDefaultGUID ) {
		for ( int i=0; i<16; i++ ) {
			pObj->m_Event.GUID[ i ] = 0;
		}
	}

	str.Printf(_("%d"), pObj->m_Event.GUID[ 0 ] );
	m_wxStrGUID0->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 1 ] );
	m_wxStrGUID1->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 2 ] );
	m_wxStrGUID2->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 3 ] );
	m_wxStrGUID3->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 4 ] );
	m_wxStrGUID4->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 5 ] );
	m_wxStrGUID5->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 6 ] );
	m_wxStrGUID6->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 7 ] );
	m_wxStrGUID7->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 8 ] );
	m_wxStrGUID8->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 9 ] );
	m_wxStrGUID9->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 10 ] );
	m_wxStrGUID10->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 11 ] );
	m_wxStrGUID11->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 12 ] );
	m_wxStrGUID12->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 13 ] );
	m_wxStrGUID13->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 14 ] );
	m_wxStrGUID14->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 15 ] );
	m_wxStrGUID15->SetValue( str );
  
	// Data is allowed to scan multiple lines and to be in hex
	// or decimal form
    str.Empty();
	writeVscpDataToString( &pObj->m_Event, str );
	m_wxStrData->SetValue( str );
  
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_StrVscpClass
//

void dlgVSCPTrmitElement::OnStrVscpClassSelected( wxCommandEvent& event )
{
  int idx = m_wxComboClass->GetSelection();
  if ( wxNOT_FOUND != idx ) {
    fillTypeCombo( (unsigned long) m_wxComboClass->GetClientData( idx ) );
  }
  event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillClassCombo
//

void dlgVSCPTrmitElement::fillClassCombo( void )
{
  VSCPInformation vscpInfo;
  
  // Clear the combo
  vscpInfo.fillClassDescriptions( (wxControlWithItems *)m_wxComboClass, WITH_DECIMAL_SUFFIX );
   
  // Select first item
  m_wxComboClass->SetSelection( 0 );
  
  // Fill the type combo to 
  fillTypeCombo( (unsigned long) m_wxComboClass->GetClientData( 0 ) );
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillTypeCombo
//

void dlgVSCPTrmitElement::fillTypeCombo( unsigned int vscp_class )
{
  VSCPInformation vscpInfo;
  
  // Clear the combo
  vscpInfo.fillTypeDescriptions( (wxControlWithItems *)m_wxComboType, 
                                    vscp_class,
                                    WITH_DECIMAL_SUFFIX );

  // Select first item
  m_wxComboType->SetSelection( 0 );
  
}
