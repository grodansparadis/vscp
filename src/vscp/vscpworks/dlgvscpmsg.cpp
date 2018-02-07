/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpmsg.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:46:25
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
#pragma implementation "dlgvscpmsg.h"
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

#include <vscphelper.h>
#include <vscpeventhelper.h>
#include "dlgvscpmsg.h"

extern VSCPInformation g_vscpinfo;  // VSCP class type information

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

/*!
 * dlgVSCPMsg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgVSCPMsg, wxDialog )

/*!
 * dlgVSCPMsg event table definition
 */

BEGIN_EVENT_TABLE( dlgVSCPMsg, wxDialog )

////@begin dlgVSCPMsg event table entries
  EVT_INIT_DIALOG( dlgVSCPMsg::OnInitDialog )
  EVT_CLOSE( dlgVSCPMsg::OnCloseWindow )

////@end dlgVSCPMsg event table entries

END_EVENT_TABLE()

/*!
 * dlgVSCPMsg constructors
 */

dlgVSCPMsg::dlgVSCPMsg()
{
    Init();
}

dlgVSCPMsg::dlgVSCPMsg( wxWindow* parent, 
                            wxWindowID id, 
                            const wxString& caption, 
                            const wxPoint& pos, 
                            const wxSize& size, 
                            long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * DlgVSCPMsg creator
 */

bool dlgVSCPMsg::Create( wxWindow* parent, 
                            wxWindowID id, 
                            const wxString& caption, 
                            const wxPoint& pos, 
                            const wxSize& size, 
                            long style )
{
////@begin dlgVSCPMsg creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgVSCPMsg creation
    return true;
}

/*!
 * dlgVSCPMsg destructor
 */

dlgVSCPMsg::~dlgVSCPMsg()
{
////@begin dlgVSCPMsg destruction
////@end dlgVSCPMsg destruction
}

/*!
 * Member initialisation 
 */

void dlgVSCPMsg::Init()
{
////@begin dlgVSCPMsg member initialisation
  m_choiceVscpClass = NULL;
  m_wxChoiceVscpType = NULL;
  m_wxChoicePriority = NULL;
  m_strGUID15 = NULL;
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
  m_strVscpData = NULL;
  m_strNote = NULL;
////@end dlgVSCPMsg member initialisation
}
/*!
 * Control creation for DlgVSCPMsg
 */

void dlgVSCPMsg::CreateControls()
{    
////@begin dlgVSCPMsg content construction
  dlgVSCPMsg* itemDialog1 = this;

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

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Class :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText8->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else    
  itemStaticText8->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_choiceVscpClassStrings;
  m_choiceVscpClass = new wxComboBox;
  m_choiceVscpClass->Create( itemDialog1, ID_COMBOBOX2, _T(""), wxDefaultPosition, wxSize(200, -1), m_choiceVscpClassStrings, wxCB_DROPDOWN );
  itemFlexGridSizer7->Add(m_choiceVscpClass, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( itemDialog1, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText10->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else 
  itemStaticText10->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxArrayString m_wxChoiceVscpTypeStrings;
  m_wxChoiceVscpType = new wxComboBox;
  m_wxChoiceVscpType->Create( itemDialog1, ID_COMBOBOX3, _T(""), wxDefaultPosition, wxSize(200, -1), m_wxChoiceVscpTypeStrings, wxCB_DROPDOWN );
  itemFlexGridSizer7->Add(m_wxChoiceVscpType, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText12 = new wxStaticText;
  itemStaticText12->Create( itemDialog1, wxID_STATIC, _("Priority :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText12->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText12->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

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
  m_wxChoicePriority->Create( itemDialog1, ID_CHOICE, wxDefaultPosition, wxSize(120, -1), m_wxChoicePriorityStrings, 0 );
  m_wxChoicePriority->SetStringSelection(_("0"));
  itemFlexGridSizer7->Add(m_wxChoicePriority, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText17 = new wxStaticText;
  itemStaticText17->Create( itemDialog1, wxID_STATIC, _("GUID"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText17->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText17->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText17, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("15 - 8 :"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText18->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText18->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer7->Add(itemBoxSizer19, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_strGUID15 = new wxTextCtrl;
  m_strGUID15->Create( itemDialog1, ID_TEXTCTRL22, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_strGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID14 = new wxTextCtrl;
  m_wxStrGUID14->Create( itemDialog1, ID_TEXTCTRL23, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID13 = new wxTextCtrl;
  m_wxStrGUID13->Create( itemDialog1, ID_TEXTCTRL24, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID12 = new wxTextCtrl;
  m_wxStrGUID12->Create( itemDialog1, ID_TEXTCTRL25, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID11 = new wxTextCtrl;
  m_wxStrGUID11->Create( itemDialog1, ID_TEXTCTRL26, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID10 = new wxTextCtrl;
  m_wxStrGUID10->Create( itemDialog1, ID_TEXTCTRL27, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID9 = new wxTextCtrl;
  m_wxStrGUID9->Create( itemDialog1, ID_TEXTCTRL28, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID8 = new wxTextCtrl;
  m_wxStrGUID8->Create( itemDialog1, ID_TEXTCTRL29, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer19->Add(m_wxStrGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText28 = new wxStaticText;
  itemStaticText28->Create( itemDialog1, wxID_STATIC, _("7 - 0 :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText28->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText28->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText28, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemFlexGridSizer7->Add(itemBoxSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID7 = new wxTextCtrl;
  m_wxStrGUID7->Create( itemDialog1, ID_TEXTCTRL30, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID6 = new wxTextCtrl;
  m_wxStrGUID6->Create( itemDialog1, ID_TEXTCTRL31, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID5 = new wxTextCtrl;
  m_wxStrGUID5->Create( itemDialog1, ID_TEXTCTRL32, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID4 = new wxTextCtrl;
  m_wxStrGUID4->Create( itemDialog1, ID_TEXTCTRL33, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID3 = new wxTextCtrl;
  m_wxStrGUID3->Create( itemDialog1, ID_TEXTCTRL34, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID2 = new wxTextCtrl;
  m_wxStrGUID2->Create( itemDialog1, ID_TEXTCTRL35, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID1 = new wxTextCtrl;
  m_wxStrGUID1->Create( itemDialog1, ID_TEXTCTRL36, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_wxStrGUID0 = new wxTextCtrl;
  m_wxStrGUID0->Create( itemDialog1, ID_TEXTCTRL37, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  itemBoxSizer29->Add(m_wxStrGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText38 = new wxStaticText;
  itemStaticText38->Create( itemDialog1, wxID_STATIC, _("Data :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText38->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText38->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText38, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_strVscpData = new wxTextCtrl;
  m_strVscpData->Create( itemDialog1, ID_TEXTCTRL38, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
  if (dlgVSCPMsg::ShowToolTips())
    m_strVscpData->SetToolTip(_("You enter data for the event as a comma separated list. You can arrange the data on lines. Hexadecimal values should be preceded with '0x'."));
  itemFlexGridSizer7->Add(m_strVscpData, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText42 = new wxStaticText;
  itemStaticText42->Create( itemDialog1, wxID_STATIC, _("Note :"), wxDefaultPosition, wxDefaultSize, 0 );
  #if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )
  itemStaticText42->SetFont(wxFont( wxFontInfo(8).FaceName("Tahoma").Bold() ) );    
  #else
  itemStaticText42->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
  #endif
  itemFlexGridSizer7->Add(itemStaticText42, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_strNote = new wxTextCtrl;
  m_strNote->Create( itemDialog1, ID_TEXTCTRL39, _T(""), wxDefaultPosition, wxSize(-1, 50), 0 );
  itemFlexGridSizer7->Add(m_strNote, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

  wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer44, 0, wxALIGN_BOTTOM|wxALL, 1);

  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer44->Add(itemBoxSizer45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton46 = new wxButton;
  itemButton46->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer45->Add(itemButton46, 0, wxALIGN_RIGHT|wxALL, 1);

  wxButton* itemButton47 = new wxButton;
  itemButton47->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer45->Add(itemButton47, 0, wxALIGN_RIGHT|wxALL, 1);

////@end dlgVSCPMsg content construction
}

/*!
 * wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_MESSAGE
 */

void dlgVSCPMsg::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_MESSAGE in DlgVSCPMsg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_MESSAGE in DlgVSCPMsg. 
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_MESSAGE
 */

void dlgVSCPMsg::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_MESSAGE in DlgVSCPMsg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_MESSAGE in DlgVSCPMsg. 
}

/*!
 * Should we show tooltips?
 */

bool dlgVSCPMsg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgVSCPMsg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgVSCPMsg bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgVSCPMsg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgVSCPMsg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgVSCPMsg icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
////@end dlgVSCPMsg icon retrieval
}
