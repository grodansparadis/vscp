/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpfilter.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 25 Oct 2007 22:17:45 CEST
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
#pragma implementation "dlgvscpfilter.h"
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

#include "dlgvscpfilter.h"
#include <vscphelper.h>

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
// dlgVSCPFilter type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgVSCPFilter, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPFilter event table definition
//

BEGIN_EVENT_TABLE( dlgVSCPFilter, wxDialog )

////@begin dlgVSCPFilter event table entries
  EVT_BUTTON( ID_BUTTON_SET_CLASS, dlgVSCPFilter::OnButtonSetClassClick )

  EVT_BUTTON( ID_BUTTON_SET_TYPE, dlgVSCPFilter::OnButtonSetTypeClick )

  EVT_BUTTON( ID_BUTTON_SET_PRIORITY, dlgVSCPFilter::OnButtonSetPriorityClick )

  EVT_BUTTON( ID_BUTTON_SET_INDEX, dlgVSCPFilter::OnButtonSetIndexClick )

  EVT_BUTTON( ID_BUTTON_SET_ZONE, dlgVSCPFilter::OnButtonSetZoneClick )

  EVT_BUTTON( ID_BUTTON_SET_SUBZONE, dlgVSCPFilter::OnButtonSetSubzoneClick )

  EVT_BUTTON( ID_BUTTON_LOAD_FILTER, dlgVSCPFilter::OnButtonLoadFilterClick )

  EVT_BUTTON( ID_BUTTON_SAVE_FILTER, dlgVSCPFilter::OnButtonSaveFilterClick )

  EVT_BUTTON( ID_BUTTON_GUID_FILTER_WIZARD, dlgVSCPFilter::OnButtonGuidFilterWizardClick )

  EVT_BUTTON( ID_BUTTON_VSCP_FILTER_SETUP_WIZARD, dlgVSCPFilter::OnButtonVscpFilterSetupWizardClick )

  EVT_BUTTON( ID_BUTTON_CLEAR, dlgVSCPFilter::OnButtonClearClick )

////@end dlgVSCPFilter event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPFilter constructors
//

dlgVSCPFilter::dlgVSCPFilter()
{
  Init();
}

dlgVSCPFilter::dlgVSCPFilter( wxWindow* parent, 
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
// dlgVSCPFilter creator
//

bool dlgVSCPFilter::Create( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style )
{
////@begin dlgVSCPFilter creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgVSCPFilter creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPFilter destructor
//

dlgVSCPFilter::~dlgVSCPFilter()
{
////@begin dlgVSCPFilter destruction
////@end dlgVSCPFilter destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgVSCPFilter::Init()
{
////@begin dlgVSCPFilter member initialisation
  m_CLassFilter = NULL;
  m_ClassMask = NULL;
  m_TypeFilter = NULL;
  m_TypeMask = NULL;
  m_PriorityFilter = NULL;
  m_PriorityMask = NULL;
  m_IndexFilter = NULL;
  m_IndexMask = NULL;
  m_ZoneFilter = NULL;
  m_ZoneMask = NULL;
  m_SubZoneFilter = NULL;
  m_SubZoneMask = NULL;
  m_filterGUID7 = NULL;
  m_filterGUID6 = NULL;
  m_filterGUID5 = NULL;
  m_filterGUID4 = NULL;
  m_filterGUID3 = NULL;
  m_filterGUID2 = NULL;
  m_filterGUID1 = NULL;
  m_filterGUID0 = NULL;
  m_filterGUID15 = NULL;
  m_filterGUID14 = NULL;
  m_filterGUID13 = NULL;
  m_filterGUID12 = NULL;
  m_filterGUID11 = NULL;
  m_filterGUID10 = NULL;
  m_filterGUID9 = NULL;
  m_filterGUID8 = NULL;
  m_maskGUID7 = NULL;
  m_maskGUID6 = NULL;
  m_maskGUID5 = NULL;
  m_maskGUID4 = NULL;
  m_maskGUID3 = NULL;
  m_maskGUID2 = NULL;
  m_maskGUID1 = NULL;
  m_maskGUID0 = NULL;
  m_maskGUID15 = NULL;
  m_maskGUID14 = NULL;
  m_maskGUID13 = NULL;
  m_maskGUID12 = NULL;
  m_maskGUID11 = NULL;
  m_maskGUID10 = NULL;
  m_maskGUID9 = NULL;
  m_maskGUID8 = NULL;
////@end dlgVSCPFilter member initialisation

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgVSCPFilter
//

void dlgVSCPFilter::CreateControls()
{    
////@begin dlgVSCPFilter content construction
  dlgVSCPFilter* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer5 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer4->Add(itemGridSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText6 = new wxStaticText;
  itemStaticText6->Create( itemDialog1, wxID_STATIC, _("Class filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_CLassFilter = new wxTextCtrl;
  m_CLassFilter->Create( itemDialog1, ID_CLassFilter, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_CLassFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer5->Add(m_CLassFilter, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Class mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ClassMask = new wxTextCtrl;
  m_ClassMask->Create( itemDialog1, ID_ClassMask, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_ClassMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer5->Add(m_ClassMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton10 = new wxButton;
  itemButton10->Create( itemDialog1, ID_BUTTON_SET_CLASS, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer5->Add(itemButton10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer12 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer11->Add(itemGridSizer12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText13 = new wxStaticText;
  itemStaticText13->Create( itemDialog1, wxID_STATIC, _("Type filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_TypeFilter = new wxTextCtrl;
  m_TypeFilter->Create( itemDialog1, ID_TEXTCTRL_TYPEFILTER, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_TypeFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer12->Add(m_TypeFilter, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText15 = new wxStaticText;
  itemStaticText15->Create( itemDialog1, wxID_STATIC, _("Type mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer12->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_TypeMask = new wxTextCtrl;
  m_TypeMask->Create( itemDialog1, ID_TEXTCTRL_TYPEMASK, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_TypeMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer12->Add(m_TypeMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton17 = new wxButton;
  itemButton17->Create( itemDialog1, ID_BUTTON_SET_TYPE, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer12->Add(itemButton17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer19, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer20 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer19->Add(itemGridSizer20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText21 = new wxStaticText;
  itemStaticText21->Create( itemDialog1, wxID_STATIC, _("Priority filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer20->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_PriorityFilter = new wxTextCtrl;
  m_PriorityFilter->Create( itemDialog1, ID_TEXTCTRL_PRIORITY_FILTER, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_PriorityFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer20->Add(m_PriorityFilter, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText23 = new wxStaticText;
  itemStaticText23->Create( itemDialog1, wxID_STATIC, _("Priority mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer20->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_PriorityMask = new wxTextCtrl;
  m_PriorityMask->Create( itemDialog1, ID_TEXTCTRL_PRIORITY_MASK, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_PriorityMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer20->Add(m_PriorityMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton25 = new wxButton;
  itemButton25->Create( itemDialog1, ID_BUTTON_SET_PRIORITY, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer20->Add(itemButton25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer27, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer28 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer27->Add(itemGridSizer28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText29 = new wxStaticText;
  itemStaticText29->Create( itemDialog1, wxID_STATIC, _("Index filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer28->Add(itemStaticText29, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_IndexFilter = new wxTextCtrl;
  m_IndexFilter->Create( itemDialog1, ID_TEXTCTRL16, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_IndexFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer28->Add(m_IndexFilter, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText31 = new wxStaticText;
  itemStaticText31->Create( itemDialog1, wxID_STATIC, _("Index mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer28->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_IndexMask = new wxTextCtrl;
  m_IndexMask->Create( itemDialog1, ID_TEXTCTRL17, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_IndexMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer28->Add(m_IndexMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton33 = new wxButton;
  itemButton33->Create( itemDialog1, ID_BUTTON_SET_INDEX, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer28->Add(itemButton33, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer34, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer35 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer34->Add(itemGridSizer35, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText36 = new wxStaticText;
  itemStaticText36->Create( itemDialog1, wxID_STATIC, _("Zone filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ZoneFilter = new wxTextCtrl;
  m_ZoneFilter->Create( itemDialog1, ID_TEXTCTRL18, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_ZoneFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer35->Add(m_ZoneFilter, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText38 = new wxStaticText;
  itemStaticText38->Create( itemDialog1, wxID_STATIC, _("Zone mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText38, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ZoneMask = new wxTextCtrl;
  m_ZoneMask->Create( itemDialog1, ID_TEXTCTRL19, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_ZoneMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer35->Add(m_ZoneMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton40 = new wxButton;
  itemButton40->Create( itemDialog1, ID_BUTTON_SET_ZONE, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer35->Add(itemButton40, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer41, 0, wxALIGN_RIGHT|wxALL, 1);

  wxGridSizer* itemGridSizer42 = new wxGridSizer(1, 5, 0, 0);
  itemBoxSizer41->Add(itemGridSizer42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText43 = new wxStaticText;
  itemStaticText43->Create( itemDialog1, wxID_STATIC, _("Subzone filter :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer42->Add(itemStaticText43, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_SubZoneFilter = new wxTextCtrl;
  m_SubZoneFilter->Create( itemDialog1, ID_TEXTCTRL20, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_SubZoneFilter->SetBackgroundColour(wxColour(255, 192, 203));
  itemGridSizer42->Add(m_SubZoneFilter, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText45 = new wxStaticText;
  itemStaticText45->Create( itemDialog1, wxID_STATIC, _("Subzone mask :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer42->Add(itemStaticText45, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_SubZoneMask = new wxTextCtrl;
  m_SubZoneMask->Create( itemDialog1, ID_TEXTCTRL21, _("0x0000"), wxDefaultPosition, wxSize(100, -1), wxTE_RIGHT );
  m_SubZoneMask->SetBackgroundColour(wxColour(144, 238, 144));
  itemGridSizer42->Add(m_SubZoneMask, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton47 = new wxButton;
  itemButton47->Create( itemDialog1, ID_BUTTON_SET_SUBZONE, _("..."), wxDefaultPosition, wxSize(30, -1), 0 );
  itemGridSizer42->Add(itemButton47, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer49, 1, wxALIGN_RIGHT|wxALL, 1);

  wxBoxSizer* itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer49->Add(itemBoxSizer50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText51 = new wxStaticText;
  itemStaticText51->Create( itemDialog1, wxID_STATIC, _("GUID Filter (7-0):"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer50->Add(itemStaticText51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer52 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer50->Add(itemBoxSizer52, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID7 = new wxTextCtrl;
  m_filterGUID7->Create( itemDialog1, ID_FilterGUID7, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID7->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID6 = new wxTextCtrl;
  m_filterGUID6->Create( itemDialog1, ID_FilterGUID6, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID6->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID5 = new wxTextCtrl;
  m_filterGUID5->Create( itemDialog1, ID_FilterGUID5, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID5->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID4 = new wxTextCtrl;
  m_filterGUID4->Create( itemDialog1, ID_FilterGUID4, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID4->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID3 = new wxTextCtrl;
  m_filterGUID3->Create( itemDialog1, ID_FilterGUID3, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID3->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID2 = new wxTextCtrl;
  m_filterGUID2->Create( itemDialog1, ID_FilterGUID2, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID2->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID1 = new wxTextCtrl;
  m_filterGUID1->Create( itemDialog1, ID_FilterGUID1, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID1->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID0 = new wxTextCtrl;
  m_filterGUID0->Create( itemDialog1, ID_FilterGUID0, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID0->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer52->Add(m_filterGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer52->Add(65, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer62 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer62, 1, wxALIGN_RIGHT|wxALL, 1);

  wxBoxSizer* itemBoxSizer63 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer62->Add(itemBoxSizer63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText64 = new wxStaticText;
  itemStaticText64->Create( itemDialog1, wxID_STATIC, _("GUID Filter (15-8):"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer63->Add(itemStaticText64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer65 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer63->Add(itemBoxSizer65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID15 = new wxTextCtrl;
  m_filterGUID15->Create( itemDialog1, ID_TEXTCTRL, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID15->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID14 = new wxTextCtrl;
  m_filterGUID14->Create( itemDialog1, ID_TEXTCTRL1, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID14->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID13 = new wxTextCtrl;
  m_filterGUID13->Create( itemDialog1, ID_TEXTCTRL2, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID13->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID12 = new wxTextCtrl;
  m_filterGUID12->Create( itemDialog1, ID_TEXTCTRL3, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID12->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID11 = new wxTextCtrl;
  m_filterGUID11->Create( itemDialog1, ID_TEXTCTRL4, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID11->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID10 = new wxTextCtrl;
  m_filterGUID10->Create( itemDialog1, ID_TEXTCTRL5, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID10->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID9 = new wxTextCtrl;
  m_filterGUID9->Create( itemDialog1, ID_TEXTCTRL6, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID9->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_filterGUID8 = new wxTextCtrl;
  m_filterGUID8->Create( itemDialog1, ID_TEXTCTRL7, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_filterGUID8->SetBackgroundColour(wxColour(255, 192, 203));
  itemBoxSizer65->Add(m_filterGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer65->Add(65, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer75 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer75, 1, wxALIGN_RIGHT|wxALL, 1);

  wxBoxSizer* itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer75->Add(itemBoxSizer76, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText77 = new wxStaticText;
  itemStaticText77->Create( itemDialog1, wxID_STATIC, _("GUID Mask (7-0):"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer76->Add(itemStaticText77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer78 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer76->Add(itemBoxSizer78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID7 = new wxTextCtrl;
  m_maskGUID7->Create( itemDialog1, ID_TEXTCTRL8, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID7->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID6 = new wxTextCtrl;
  m_maskGUID6->Create( itemDialog1, ID_TEXTCTRL9, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID6->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID5 = new wxTextCtrl;
  m_maskGUID5->Create( itemDialog1, ID_TEXTCTRL10, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID5->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID4 = new wxTextCtrl;
  m_maskGUID4->Create( itemDialog1, ID_TEXTCTRL11, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID4->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID3 = new wxTextCtrl;
  m_maskGUID3->Create( itemDialog1, ID_TEXTCTRL12, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID3->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID2 = new wxTextCtrl;
  m_maskGUID2->Create( itemDialog1, ID_TEXTCTRL13, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID2->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID1 = new wxTextCtrl;
  m_maskGUID1->Create( itemDialog1, ID_TEXTCTRL14, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID1->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID0 = new wxTextCtrl;
  m_maskGUID0->Create( itemDialog1, ID_TEXTCTRL15, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID0->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer78->Add(m_maskGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer78->Add(65, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer88 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer88, 1, wxALIGN_RIGHT|wxALL, 1);

  wxBoxSizer* itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer88->Add(itemBoxSizer89, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText90 = new wxStaticText;
  itemStaticText90->Create( itemDialog1, wxID_STATIC, _("GUID Mask (15-8):"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer89->Add(itemStaticText90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer91 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer89->Add(itemBoxSizer91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID15 = new wxTextCtrl;
  m_maskGUID15->Create( itemDialog1, ID_MaskGUID15, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID15->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID14 = new wxTextCtrl;
  m_maskGUID14->Create( itemDialog1, ID_MaskGUID14, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID14->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID13 = new wxTextCtrl;
  m_maskGUID13->Create( itemDialog1, ID_MaskGUID13, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID13->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID12 = new wxTextCtrl;
  m_maskGUID12->Create( itemDialog1, ID_MaskGUID12, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID12->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID11 = new wxTextCtrl;
  m_maskGUID11->Create( itemDialog1, ID_MaskGUID11, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID11->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID10 = new wxTextCtrl;
  m_maskGUID10->Create( itemDialog1, ID_MaskGUID10, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID10->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID9 = new wxTextCtrl;
  m_maskGUID9->Create( itemDialog1, ID_MaskGUID9, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID9->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_maskGUID8 = new wxTextCtrl;
  m_maskGUID8->Create( itemDialog1, ID_MaskGUID8, _("0x00"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
  m_maskGUID8->SetBackgroundColour(wxColour(144, 238, 144));
  itemBoxSizer91->Add(m_maskGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer91->Add(65, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer102, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 20);

  wxButton* itemButton103 = new wxButton;
  itemButton103->Create( itemDialog1, ID_BUTTON_LOAD_FILTER, _("Load Filter..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemButton103, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  wxButton* itemButton104 = new wxButton;
  itemButton104->Create( itemDialog1, ID_BUTTON_SAVE_FILTER, _("Save Filter..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemButton104, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  itemBoxSizer102->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton106 = new wxButton;
  itemButton106->Create( itemDialog1, ID_BUTTON_GUID_FILTER_WIZARD, _("GUID filter wizard..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemButton106, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  wxButton* itemButton107 = new wxButton;
  itemButton107->Create( itemDialog1, ID_BUTTON_VSCP_FILTER_SETUP_WIZARD, _("Setup wizard..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemButton107, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  itemBoxSizer102->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton109 = new wxButton;
  itemButton109->Create( itemDialog1, ID_BUTTON_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemButton109, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

  wxBoxSizer* itemBoxSizer110 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer110, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer111 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer110->Add(itemBoxSizer111, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton112 = new wxButton;
  itemButton112->Create( itemDialog1, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer111->Add(itemButton112, 0, wxALIGN_BOTTOM|wxALL, 5);

  itemBoxSizer110->Add(100, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer114 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer110->Add(itemBoxSizer114, 0, wxALIGN_BOTTOM|wxALL, 5);

  wxButton* itemButton115 = new wxButton;
  itemButton115->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemButton115, 0, wxALIGN_BOTTOM|wxALL, 5);

  wxButton* itemButton116 = new wxButton;
  itemButton116->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer114->Add(itemButton116, 0, wxALIGN_BOTTOM|wxALL, 5);

////@end dlgVSCPFilter content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgVSCPFilter::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgVSCPFilter::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgVSCPFilter bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgVSCPFilter bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgVSCPFilter::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgVSCPFilter icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
////@end dlgVSCPFilter icon retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setFilter
//

void dlgVSCPFilter::setFilter( vscpEventFilter *pFilter )
{
    m_CLassFilter->SetValue( wxString::Format( _("0x%04X"), pFilter->filter_class ) );
    m_ClassMask->SetValue( wxString::Format( _("0x%04X"), pFilter->mask_class ) );
    
    m_TypeFilter->SetValue( wxString::Format( _("0x%04X"), pFilter->filter_type ) );
    m_TypeMask->SetValue( wxString::Format( _("0x%04X"), pFilter->mask_type ) );
    
    m_PriorityFilter->SetValue( wxString::Format( _("0x%04X"), pFilter->filter_priority ) );
    m_PriorityMask->SetValue( wxString::Format( _("0x%04X"), pFilter->mask_priority ) );
    
    m_PriorityFilter->SetValue( wxString::Format( _("0x%04X"), pFilter->filter_priority ) );
    m_PriorityMask->SetValue( wxString::Format( _("0x%04X"), pFilter->mask_priority ) );
    
    m_filterGUID0->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 0 ] ) );
    m_filterGUID1->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 1 ] ) );
    m_filterGUID2->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 2 ] ) );
    m_filterGUID3->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 3 ] ) );
    m_filterGUID4->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 4 ] ) );
    m_filterGUID5->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 5 ] ) );
    m_filterGUID6->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 6 ] ) );
    m_filterGUID7->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 7 ] ) );
    m_filterGUID8->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 8 ] ) );
    m_filterGUID9->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 9 ] ) );
    m_filterGUID10->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 10 ] ) );
    m_filterGUID11->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 11 ] ) );
    m_filterGUID12->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 12 ] ) );
    m_filterGUID13->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 13 ] ) );
    m_filterGUID14->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 14 ] ) );
    m_filterGUID15->SetValue( wxString::Format( _("0x%02X"), pFilter->filter_GUID[ 15 ] ) );
    
    m_maskGUID0->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 0 ] ) );
    m_maskGUID1->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 1 ] ) );
    m_maskGUID2->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 2 ] ) );
    m_maskGUID3->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 3 ] ) );
    m_maskGUID4->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 4 ] ) );
    m_maskGUID5->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 5 ] ) );
    m_maskGUID6->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 6 ] ) );
    m_maskGUID7->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 7 ] ) );
    m_maskGUID8->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 8 ] ) );
    m_maskGUID9->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 9 ] ) );
    m_maskGUID10->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 10 ] ) );
    m_maskGUID11->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 11 ] ) );
    m_maskGUID12->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 12 ] ) );
    m_maskGUID13->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 13 ] ) );
    m_maskGUID14->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 14 ] ) );
    m_maskGUID15->SetValue( wxString::Format( _("0x%02X"), pFilter->mask_GUID[ 15 ] ) );
}	
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getFilter
//

void dlgVSCPFilter::getFilter( vscpEventFilter *pFilter )
{
    pFilter->filter_class = vscp_readStringValue( m_CLassFilter->GetValue() );
    pFilter->mask_class = vscp_readStringValue( m_ClassMask->GetValue() );
    
    pFilter->filter_type = vscp_readStringValue( m_TypeFilter->GetValue() );
    pFilter->mask_type = vscp_readStringValue( m_TypeMask->GetValue() );

		pFilter->filter_priority = vscp_readStringValue( m_PriorityFilter->GetValue() );
    pFilter->mask_priority = vscp_readStringValue( m_PriorityMask->GetValue() );

		pFilter->filter_GUID[ 0 ] = vscp_readStringValue( m_filterGUID0->GetValue() );
		pFilter->filter_GUID[ 1 ] = vscp_readStringValue( m_filterGUID1->GetValue() );
		pFilter->filter_GUID[ 2 ] = vscp_readStringValue( m_filterGUID2->GetValue() );
		pFilter->filter_GUID[ 3 ] = vscp_readStringValue( m_filterGUID3->GetValue() );
		pFilter->filter_GUID[ 4 ] = vscp_readStringValue( m_filterGUID4->GetValue() );
		pFilter->filter_GUID[ 5 ] = vscp_readStringValue( m_filterGUID5->GetValue() );
		pFilter->filter_GUID[ 6 ] = vscp_readStringValue( m_filterGUID6->GetValue() );
		pFilter->filter_GUID[ 7 ] = vscp_readStringValue( m_filterGUID7->GetValue() );
		pFilter->filter_GUID[ 8 ] = vscp_readStringValue( m_filterGUID8->GetValue() );
		pFilter->filter_GUID[ 9 ] = vscp_readStringValue( m_filterGUID9->GetValue() );
		pFilter->filter_GUID[ 10 ] = vscp_readStringValue( m_filterGUID10->GetValue() );
		pFilter->filter_GUID[ 11 ] = vscp_readStringValue( m_filterGUID11->GetValue() );
		pFilter->filter_GUID[ 12 ] = vscp_readStringValue( m_filterGUID12->GetValue() );
		pFilter->filter_GUID[ 13 ] = vscp_readStringValue( m_filterGUID13->GetValue() );
		pFilter->filter_GUID[ 14 ] = vscp_readStringValue( m_filterGUID14->GetValue() );
		pFilter->filter_GUID[ 15 ] = vscp_readStringValue( m_filterGUID15->GetValue() );

		pFilter->mask_GUID[ 0 ] = vscp_readStringValue( m_maskGUID0->GetValue() );
		pFilter->mask_GUID[ 1 ] = vscp_readStringValue( m_maskGUID1->GetValue() );
		pFilter->mask_GUID[ 2 ] = vscp_readStringValue( m_maskGUID2->GetValue() );
		pFilter->mask_GUID[ 3 ] = vscp_readStringValue( m_maskGUID3->GetValue() );
		pFilter->mask_GUID[ 4 ] = vscp_readStringValue( m_maskGUID4->GetValue() );
		pFilter->mask_GUID[ 5 ] = vscp_readStringValue( m_maskGUID5->GetValue() );
		pFilter->mask_GUID[ 6 ] = vscp_readStringValue( m_maskGUID6->GetValue() );
		pFilter->mask_GUID[ 7 ] = vscp_readStringValue( m_maskGUID7->GetValue() );
		pFilter->mask_GUID[ 8 ] = vscp_readStringValue( m_maskGUID8->GetValue() );
		pFilter->mask_GUID[ 9 ] = vscp_readStringValue( m_maskGUID9->GetValue() );
		pFilter->mask_GUID[ 10 ] = vscp_readStringValue( m_maskGUID10->GetValue() );
		pFilter->mask_GUID[ 11 ] = vscp_readStringValue( m_maskGUID11->GetValue() );
		pFilter->mask_GUID[ 12 ] = vscp_readStringValue( m_maskGUID12->GetValue() );
		pFilter->mask_GUID[ 13 ] = vscp_readStringValue( m_maskGUID13->GetValue() );
		pFilter->mask_GUID[ 14 ] = vscp_readStringValue( m_maskGUID14->GetValue() );
		pFilter->mask_GUID[ 15 ] = vscp_readStringValue( m_maskGUID15->GetValue() );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_CLASS
//

void dlgVSCPFilter::OnButtonSetClassClick( wxCommandEvent& event )
{
	wxMessageBox(_("Not yet implemented."));
  event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_TYPE
//

void dlgVSCPFilter::OnButtonSetTypeClick( wxCommandEvent& event )
{
	wxMessageBox(_("Not yet implemented."));
  event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_PRIORITY
//

void dlgVSCPFilter::OnButtonSetPriorityClick( wxCommandEvent& event )
{
  wxMessageBox(_("Set priority not yet implemented."));
  event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_GUID_FILTER_WIZARD
//

void dlgVSCPFilter::OnButtonGuidFilterWizardClick( wxCommandEvent& event )
{
	wxMessageBox(_("GUID filter wizard not yet implemented."));
  event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_VSCP_FILTER_SETUP_WIZARD
//

void dlgVSCPFilter::OnButtonVscpFilterSetupWizardClick( wxCommandEvent& event )
{
	wxMessageBox(_("Filter setup wizard not yet implemented."));
  event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILTER
//

void dlgVSCPFilter::OnButtonLoadFilterClick( wxCommandEvent& event )
{
	wxMessageBox(_("Load filter from file not yet implemented."));
  event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE_FILTER
//

void dlgVSCPFilter::OnButtonSaveFilterClick( wxCommandEvent& event )
{
	wxMessageBox(_("Save filter to file not yet implemented."));
  event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CLEAR
//

void dlgVSCPFilter::OnButtonClearClick( wxCommandEvent& event )
{
  m_CLassFilter->SetValue(_("0x0000"));
	m_ClassMask->SetValue(_("0x0000"));
	m_TypeFilter->SetValue(_("0x0000"));
	m_TypeMask->SetValue(_("0x0000"));
  m_PriorityFilter->SetValue(_("0x0000"));
	m_PriorityMask->SetValue(_("0x0000"));

	m_filterGUID0->SetValue(_("0x00"));
	m_filterGUID1->SetValue(_("0x00"));
	m_filterGUID2->SetValue(_("0x00"));
	m_filterGUID3->SetValue(_("0x00"));
	m_filterGUID4->SetValue(_("0x00"));
	m_filterGUID5->SetValue(_("0x00"));
	m_filterGUID6->SetValue(_("0x00"));
	m_filterGUID7->SetValue(_("0x00"));
	m_filterGUID8->SetValue(_("0x00"));
	m_filterGUID9->SetValue(_("0x00"));
	m_filterGUID10->SetValue(_("0x00"));
	m_filterGUID11->SetValue(_("0x00"));
	m_filterGUID12->SetValue(_("0x00"));
	m_filterGUID13->SetValue(_("0x00"));
	m_filterGUID14->SetValue(_("0x00"));
	m_filterGUID15->SetValue(_("0x00"));

	m_maskGUID0->SetValue(_("0x00"));
	m_maskGUID1->SetValue(_("0x00"));
	m_maskGUID2->SetValue(_("0x00"));
	m_maskGUID3->SetValue(_("0x00"));
	m_maskGUID4->SetValue(_("0x00"));
	m_maskGUID5->SetValue(_("0x00"));
	m_maskGUID6->SetValue(_("0x00"));
	m_maskGUID7->SetValue(_("0x00"));
	m_maskGUID8->SetValue(_("0x00"));
	m_maskGUID9->SetValue(_("0x00"));
	m_maskGUID10->SetValue(_("0x00"));
	m_maskGUID11->SetValue(_("0x00"));
	m_maskGUID12->SetValue(_("0x00"));
	m_maskGUID13->SetValue(_("0x00"));
	m_maskGUID14->SetValue(_("0x00"));
	m_maskGUID15->SetValue(_("0x00"));

	event.Skip();
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_INDEX
//

void dlgVSCPFilter::OnButtonSetIndexClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_INDEX in dlgVSCPFilter.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_INDEX in dlgVSCPFilter. 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_ZONE
//

void dlgVSCPFilter::OnButtonSetZoneClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_ZONE in dlgVSCPFilter.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_ZONE in dlgVSCPFilter. 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_SUBZONE
//

void dlgVSCPFilter::OnButtonSetSubzoneClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_SUBZONE in dlgVSCPFilter.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_SUBZONE in dlgVSCPFilter. 
}

