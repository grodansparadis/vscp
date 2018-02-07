/////////////////////////////////////////////////////////////////////////////
// Name:        dlgabout.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     18/05/2009 17:14:13
// RCS-ID:      
// Copyright:   (C) 2009-2018
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
#pragma implementation "dlgabout.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif



#include "version.h"
#include "dlgabout.h"

#define HTML _("<html><head><meta content=\"text/html; charset=ISO-8859-1\"  http-equiv=\"content-type\"><title></title></head><body><h1 style=\"text-align: center;\">VSCP Works</h1><div style=\"text-align: center;\"><span style=\"font-weight: bold;\">\
        Copyright (c) 2000-2017</span><br> Grodans Paradis AB &lt;<a href=\"mailto:info@grodansparadis.com\">info@grodansparadis.com</a>&gt<br><a href=\"http://www.grodansparadis.com\">http://www.grodansparadis.com</a>\
        <br><br>Part of the VSCP &amp; friends package.<br><span style=\"font-weight: bold;\">VSCP site:</span> <a href=\"http://www.vscp.org\">http://www.vscp.org</a><br><br><h3><span style=\"font-weight: bold;\">Version</span>:%s</h3></div></body></html>") 

static const wxChar *about_html[] = { HTML  };

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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgAbout type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgAbout, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgAbout event table definition
//

BEGIN_EVENT_TABLE( dlgAbout, wxDialog )


END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgAbout constructors
//

dlgAbout::dlgAbout()
{
    Init();
}

dlgAbout::dlgAbout( wxWindow* parent, 
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
// dlgAbout creator
//

bool dlgAbout::Create( wxWindow* parent, 
                        wxWindowID id, 
                        const wxString& caption, 
                        const wxPoint& pos, 
                        const wxSize& size, 
                        long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    if ( GetSizer() ) {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgAbout destructor
//

dlgAbout::~dlgAbout()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgAbout::Init()
{


}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgAbout
//

void dlgAbout::CreateControls()
{    
    dlgAbout* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxHtmlWindow* itemHtmlWindow3 = new wxHtmlWindow;
    itemHtmlWindow3->Create( itemDialog1, 
                                ID_HTMLWINDOW, 
                                wxDefaultPosition, 
                                wxSize(500, 300), 
                                wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(itemHtmlWindow3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton;
    itemButton5->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxString html( *about_html, wxConvUTF8 );
    //wxString wxStringhtml = itemHtmlWindow3->ToText();
    wxString out;
    wxString versionstr( _(VSCPD_DISPLAY_VERSION) );
    out.Printf( html, (const char *)versionstr.c_str() );
  
    itemHtmlWindow3->SetPage( out );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgAbout::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgAbout::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgAbout::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == _T("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }

    return wxNullIcon;
}
