/////////////////////////////////////////////////////////////////////////////
//  Name:        frmdeviceconfig.cpp
//  Purpose:     
//  Author:      Ake Hedman
//  Modified by: 
//  Created:     Sun 04 May 2007 17:28:12 CEST
//  RCS-ID:      
//  Copyright:   (C) 2007-2013 
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
/////////////////////////////////////////////////////////////////////////////

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
#include "../common/canal.h"
#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "../common/mdf.h"
#include "dlgabout.h"
#include "dialogeditlevelidmrow.h"
#include "dialogabstractionedit.h"
#include "readregister.h"
#include "frmdeviceconfig.h"

extern appConfiguration g_Config;

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

////@begin XPM images
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
static char *find_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 41 1",
"y c #A06959",
"9 c #A7DAF2",
"$ c #B5CAD7",
"> c #35B4E1",
"t c #6B98B8",
"w c #B6E0F4",
"q c #AEC9D7",
"1 c #5A89A6",
"+ c #98B3C6",
"4 c #EAF6FC",
"3 c #DEF1FA",
"= c #4CBCE3",
"d c #DB916B",
"X c #85A7BC",
"s c #D8BCA4",
"o c #749BB4",
"e c #BCD9EF",
"* c #62B4DD",
"< c #91D2EF",
"a c #E6DED2",
"0 c #E9F4FB",
"  c None",
"@ c #A0BACB",
"O c #AABFCD",
"i c #6591AE",
": c #B9CBD5",
"- c #71C5E7",
"5 c #D3ECF8",
"% c #81A3B9",
"6 c #8AD0EE",
"8 c #FDFDFE",
"p c #8EA9BC",
"r c #B6D5EE",
", c #81CCEB",
". c #ACC4D3",
"; c #AFD1DE",
"7 c #EFF8FC",
"u c #C2CBDB",
"# c #C0D1DC",
"2 c #CAD6E1",
"& c #8FB0C3",
/* pixels */
"       .XooXO   ",
"      +@###$+%  ",
"     .&#*==-;@@ ",
"     o:*>,<--:X ",
"     12>-345-#% ",
"     12>678392% ",
"     %$*,3059q& ",
"     @Oq,wwer@@ ",
"      t@q22q&+  ",
"    yyui+%o%p   ",
"   yasy         ",
"  yasdy         ",
" yasdy          ",
" ysdy           ",
"  yy            "
};

/* XPM */
static const char *database_process_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 621 2",
"|o c #DDDDDC",
"+X c #C8D6DF",
"DX c #E3E2E1",
"`O c #E3E2E2",
"uo c #D1DCE2",
"ao c #D7DEE3",
"Lo c #E6E6E5",
"]+ c #E6E6E6",
"cO c #556C79",
"'o c #E9EAE9",
"7. c #E0E7EB",
"~O c #58707E",
"O. c #E0E7EC",
"<+ c #EFEFEF",
"TO c #ECEEF0",
"BX c #617986",
"Do c #F8F8F7",
"M  c #FBFCFC",
"aO c #F5FAFD",
"n. c #FBFCFD",
"5+ c #708794",
"C@ c #A3A19F",
"0+ c #A3A1A0",
"C. c #7090A1",
"w@ c #7090A2",
"Oo c #A6A5A4",
"yX c #7C94A3",
"x+ c #A3A7A9",
"k+ c #9AA4AA",
"@o c #829CAB",
"7@ c #AFB1B2",
"cX c #B5B6B6",
"bo c #B8B7B7",
"G+ c #C1C0BE",
"]O c #B8BDBF",
"%  c #B2BBBF",
"-X c #94AEBE",
"to c #A0B5C1",
"L  c #A9B8C2",
"#@ c #B8C0C4",
"a. c #9DB4C3",
"N+ c #A6BAC5",
"X  c #B8C0C5",
"2@ c #C7C8C9",
"gO c #CDCDCC",
"H. c #A9BECB",
"c@ c #AFC3D1",
"5@ c #D3D2D2",
"W. c #B2C4D1",
"*  c #C7CED2",
";  c #C7CED3",
" @ c #D6D6D5",
"&@ c #D0D4D7",
"3O c #DCDBDA",
"=. c #C1CFD9",
"ZO c #DCDBDB",
"Jo c #DFDFDE",
"-O c #DFDFDF",
"A  c #CDD9E1",
"V+ c #D6DFE4",
"v@ c #D0DDE5",
"^  c #D3DEE5",
"C+ c #D9E0E6",
"WO c #546D7A",
"C  c #DCE4EA",
"Fo c #EEEDEC",
"h+ c #F1F1F0",
"V  c #E5EAEF",
"=@ c #F1F1F1",
"x. c #E8EEF1",
"1. c #E8EEF2",
"{+ c #F4F5F4",
".. c #EBEFF3",
"0X c #697D87",
"lO c #8A8888",
"u  c #EEF3F6",
"E  c #F1F4F7",
"V@ c #7E878C",
"bO c #637E8D",
"3. c #5A7B8E",
".@ c #939190",
"YX c #939191",
"&+ c #6F8592",
"d. c #638495",
"eo c #638497",
"h@ c #78919F",
"(X c #8498A3",
"/X c #8498A4",
" o c #939DA4",
"h. c #7B98A8",
"e@ c #7E9CAD",
"k@ c #87A2B2",
"b+ c #BAB9B9",
"BO c #C0BEBC",
"0o c #BDBDBC",
"t+ c #A8B6BF",
":o c #A2B4BF",
"3@ c #C3C2C1",
"(. c #99B1C0",
"xX c #B1BCC3",
"x@ c #A5BBC9",
"SO c #C9CACA",
"8X c #C0C7CA",
"5X c #C3C8CB",
"7  c #C0C7CB",
"0  c #BAC5CB",
"do c #CFCFCE",
";. c #AEC1CE",
"&  c #C6CCD0",
"8@ c #D2D3D3",
"_  c #B4C6D2",
"r@ c #BACBD6",
"4X c #D2D6D8",
"D  c #BDCCD7",
"l. c #C3D1DA",
"   c None",
"8o c #DEDDDC",
"w  c #D2D9DE",
"*. c #C9D6DE",
"<. c #CCD7DF",
"*@ c #E1E1E1",
"wX c #4D6673",
"Z+ c #D5DDE2",
"Po c #E7E6E5",
"U+ c #E7E6E6",
">O c #DEE3E7",
"y. c #D8E1E7",
"Ko c #EAEAE9",
"P  c #E7E9EB",
"AO c #F0EFEF",
"vX c #EDF1F2",
"Y. c #E7ECF1",
"2o c #F3F3F3",
"8. c #EDF1F4",
"CX c #627987",
"U. c #F3F6F8",
"rX c #627C8A",
"GO c #748892",
"wo c #5F8192",
"nO c #748E9E",
"No c #7190A2",
"YO c #A7A5A5",
"!. c #7795A6",
"Bo c #7795A7",
"J@ c #98A3A9",
"s@ c #9BA4AA",
"z+ c #98A3AA",
"ro c #7D9AAA",
"*X c #7D9AAB",
"OO c #B9B7B7",
"/o c #B0B4B7",
"eO c #9EAEB9",
"!o c #8FA9BA",
"a@ c #AAB5BD",
";o c #A1B2BD",
"z@ c #9BB3C2",
"h  c #A1B5C2",
"UX c #C5C4C3",
"(+ c #C5C4C4",
"[. c #A1B8C6",
"co c #C8C8C7",
"_+ c #C8C8C8",
"vo c #CBCCCB",
"X+ c #CECDCC",
"l  c #BFC8CE",
"d+ c #D1D1D0",
"9X c #C5CDD0",
"-  c #C5CDD1",
"6  c #C8CED2",
"m@ c #D7D6D5",
"'X c #D7D6D6",
"lo c #DADAD9",
"j+ c #DADADA",
"(  c #C5D3DC",
"<@ c #C8D4DD",
",+ c #E0DFDF",
"[O c #496371",
"v+ c #E3E3E3",
"s  c #CED9E2",
"GX c #4C6776",
"Z  c #D4DEE5",
"|O c #526C79",
"9@ c #E9E8E8",
"QO c #556D7A",
"p@ c #ECECEB",
"R  c #DDE4EA",
"jo c #ECECEC",
"AX c #61747E",
"U  c #E3E9EE",
"6@ c #F5F5F5",
"m  c #EFF3F5",
"1X c #EFF3F7",
"2X c #FBFDFF",
"Q. c #5E7F91",
"E+ c #9A9998",
"W+ c #67889A",
"2+ c #82919A",
"}. c #67889B",
"RO c #82949E",
"WX c #8598A4",
"l+ c #919FA7",
"S@ c #8B9DA8",
" X c #7997A8",
"X@ c #85A1B1",
"5O c #B5B4B3",
"NO c #9DACB4",
"5. c #91A8B5",
"0O c #8BA6B6",
"iX c #94A9B6",
")+ c #BEBDBB",
"j. c #94ACBA",
"l@ c #91ABBB",
"Eo c #BEBDBC",
"j  c #9DAFBC",
"qo c #BEBDBD",
"#X c #ACB7BD",
"oO c #C1C1C0",
"pX c #A0B3BF",
"[  c #97B0BF",
"-o c #A3B4BF",
"n@ c #C4C2C2",
"VO c #C7C6C5",
";X c #9DB5C4",
"*o c #AFBEC7",
"@X c #A9BCC9",
">. c #A6BBC9",
"4  c #BBC5CB",
"sX c #B5C3CB",
"_. c #ACC0CD",
"mo c #D0CFCE",
"_O c #D0CFCF",
",. c #AFC1CE",
"Zo c #AFC4D0",
">  c #C1CAD0",
",@ c #B5C6D1",
"Go c #D9D8D7",
"6. c #C4D1D8",
"xo c #D9D8D8",
"b@ c #D6D7D9",
"%o c #CAD3D9",
"ko c #DCDCDC",
"r. c #CAD6DF",
"}X c #E2E1E0",
"qX c #4E6672",
"5o c #E2E1E1",
" . c #D3DCE2",
"t@ c #CDDAE3",
"Uo c #E5E5E4",
"&. c #D0DBE3",
"RX c #DCE2E6",
"a  c #D6E0E7",
"e  c #E2E7EB",
"$. c #DFE6EB",
"uO c #EEEEED",
",O c #DFE6EC",
"g+ c #EEEEEE",
"+. c #E5EBEF",
"d@ c #F4F3F2",
"pO c #F1F2F3",
"i. c #EBF0F3",
"VX c #607885",
"zX c #6F7D85",
"M@ c #F1F5F7",
"W  c #F4F6F8",
"K@ c #8A8C8E",
"v. c #FAFBFC",
":@ c #698492",
"-+ c #758894",
"@O c #638598",
"jO c #9C9B9A",
"FX c #90979B",
"g. c #698A9C",
"jX c #969C9E",
"JO c #87979F",
"kO c #A2A09F",
"A@ c #8496A1",
"!X c #8195A2",
"HX c #99A0A4",
"SX c #A2A6A8",
",o c #96A2AB",
">o c #99A6AF",
"9O c #819EB0",
"{X c #ABAFB2",
"&O c #AEB6BB",
"G@ c #A8B4BC",
"(O c #C3C0BE",
"F. c #93ADBD",
"Xo c #C0BFBE",
")O c #C3C3C1",
"fo c #B7BCC0",
"[o c #C3C3C2",
"D+ c #BAC0C4",
"J  c #A2B8C7",
"4O c #CCCCCB",
"g  c #ABBECB",
"'  c #A8BDCB",
",  c #BDC7CD",
"CO c #D2D1D0",
"So c #D5D2D0",
"f+ c #D2D1D1",
"G  c #AEC2D0",
"M. c #B1C3D1",
"(o c #D5D5D4",
"P+ c #D8D6D5",
"-. c #B7C8D4",
"8  c #CCD2D5",
"c  c #C6D0D5",
"fO c #D8D6D6",
"7o c #DBDADA",
"m. c #C3D2DB",
"[+ c #DEDEDD",
"S+ c #C9D4DC",
"S  c #C6D3DC",
"`o c #DEDEDE",
"%@ c #D2DAE0",
"^+ c #CCD8E1",
"7+ c #E4E3E2",
"{O c #4D6775",
",X c #CFDCE3",
"<O c #D2DDE4",
"1+ c #E7E7E6",
";O c #E7E7E7",
"K. c #D8E2E8",
"io c #DBE3E9",
"HO c #5C727D",
"po c #E1E8EC",
"eX c #56707E",
"p  c #DEE7EC",
"nX c #56707F",
"X. c #E1E8EE",
"i  c #E7EDF1",
"@. c #EAEEF2",
"'O c #F6F5F4",
"r  c #F0F3F5",
"c. c #F0F3F6",
"P. c #F6F8FA",
"I. c #FCFDFD",
"m+ c #5C7E91",
"s. c #5F7F91",
"0@ c #6E8794",
"4. c #628394",
"=+ c #748996",
"q@ c #6B899B",
"~X c #8397A3",
"|. c #7191A3",
"L@ c #A7A6A5",
"8O c #7796A7",
"j@ c #7D98A8",
"mO c #839DAC",
"FO c #9EACB5",
"D. c #89A5B6",
"gX c #AAB3B7",
"s+ c #B0B5B8",
"go c #B6BABD",
"k  c #A4B4BD",
"Q+ c #98B0BE",
"rO c #B6BABE",
"y@ c #C2C1C0",
"~o c #98B0BF",
"dO c #B9BEC2",
"N@ c #C2C4C4",
"+@ c #9EB5C3",
"oo c #C5C5C4",
"wO c #9BB4C4",
"{o c #C2C4C5",
"aX c #ADBDC7",
"LO c #CBCAC9",
"iO c #CBCACA",
"<  c #B9C4CB",
"!+ c #ADC0CC",
"Ro c #CECECD",
"}o c #CBCDCE",
"5  c #C2CACF",
"&o c #BCC8CF",
"r+ c #BFC9D0",
":X c #B0C4D1",
"]. c #B3C5D2",
"/+ c #C5CED4",
"To c #D7D7D6",
"`. c #B9CAD5",
")  c #BCCBD6",
"7X c #CED4D8",
"H+ c #DDDCDC",
"zO c #DADEE0",
"Ho c #E0E0E0",
"$o c #D4DCE1",
"e. c #CEDAE1",
"z. c #D1DBE2",
"}O c #4F6976",
"6o c #E6E5E5",
"1@ c #D7E0E6",
".O c #E6E5E6",
"R. c #D4DFE6",
"%. c #D7E0E7",
"o. c #DDE5EA",
"OX c #D7E3EA",
"I+ c #EFEEED",
"^O c #587280",
"b  c #E3EAEF",
"0. c #E6EBEF",
"J+ c #F2F2F1",
"#. c #E9EFF2",
"B  c #ECF0F3",
"n  c #E9EFF3",
"9. c #F2F5F7",
"mX c #647C89",
"/O c #73848D",
"3o c #FBFBFB",
"[X c #7F8B92",
"B. c #648596",
"g@ c #768B97",
"tX c #708998",
"f. c #648598",
"@+ c #9D9B9A",
"Z@ c #7F919B",
"6+ c #91979B",
"V. c #6A8A9C",
"f@ c #A09F9E",
":+ c #82929D",
"bX c #82959F",
"7O c #7694A4",
"Z. c #7694A5",
")X c #8599A5",
"_X c #8599A6",
"a+ c #91A0AA",
"no c #AFADAC",
"D@ c #91A3AE",
"p+ c #8EA2AE",
"i+ c #91A3AF",
"S. c #829EAF",
"^. c #88A3B3",
"Ao c #A9B1B7",
"lX c #BBBAB8",
"Qo c #BBBAB9",
"u@ c #BEBEBC",
"O@ c #94ADBD",
":O c #BEBEBE",
"H@ c #ACB8BF",
"hO c #C4C3C3",
"fX c #C1C2C3",
"IO c #C7C7C6",
"N. c #A0B7C5",
"). c #A3B8C6",
"KX c #CAC8C7",
"$  c #BBC3C7",
"KO c #C1C5C8",
"2  c #B5C1C8",
"w+ c #B2C3CC",
"@  c #C1C8CC",
"1O c #BEC7CE",
"Yo c #D0D0CF",
"DO c #C1CBD0",
"|X c #D3D1D1",
":  c #C4CCD2",
"f  c #B8C8D3",
"F  c #B5C7D3",
"#+ c #C1CED5",
"PO c #D6D5D5",
"9  c #CAD1D5",
"yo c #BECDD6",
"c+ c #DCDAD9",
"9+ c #D9D9D9",
"B+ c #C7D3DA",
"4o c #DCDADA",
"d  c #C4D2DC",
" O c #DFDEDE",
"A+ c #D0D9DF",
"'. c #CAD7DF",
"Io c #E2E2E1",
"/  c #CDD8E0",
"x  c #DCE0E3",
"K+ c #E8E7E6",
"T  c #D3DDE5",
"!O c #546C79",
"]o c #E8E7E7",
"vO c #516B7A",
"w. c #D9E2E8",
"R+ c #EBEBEA",
"=O c #EBEBEB",
"q. c #DFE7EC",
"<X c #DFE7ED",
"L. c #E8EDF1",
")o c #F4F4F3",
".+ c #6C7D86",
"}  c #EEF2F4",
"ZX c #607986",
"PX c #697C87",
"NX c #637A87",
"XX c #EBF1F5",
"N  c #F4F7F8",
"y  c #F4F7F9",
"t  c #F7F8FA",
"+o c #57798C",
"_o c #FDFDFC",
"-@ c #FDFDFD",
"n+ c #5D7E90",
"+O c #608294",
"3+ c #6F8795",
"*+ c #728895",
"%X c #6F8A98",
"`X c #819099",
"|+ c #9F9D9C",
"|  c #6F8D9D",
"#O c #6F90A1",
"hX c #8497A1",
"^X c #8497A3",
"~. c #7E9BAC",
".X c #7E9BAD",
"u+ c #93A5B0",
"Vo c #84A0B0",
"F@ c #99AAB4",
"y+ c #99AAB5",
"=X c #8AA5B6",
"TX c #96ACB8",
"MO c #90AAB9",
"EX c #A2B0B9",
".o c #B1B5B9",
"Wo c #BDBCBB",
"/. c #90AABA",
"++ c #BDBCBC",
"6O c #9CB1BC",
"so c #BABBBC",
".  c #AEB7BD",
"^o c #A2B3BF",
"=o c #A8B8C2",
"F+ c #C6C5C4",
"G. c #9FB5C4",
"4@ c #C9C9C8",
"o  c #BDC5C9",
"k. c #AEC0CB",
"9o c #CFCECD",
"$+ c #C0C9CE",
"+  c #C3CACE",
"H  c #ABBFCD",
"L+ c #D2D2D0",
"8+ c #D2D2D1",
"2. c #B1C4D0",
"=  c #C6CED2",
"%O c #B4C5D2",
"o+ c #D5D6D5",
"J. c #B7C9D4",
"`+ c #D5D6D7",
"#o c #C3D0D7",
"ho c #D8D7D7",
">X c #BDCED9",
"z  c #D2D8DA",
"JX c #DEDCDB",
"E. c #C6D4DD",
"t. c #C9D5DE",
"]X c #E1E0DF",
"yO c #E1E0E0",
"e+ c #D5DCE1",
"EO c #4A6473",
"Y  c #CCD9E1",
"'+ c #E4E4E3",
" + c #4D6876",
"I  c #D8E0E4",
"i@ c #E1E3E5",
"B@ c #EAE9E9",
"~  c #DBE4E9",
"p. c #DBE4EA",
"T+ c #EDEDEC",
"T. c #DEE5EB",
"u. c #E4EAEF",
"oX c #E1E9EF",
"sO c #E4EAF0",
"!  c #EAEFF2",
"Q  c #F0F4F6",
"LX c #6B7E89",
"MX c #657C89",
"{. c #5F8091",
"b. c #FFFFFF",
"3X c #FCFEFF",
"q+ c #5F8092",
">@ c #628496",
"%+ c #748A96",
"4+ c #748A98",
";+ c #83929B",
"&X c #6E8B9C",
";@ c #A19F9E",
">+ c #8C989F",
"Mo c #6E8E9F",
"QX c #8094A1",
"M+ c #7794A5",
"xO c #95A1A8",
"A. c #7A98A9",
"uX c #89A0AE",
"1o c #A4ACB1",
"}+ c #B6B5B4",
"o@ c #8CA7B7",
"XO c #B9B9B8",
"K  c #92A9B8",
"Co c #92ACBB",
"qO c #92ACBC",
"<o c #9EB0BD",
"UO c #C2C2C1",
"tO c #BFC1C3",
"6X c #B9BFC4",
"IX c #C8C7C5",
"O+ c #C8C7C6",
"]  c #A1B7C5",
"dX c #B6C1C7",
"kX c #CBC8C7",
"3  c #B6C1C8",
"#  c #BFC7CA",
"1  c #B6C1C9",
"O  c #BFC7CB",
":. c #A7BCCA",
"$@ c #C8CACC",
"@@ c #A7BCCB",
"$X c #B9C5CC",
"$O c #AAC0CE",
"{  c #B0C2CE",
"`  c #ADC1CE",
"Y+ c #D4D4D3",
"q  c #C2CBD2",
"v  c #BCC9D2",
"~+ c #BCCCD6",
"*O c #DAD9D8",
"2O c #D7D8D9",
"zo c #DAD9D9",
/* pixels */
"                . X o O + + @ # $ %                             ",
"        $ & * = - ; = : > , < 1 2 3 4 5 6 7                     ",
"    8 9 0 q w e r t y u i p a s d f g h j k l z                 ",
"  x c v a b n m t M N B V C Z A S D F G H J K L P               ",
"  I U Y T R V B E W Q ! U ~ ^ / ( ) _ ` ' ] [ { }               ",
"  |  ...X.o.O.+.@...#.V $.%.&.*.=.-.;.:.>.,.<.1.2.              ",
"  3.4.5.6.7.8.9.E m ..0.q.w.T e.r.t.A y.u.i.p._ a.              ",
"  s.d.f.g.h.j.k.l.z.o.U x.8.c.W v.b.n.E U ^ m.M.N.              ",
"  s.B.V.C.Z.A.S.D.F.G.H.J.*.K.L.P.I.U.Y.T.R.E.W.N.              ",
"  Q.B.V.C.!.~.^./.(.)._.`.'.w.L.P.I.U.L.T.R.E.].[.              ",
"    {.}.|. X.X^./.(.)._.`.'.w.L.P.I.U.XXoXOX+X@X                ",
"    #X$X%X&X*X=X-X;X:.:X>X,X<X1X2Xb.3X4X5X6X7X,                 ",
"  8X9X0XqXwXeXrXtXyXuXiXpXaXsXdXfXgXhXjXkXlXzXxXcX              ",
"  vXbXnXmXMXNXBXVXVXBXBXCXZXAXSXDXFXGXHXJXKXLXPXIXUXYX          ",
"  TXRXEXWXQX!X~X^X/X/X(X)X_X`X'Xb.]X[X{X}X|X o.oXoooOo          ",
"  +o@o#o$o%o&o*o=o-o;o:o>o,o<o1o2o3o4o5o6o7o8o9o0oqo            ",
"  s.woeorotoyouoio$.poaosodofogohob.jokolozoxocovobo  nomo      ",
"  s.B.V.MoNoBoVoCo] ZoAoSoDoFoGo2oHoJoKoLoPoIoUoYoToRoEoWoQo    ",
"  s.B.V.C.!.~.^.!o~o[.^o/o(o)o_o`o'o]o[o{o}o|o O.OcoXOoOOO      ",
"    +O@O#O!.~.^./.(.).$O%O&O*O=O-O;O:O>O,O<O1O2O3O7o4O5O        ",
"      6O7O8O9O0OqOwOeO{XrOtOyOJouOiOpOaOsO<O^ dOfOxogOhOjOkOlO  ",
"    zOxOcOvObOnOmOMONOBOVOCOZO-OAOSODOFOGOHOJOKOLOPOIOUOooooYO  ",
"  TOROEOWOQO!OWO~O^O/O(O)O_Oxo`O'O]O[O{O}O|O +.+X+o+iOO+0o++@+  ",
"  #+$+%+&+*+=+%+=+*+-+;+:+>+,+yO<+1+2+3+4+4+5+6+7+8+9+0+        ",
"  q+w+e+r+t+y+u+i+i+i+p+a+s+d+f+g+h+j+k+l+z+x+c+5oTov+b+        ",
"  n+m+M+N+B+V+C+Z+A+S+D+F+G+0of+H+joJ+K+L+P+I+U+Y+T+R+Ho_OE+    ",
"  s.B.W+g.NoS.Q+!+~+^+/+(+)+_+`+ OZOv+'+]+Lo[+Io{+}+|+ @ O.@    ",
"  s.B.V.C.!.*XX@o@O@+@@@#@$@%@&@dof+*@]o;O=O=@b.-@;@    @+      ",
"  :@>@V.C.!.~.^./.(.).` ,@<@1@2@3@gO4@5@6@R+7@8@b.9@            ",
"    0@q@w@8Oe@^./.(.)._.r@t@& y@u@hOi@2@p@[+a@s@d@b.f@          ",
"        g@h@j@k@l@z@x@c@>Xv@iob@n@m@M@N@B@]XV@  C@@+            ",
"                Z@A@S@D@F@EXG@H@J@xO  K@noL@                    "
};

/* XPM */
static const char *database_accept_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 630 2",
"(X c #D7DBDC",
"R+ c #CBD7DF",
"+X c #C8D6DF",
"9O c #E0D8DE",
"5o c #D1DCE2",
"jX c #526B78",
"wO c #556C79",
"W+ c #ECF4ED",
"7@ c #B2DC97",
"IO c #EFF8EF",
"7. c #E0E7EB",
"SO c #58707E",
".O c #F2F9F0",
"O. c #E0E7EC",
"OX c #DAE5EC",
"9+ c #DDE0EB",
"DO c #5B7481",
"lo c #E3E8EE",
"Q@ c #ECE8EE",
"NO c #ECEEF0",
"E@ c #F2F3F2",
"R@ c #F2F3F3",
"`X c #ECE8F0",
"NX c #617986",
"L+ c #AFDBA1",
"uo c #189546",
"M  c #FBFCFC",
"hX c #6D838E",
"n. c #FBFCFD",
"[O c #738895",
"C. c #7090A1",
"N@ c #D0E9B5",
"q@ c #7090A2",
"]o c #60B95C",
"tX c #7C94A3",
"gX c #8B9CA5",
">O c #60B960",
"U@ c #97A3AA",
"]X c #829CAB",
"7+ c #91A4AF",
"#+ c #D3EABF",
"C@ c #A2D269",
"PO c #CAE7C1",
"+@ c #A8D46B",
"hO c #84C871",
"fX c #A9B5BC",
"(+ c #6BB81A",
"%  c #B2BBBF",
"-X c #94AEBE",
"0+ c #96D175",
"3o c #A0B5C1",
"L  c #A9B8C2",
"a. c #9DB4C3",
"l+ c #A6BAC5",
"X  c #B8C0C5",
"@X c #A9BBC9",
"H. c #A9BECB",
"Z@ c #B1DA83",
"c@ c #AFC3D1",
"W. c #B2C4D1",
"*  c #C7CED2",
";  c #C7CED3",
"/o c #B8C9D5",
"m+ c #C7D1D9",
"=. c #C1CFD9",
"Do c #C7D4DD",
"/X c #D6D6DD",
"A  c #CDD9E1",
"3@ c #77BF38",
"x+ c #D6DFE4",
"Qo c #E5DBE1",
"^  c #D3DEE5",
"c+ c #D9E0E6",
"CO c #546D7A",
"C  c #DCE4EA",
"b@ c #DFE8ED",
"qo c #0B8F3D",
"V  c #E5EAEF",
"x. c #E8EEF1",
"7O c #35A944",
"1. c #E8EEF2",
".. c #EBEFF3",
"9X c #697D87",
"u  c #EEF3F6",
"E  c #F1F4F7",
"~O c #9BCE4A",
"y+ c #FDFEFC",
"rO c #637E8D",
"3. c #5A7B8E",
";O c #9FD7A6",
"`O c #6F8592",
"u@ c #95CC51",
"&o c #96CEA7",
"d. c #638495",
"1o c #638497",
"Uo c #47AF54",
"j@ c #78919F",
"^O c #71C05B",
"!O c #8FCA5C",
"}o c #6EBF5D",
"{o c #6BBE5D",
"QX c #8498A3",
"WX c #8498A4",
".+ c #86CA60",
"mo c #38AD5F",
"Ko c #44B160",
"h. c #7B98A8",
"w@ c #7E9CAD",
"-o c #3BA562",
"Bo c #3BAB66",
"l@ c #87A2B2",
"mO c #74C46F",
"@o c #9FB0BC",
"2+ c #A8B6BF",
"(. c #99B1C0",
"4X c #AEBBC3",
"C+ c #85C223",
"x@ c #A5BBC9",
"i@ c #7FC024",
"7X c #C0C7CA",
"7  c #C0C7CB",
"0  c #BAC5CB",
"V+ c #79BE27",
"6@ c #79BE28",
";. c #AEC1CE",
"&  c #C6CCD0",
"<@ c #7FC02A",
"_  c #B4C6D2",
"xX c #CCD1D5",
"jo c #BACBD6",
"D  c #BDCCD7",
"@+ c #A7D588",
"l. c #C3D1DA",
"kX c #47616E",
"   c None",
":+ c #DEE3E0",
"w  c #D2D9DE",
"*. c #C9D6DE",
"<. c #CCD7DF",
"qX c #4D6673",
"v+ c #D5DDE2",
")o c #E4F1E9",
"i+ c #67B83A",
"Eo c #049439",
"EO c #E1F3EC",
"y. c #D8E1E7",
"e@ c #D8E1E8",
"_X c #E4E2E7",
"P  c #E7E9EB",
"5O c #80C897",
"=+ c #97CB41",
"cX c #EDF1F2",
"Y. c #E7ECF1",
"CX c #5F7885",
"Q+ c #7FC346",
"8. c #EDF1F4",
"VX c #627987",
":o c #89C59D",
"nO c #7FC349",
"U. c #F3F6F8",
"eX c #627C8A",
"0o c #1C9648",
"zo c #FFFDFF",
"<o c #5F8192",
"F+ c #64BA55",
"`o c #37AB55",
"tO c #748E9E",
"!X c #8095A1",
"f@ c #CBE7B5",
"so c #7190A2",
"Io c #55B55D",
"Po c #52B45D",
"No c #34AA5F",
"!. c #7795A6",
"do c #7795A7",
"2o c #7D9AAA",
"QO c #7CC263",
"*X c #7D9AAB",
"8+ c #8C9FAC",
"I@ c #A4B0B6",
"Co c #8FA9BA",
"oo c #A1B2BD",
"*O c #9BB3C2",
"h  c #A1B5C2",
"}+ c #9EB4C3",
"zX c #B6BFC5",
"[. c #A1B8C6",
"=O c #A4B9C7",
"So c #AABECC",
"l  c #BFC8CE",
"8X c #C5CDD0",
"-  c #C5CDD1",
"6  c #C8CED2",
")X c #D4D8DA",
"5X c #CED6DB",
"3X c #CBD5DB",
"(  c #C5D3DC",
"%o c #DDEAE1",
"Yo c #7FC98E",
"s  c #CED9E2",
"co c #008E36",
" + c #DAE0E5",
"Z  c #D4DEE5",
"(o c #D4DBE5",
"ZO c #556D7A",
"g+ c #E6E4E8",
":@ c #DAE3EA",
"R  c #DDE4EA",
"U  c #E3E9EE",
"D@ c #ECE9EF",
"XX c #E9EEF2",
"g@ c #F2F1F4",
"m  c #EFF3F5",
"^+ c #F8FCF9",
"1X c #EFF3F7",
"GX c #647B89",
"t+ c #FBFDFB",
"r+ c #FBFDFC",
"sO c #EFF0F9",
"2X c #FBFDFE",
"Q. c #5E7F91",
">@ c #FBFAFF",
"YO c #7BC051",
"I+ c #67889A",
"zO c #75BE56",
"}. c #67889B",
"BO c #82949E",
"gO c #6FBF5D",
"Lo c #4BB35D",
"Jo c #36AC5D",
"UX c #8598A4",
"H@ c #8B9DA8",
" X c #7997A8",
"bo c #36AC62",
"3O c #7BC364",
"=o c #3CA563",
"]+ c #85A1B1",
"5. c #91A8B5",
"%O c #8BA6B6",
"uX c #94A9B6",
"j. c #94ACBA",
"z@ c #91ABBB",
"j  c #9DAFBC",
"#X c #ACB7BD",
"p@ c #7DBF1C",
"iX c #A0B3BF",
"[  c #97B0BF",
"Xo c #A3B4BF",
"1@ c #80C01D",
"2@ c #83C11E",
"$@ c #86C21E",
"o@ c #83C120",
";X c #9DB5C4",
"%@ c #83C121",
"*o c #57B176",
" o c #AFBEC7",
"s@ c #7DBF23",
"4@ c #7ABE24",
">. c #A6BBC9",
"K+ c #74BC25",
"4  c #BBC5CB",
"aX c #B5C3CB",
"jO c #90CD7E",
"X@ c #7ABE27",
"ho c #AFC1CD",
"_. c #ACC0CD",
",. c #AFC1CE",
">  c #C1CAD0",
"aO c #B5C6D1",
"M+ c #C1CDD5",
"6. c #C4D1D8",
"}X c #CAD3D9",
"ko c #C7D5DE",
"r. c #CAD6DF",
"0X c #4E6672",
" . c #D3DCE2",
",o c #E5DCE1",
"&. c #D0DBE3",
"S@ c #EEF4E9",
"PX c #DCE2E6",
"a  c #D6E0E7",
"q+ c #8FC83D",
"E+ c #DCE5EB",
"fO c #80C33F",
"e  c #E2E7EB",
"$. c #DFE6EB",
"d+ c #8FC840",
"+. c #E5EBEF",
"SX c #5A7381",
"i. c #EBF0F3",
"BX c #607885",
"8@ c #F1EFF3",
"xO c #B1DB9F",
"oO c #17A048",
"W  c #F4F6F8",
"Fo c #F4F0F9",
"v. c #FAFBFC",
"t@ c #FDFCFF",
"-@ c #698492",
"/+ c #99D3A8",
"to c #1D994E",
"B+ c #BDE2AD",
"~o c #638598",
"o+ c #A1D156",
"g. c #698A9C",
"vo c #26A256",
"A@ c #CCE7B2",
"G@ c #8496A1",
"TX c #8195A2",
"6+ c #90A3AF",
"$O c #819EB0",
"cO c #AEDDC2",
" O c #7AC46D",
"u+ c #B7E0C9",
"+o c #9FB1BC",
"F. c #93ADBD",
";o c #53AE73",
"J  c #A2B8C7",
"pO c #A8BDCA",
"sX c #BAC6CC",
"g  c #ABBECB",
"'  c #A8BDCB",
",  c #BDC7CD",
"w+ c #88C42A",
"G  c #AEC2D0",
"M. c #B1C3D1",
"B@ c #B3DA84",
"p+ c #8BC52D",
"-. c #B7C8D4",
"8  c #CCD2D5",
"c  c #C6D0D5",
"@@ c #D5EDE1",
"m. c #C3D2DB",
"S  c #C6D3DC",
"|+ c #C6D3DD",
"xo c #74C28E",
",X c #CFDCE3",
"v@ c #CFDCE4",
"K. c #D8E2E8",
"6o c #DBE3E9",
"XO c #EAF7F0",
"wX c #56707E",
"p  c #DEE7EC",
"O+ c #F6FBF1",
"bX c #56707F",
"++ c #F3FAF2",
"X. c #E1E8EE",
"#@ c #61B742",
"AX c #5C7582",
"i  c #E7EDF1",
"@. c #EAEEF2",
"r@ c #EAEEF3",
"r  c #F0F3F5",
"c. c #F0F3F6",
"Wo c #89CCA0",
"P. c #F6F8FA",
"GO c #687F8D",
"io c #8CCAA2",
"I. c #FCFDFD",
"T@ c #F9F6FC",
"m@ c #FCFDFF",
"j+ c #5C7E91",
"s. c #5F7F91",
"9@ c #6E8794",
"4. c #628394",
"]O c #748996",
"0@ c #6B899B",
"<O c #70BF5C",
"[o c #67BC5C",
"RX c #8397A3",
"|. c #7191A3",
"JO c #7FC761",
"#O c #7796A7",
"Mo c #37AC60",
"k@ c #7D98A8",
"yO c #839DAC",
"~X c #95A3AF",
"V@ c #A3D36A",
"A+ c #A6D46D",
"D. c #89A5B6",
"TO c #91CA6E",
"G+ c #81C019",
"e+ c #A9D571",
"dO c #7FC771",
"k  c #A4B4BD",
"U+ c #98B0BE",
"Zo c #98B0BF",
"!+ c #81C01D",
"_+ c #81C01F",
"Ao c #A1B6C5",
"pX c #ADBDC7",
"6X c #BCC5CA",
"<  c #B9C4CB",
"Y+ c #ADC0CC",
"5  c #C2CACF",
"|X c #BCC8CF",
"1+ c #BFC9D0",
":X c #B0C4D1",
"]. c #B3C5D2",
"`. c #B9CAD5",
")  c #BCCBD6",
"Vo c #6ABD87",
"/O c #DDE2E0",
"0O c #DADEE0",
"Ho c #009235",
"{X c #D4DCE1",
"e. c #CEDAE1",
"z. c #D1DBE2",
"P+ c #E0DAE1",
"R. c #D4DFE6",
"MO c #E3E1E6",
"%. c #D7E0E7",
"o. c #DDE5EA",
"po c #ECE7E9",
"`+ c #75BF3F",
"7o c #E3E7EC",
"b  c #E3EAEF",
"0. c #E6EBEF",
"#. c #E9EFF2",
"ZX c #5E7784",
"yo c #0F9141",
"B  c #ECF0F3",
"n  c #E9EFF3",
",@ c #BBDE9E",
"-O c #E9E9F4",
"9. c #F2F5F7",
"nX c #647C89",
"FO c #647C8A",
"8o c #F5F3F7",
"W@ c #F5F3F8",
" @ c #F2F2FC",
"$+ c #FEFFFF",
"B. c #648596",
"h@ c #768B97",
"rX c #708998",
"f. c #648598",
"6O c #4BB155",
"F@ c #7F919B",
"eo c #249E54",
"V. c #6A8A9C",
"ro c #249E55",
";+ c #78C35A",
"vX c #82959F",
".@ c #C7E5B6",
"@O c #7694A4",
"Z. c #7694A5",
"y@ c #D0E8B9",
"1O c #7BC465",
"J@ c #91A3AE",
"5+ c #91A3AF",
"S. c #829EAF",
"^. c #88A3B3",
"uO c #91A9B8",
"P@ c #A9B4BB",
"{+ c #94ADBD",
"Oo c #A0B1BD",
"N. c #A0B7C5",
"). c #A3B8C6",
"$  c #BBC3C7",
"4O c #D9EED3",
"2  c #B5C1C8",
"5@ c #86C325",
",+ c #B2C3CC",
"@  c #C1C8CC",
"s+ c #86C327",
"^X c #BEC1CC",
":  c #C4CCD2",
"f  c #B8C8D3",
"F  c #B5C7D3",
"(O c #C1CED5",
">o c #D6E4DA",
"9  c #CAD1D5",
"4o c #BECDD6",
"z+ c #C7D3DA",
"n+ c #CAD4DB",
"d  c #C4D2DC",
"Ro c #6CC28C",
"b+ c #D0D9DF",
"'. c #CAD7DF",
"/  c #CDD8E0",
"x  c #DCE0E3",
"*+ c #80C139",
"HO c #D9DFE4",
":O c #32A739",
"To c #E5F5EA",
"$o c #D9D9E2",
"T  c #D3DDE5",
"AO c #546C79",
"eO c #516B7A",
"w. c #D9E2E8",
"q. c #DFE7EC",
"RO c #F4FAF1",
"<X c #DFE7ED",
"X+ c #95CB42",
"KO c #98CC42",
"oX c #E2E8EE",
"8O c #8DD09B",
"L. c #E8EDF1",
"HX c #5D7584",
"}  c #EEF2F4",
"KX c #F1F3F5",
"MX c #637A87",
"n@ c #EEF2F6",
"N  c #F4F7F8",
"D+ c #F1F9FA",
"y  c #F4F7F9",
"t  c #F7F8FA",
"'X c #57798C",
"d@ c #8FC94E",
"h+ c #5D7E90",
"JX c #6F8490",
"wo c #1A994C",
"&+ c #74BD4F",
"|O c #6F8492",
"{O c #728894",
"!o c #608294",
"'O c #728895",
"LO c #8CC854",
"%X c #6F8A98",
"9o c #99D1AB",
"|o c #68BC58",
"|  c #6F8D9D",
",O c #68BC5A",
"^o c #6F90A1",
"EX c #8497A3",
"'o c #56B65F",
"no c #38AC60",
"kO c #7DC362",
"lO c #80C463",
"~. c #7E9BAC",
".X c #7E9BAD",
"4+ c #93A5B0",
"fo c #84A0B0",
"~+ c #A4D369",
"K@ c #99AAB4",
"3+ c #99AAB5",
"=X c #8AA5B6",
"Z+ c #7CBE16",
"LX c #96ACB8",
"O@ c #7CBE17",
"IX c #A2B0B9",
"/. c #90AABA",
"a@ c #7FBF19",
"+O c #9CB1BC",
".  c #AEB7BD",
"#o c #A5B4BF",
".o c #A8B8C2",
"G. c #9FB5C4",
"o  c #BDC5C9",
"&@ c #73BB25",
"k. c #AEC0CB",
")O c #C0C9CE",
"+  c #C3CACE",
"H  c #ABBFCD",
"2. c #B1C4D0",
"OO c #D2E4D8",
"=  c #C6CED2",
"J. c #B7C9D4",
"[X c #C3D0D7",
">X c #BDCED9",
"z  c #D2D8DA",
"'+ c #E1ECE0",
"E. c #C6D4DD",
"t. c #C9D5DE",
"<+ c #D5DCE1",
"VO c #4A6473",
"Y  c #CCD9E1",
"I  c #D8E0E4",
"%+ c #7DC691",
"~  c #DBE4E9",
"p. c #DBE4EA",
"M@ c #F3F8EF",
"=@ c #E7E2E9",
"lX c #5C737E",
"T. c #DEE5EB",
"DX c #597280",
"u. c #E4EAEF",
"FX c #597281",
"!  c #EAEFF2",
"Y@ c #EDEAF1",
"N+ c #E7E8F2",
"Q  c #F0F4F6",
"_o c #19A048",
"mX c #657C89",
"S+ c #F9FDFA",
"-+ c #9ACE4D",
"{. c #5F8091",
"b. c #FFFFFF",
">+ c #5F8092",
"}O c #718693",
";@ c #628496",
"_O c #748A96",
"Go c #98D2AB",
"*@ c #BCE1AD",
"vO c #55B455",
"&X c #6E8B9C",
"ao c #6E8E9F",
"YX c #8094A1",
"bO c #70C05E",
"k+ c #7794A5",
"WO c #55B45F",
"qO c #95A1A8",
"A. c #7A98A9",
"yX c #89A0AE",
"2O c #7FC569",
"[+ c #8CA7B7",
"K  c #92A9B8",
"L@ c #A7B3BB",
"f+ c #8BCC71",
"go c #92ACBB",
"&O c #92ACBC",
"iO c #9EB3C1",
"a+ c #84C221",
"UO c #91CB78",
"]  c #A1B7C5",
"dX c #B6C1C7",
"H+ c #84C223",
"3  c #B6C1C8",
")+ c #87C324",
"#  c #BFC7CA",
"1  c #B6C1C9",
"J+ c #84C225",
"O  c #BFC7CB",
":. c #A7BCCA",
"$X c #B9C5CC",
"{  c #B0C2CE",
"`  c #ADC1CE",
"q  c #C2CBD2",
"v  c #BCC9D2",
"T+ c #BCCCD6",
/* pixels */
"                . X o O + + @ # $ %                             ",
"        $ & * = - ; = : > , < 1 2 3 4 5 6 7                     ",
"    8 9 0 q w e r t y u i p a s d f g h j k l z                 ",
"  x c v a b n m t M N B V C Z A S D F G H J K L P               ",
"  I U Y T R V B E W Q ! U ~ ^ / ( ) _ ` ' ] [ { }               ",
"  |  ...X.o.O.+.@...#.V $.%.&.*.=.-.;.:.>.,.<.1.2.              ",
"  3.4.5.6.7.8.9.E m ..0.q.w.T e.r.t.A y.u.i.p._ a.              ",
"  s.d.f.g.h.j.k.l.z.o.U x.8.c.W v.b.n.E U ^ m.M.N.              ",
"  s.B.V.C.Z.A.S.D.F.G.H.J.*.K.L.P.I.U.Y.T.R.E.W.N.              ",
"  Q.B.V.C.!.~.^./.(.)._.`.'.w.L.P.I.U.L.T.R.E.].[.              ",
"    {.}.|. X.X^./.(.)._.`.'.w.L.P.I.U.XXoXOX+X@X                ",
"    #X$X%X&X*X=X-X;X:.:X>X,X<X1Xb.b.2XXX3X4X5X6X                ",
"  7X8X9X0XqXwXeXrXtXyXuXiXpXaXsXdXfXgXhXjXkXlXzXxX              ",
"  cXvXbXnXmXMXNXBXBXNXNXVXNXCXZXAXSXDXFXZXGXHXJXKX              ",
"  LXPXIXUXYXTXRXEXWXWXWXQXQXQXWXRX!X~X^X/X(X)X_X`X              ",
"  'X]X[X{X}X|X o.oXoooOo+o+o+o@o#o$o%o&o*o=o-o;o:o>o,o          ",
"  s.<o1o2o3o4o5o6o$.7.$.$.$.O.7o8o9o0oqowoeorotoyouoiopo        ",
"  s.B.V.aosodofogo] hojokoa lozoxocovobonomomoMoNoBocoVopo      ",
"  s.B.V.C.!.~.^.CoZoAoSoJ.DoFoGoHoJoKoLoPoIoIoUoYoToRoEoWoQo    ",
"    !o~o^o!.~.^./.(.)._./o(o)o_o`o'o]o[o{o}o|o O.Ob.XOnooOOO    ",
"      +O@O#O$O%O&O*O=O` /o-O;O:O>O,O<O1O2O2O3O4Ob.b.5O6O7O8O9O  ",
"    0OqOwOeOrOtOyOuOiOpOaOsOdOfOgOhOjOkOlOzOxOb.b.cOvObOnOmOMO  ",
"  NOBOVOCOZOAOCOSODONXFOGOHOJOKOLOPOIOUOYOTOROb.EOWOQO!O~O^O/O  ",
"  (O)O_O`O'O]O_O]O[O{O}O|O +.+X+o+O+b.++@+#+b.$+%+&+*+=+-+;+:+  ",
"  >+,+<+1+2+3+4+5+6+5+7+8+9+0+q+w+e+r+b.t+y+b.u+i+p+a+s+d+f+g+  ",
"  h+j+k+l+z+x+c+v+b+n+m+M+N+B+V+C+Z+A+S+b.b.D+F+G+H+J+C+K+L+P+  ",
"  s.B.I+g.soS.U+Y+T+R+Z E+Y.W+Q+!+J+Z+~+^+b./+(+)+J+J+_+`+'+    ",
"  s.B.V.C.!.*X]+[+{+}+:.F |+ @.@X@o@J+O@+@@@#@$@J+J+%@&@*@=@    ",
"  -@;@V.C.!.~.^./.(.)._.`.r.:@>@,@<@1@H+2@3@4@5@H+1@6@7@8@      ",
"    9@0@q@#Ow@^./.(.)._.`.'.e@r@t@y@u@i@p@a@1@p@s@d@f@g@        ",
"        h@j@k@l@z@*Ox@c@>Xv@b@n@m@b.M@N@B@V@C@Z@A@S@D@          ",
"                F@G@H@J@K@IXL@P@I@U@  Y@T@R@E@W@Q@              "
};

/* XPM */
static const char *database_down_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 625 2",
"Lo c #82C98B",
".o c #D7D5DD",
"w. c #CBD7DF",
" X c #C8D6DF",
"QX c #E0E1E2",
"ro c #76C291",
"]+ c #93CA3F",
"VO c #58707D",
"1o c #E0E7EB",
"7X c #556F7D",
"BO c #58707E",
"<o c #E0E7EC",
"j+ c #E3E2ED",
"sO c #7EC343",
"BX c #5B7482",
"I@ c #F2F3F3",
"!X c #ECE8F0",
"mX c #617986",
"DO c #617987",
"H. c #F2F6F8",
"EO c #6FBE4C",
"M  c #FBFCFC",
"U@ c #F8F5FB",
"x. c #FBFCFD",
"r+ c #5E7E90",
":@ c #81C44F",
"4@ c #708794",
"3. c #618294",
"{o c #5DB853",
")O c #768C98",
"V@ c #CDE8B2",
"M. c #7090A1",
"Z+ c #7090A2",
"y+ c #7391A2",
"}o c #60B95C",
"`o c #5DB85C",
"bo c #30A95C",
"PO c #A6D8B5",
"D@ c #889BA5",
"_o c #51B45E",
"F@ c #8EA0AA",
"w+ c #87C964",
"qX c #889EAC",
"V. c #829FAF",
"8@ c #88A4B4",
"KO c #9CD06D",
"^+ c #94AEBD",
"{X c #A0B2BD",
"$X c #94AEBE",
"u+ c #A0B5C1",
"h@ c #9AB3C2",
"uX c #B2BEC4",
"'X c #A9B8C3",
"t. c #9DB4C3",
"X  c #B8C0C5",
".X c #A9BBC8",
"S. c #A9BECB",
"#  c #C1C9CC",
"TX c #C1C3CC",
"k@ c #AFC3D0",
"I. c #B2C4D1",
";  c #C7CED2",
",  c #C4CDD2",
":  c #C7CED3",
"[o c #6CC084",
"do c #B8C9D4",
" + c #EEF6DE",
"h+ c #C4D0D8",
"*X c #BECED9",
"z  c #D3D8DA",
"*. c #C1CFD9",
"So c #C7D4DD",
"8+ c #D6EEE5",
"d+ c #D3DBE1",
"A  c #CDD9E1",
"6+ c #8FC738",
"++ c #DCF0E8",
"~  c #D3DEE5",
"$@ c #D9E0E7",
"h. c #DCE4E9",
"C  c #DCE4EA",
"2o c #E2E9ED",
"-X c #DFE8ED",
"G+ c #E2E6ED",
"j. c #E2E9EE",
"V  c #E5EAEF",
"}. c #E2E9EF",
"9@ c #E8EBF0",
"<. c #E8EEF1",
"@. c #EBEFF3",
"Y@ c #F1EEF4",
"@+ c #9BCE48",
"c. c #F1F4F6",
"cX c #637B88",
"b+ c #68BA48",
"T  c #F1F4F7",
"2. c #5A7B8E",
"o@ c #BAE0A6",
"X+ c #FDFEFE",
"wo c #1A974B",
"SX c #728692",
"w@ c #92CB51",
"u. c #638495",
"9X c #6F8896",
"`O c #758A97",
"Io c #41AD54",
"dO c #68BD58",
"hO c #5FB758",
"pX c #8799A2",
"UX c #8498A3",
"q@ c #CCE6B6",
"IX c #8498A4",
"Jo c #41B05F",
"Yo c #3BAE61",
"=o c #7B98A8",
"^X c #7E99A9",
"7@ c #7E9CAD",
":+ c #96A7B3",
"iX c #A5B2B9",
"j  c #9CAFBC",
"vo c #4AB372",
"=@ c #7FC01D",
"W. c #99B1C0",
"0+ c #82C11F",
"ao c #9FB6C4",
"u@ c #7FC023",
",@ c #85C223",
"M+ c #79BE23",
"`X c #B1BFC8",
"0  c #BAC5CB",
"L+ c #85C226",
"O  c #C0C7CC",
"-. c #AEC1CE",
"3X c #C3CBCF",
")  c #B4C6D2",
"EX c #CFD8D5",
"[O c #CFD8D7",
"D  c #BDCCD7",
"<X c #CCD4DA",
"6O c #80CB8B",
"   c None",
"w  c #D2D9DE",
"&. c #C9D6DE",
"e. c #C9D6DF",
"Ro c #74C490",
"fo c #D5E0E7",
"D. c #D8E1E8",
"4+ c #8EC83F",
"q+ c #94CA40",
"|. c #DBE5EC",
"o. c #E1E7EC",
"VX c #597381",
"J. c #E7ECF1",
";@ c #97CB45",
"6. c #EDF1F4",
"vX c #627986",
"Z@ c #F3F0F6",
"6o c #169445",
"/. c #F3F6F8",
"a@ c #F9F8FA",
"7o c #169447",
"&o c #5F8193",
"^O c #718794",
"}  c #6E8C9C",
"p@ c #CBE7B2",
"0X c #7A93A1",
"|o c #55B55D",
"+o c #31A05A",
"zo c #31A95E",
"Y. c #7795A6",
"f@ c #7D97A7",
"io c #7795A7",
"aO c #6DC064",
"Q+ c #7D9AAC",
",+ c #92A4B0",
"po c #839FB0",
"wX c #92A7B5",
"k  c #A4B3BD",
"eX c #9EB1BD",
"[X c #A1B2BE",
" o c #A7B4BF",
"$O c #9BB3C2",
"h  c #A1B5C2",
"/+ c #9EB4C3",
"). c #A1B8C6",
"%O c #A4B9C7",
"d. c #ADBFCA",
"2X c #C2C9CD",
"l  c #BFC8CD",
"1  c #BCC7CD",
"IO c #85C880",
"iO c #B3C4D0",
"B@ c #B2DA83",
"!O c #C5CDD2",
"QO c #BCCAD2",
"hX c #CED3D6",
".+ c #D7EEE1",
"/  c #C5D3DC",
"p+ c #D4DEE3",
"YO c #DAEFE8",
"NO c #4C6775",
",O c #A0D490",
"^o c #D1D7E1",
"x  c #DDE1E3",
"*O c #85CB91",
"C@ c #EFF6EA",
"Z  c #D4DEE5",
"0O c #556D79",
">o c #DAE3E8",
"qO c #526C7B",
"F+ c #DAE3EA",
"ko c #06903B",
"go c #E0E5EA",
"Y  c #DDE4EA",
"JO c #96CC42",
"Q  c #E3E9EE",
".O c #129D44",
"{. c #E9EEF2",
"NX c #5E7684",
"m  c #EFF3F5",
"u  c #EFF3F6",
";X c #EFF3F7",
"0@ c #F8F6FA",
"U. c #5E7F91",
"'O c #738995",
"~O c #798E9B",
"+X c #6D8A9B",
"MO c #82949D",
"gO c #AFDCB0",
"co c #9AD5B0",
"3O c #69BD59",
")o c #39AD59",
"S@ c #82949F",
"zO c #BBE0B4",
"OO c #7693A3",
" O c #42B05E",
"]. c #7997A8",
"[. c #7F9CAD",
"H@ c #9DACB5",
"@O c #8BA6B6",
"@o c #45A86A",
"]  c #97B0BF",
"<@ c #80C01D",
"yO c #9DB2C0",
"-+ c #ACBAC2",
"R+ c #83C11F",
"K+ c #80C01F",
"fO c #8ACB77",
"'+ c #83C120",
"%X c #9DB5C4",
"&+ c #ACBDC7",
":. c #A6BBC9",
"7  c #C1C7CB",
"5  c #BBC5CB",
"l+ c #74BC26",
"1@ c #7DBF27",
"!. c #ACC0CD",
"@  c #C4CBCF",
"<  c #C1CAD0",
"-  c #C7CFD2",
"_X c #BEC9D1",
";o c #BBCBD4",
"f+ c #CDD7DD",
"*+ c #D6DDE2",
"g. c #D0DBE2",
"c+ c #8FC839",
"$+ c #E5EEE7",
"%. c #D0DBE3",
"a+ c #D9E1E6",
"LO c #EBF6EB",
"Go c #029339",
"a  c #D6E0E7",
"4o c #E8E6E8",
"CO c #576F7C",
"%o c #EBE4E8",
"fX c #5A707C",
"{+ c #DFF2EF",
"ZO c #576F7D",
"pO c #E2E4EA",
"e  c #E2E7EB",
"#. c #DFE6EB",
"O. c #E5EBEF",
"CX c #5A7381",
"k+ c #AEDA9D",
"4X c #667A85",
" . c #EBF0F3",
"bX c #607885",
")+ c #EBEAF3",
"%@ c #F4F3F7",
"O@ c #F4F0F6",
"R  c #F4F6F8",
"P@ c #F7F4F9",
"ZX c #697E8C",
"l. c #FAFBFC",
"v@ c #FDF9FF",
"}+ c #6BBC50",
"{O c #74C252",
"_O c #758B97",
"9o c #209D51",
"'o c #5FB854",
"Qo c #638598",
"Ko c #41AE55",
"p. c #698A9C",
"KX c #8195A1",
"|O c #A7D35D",
"-O c #5CB761",
"rO c #819BAA",
"+O c #819EAF",
"DX c #93AAB6",
"xO c #6BBF71",
"Z. c #93ADBC",
"#O c #93ADBD",
"&  c #B1BABF",
",X c #ABB8C0",
"Mo c #53B476",
"rX c #ABBBC5",
"J  c #A2B8C7",
"S+ c #A8BDC9",
"XO c #C9E6D3",
"yX c #B7C2C9",
"(+ c #A8BDCA",
"`  c #A8BDCB",
"[  c #B1C3CE",
"v+ c #EAF4D9",
"1. c #B1C3CF",
"kO c #6EC081",
"G  c #AEC2D0",
"=  c #C6CDD1",
"=+ c #C3CCD2",
"b. c #B1C3D1",
"=. c #B7C8D4",
"]o c #92CF87",
"O+ c #D8EEE1",
")X c #CCD5DB",
"v. c #C3D2DB",
"S  c #C6D3DC",
",. c #CCD8E0",
"=X c #CFDCE4",
"=O c #2EA63D",
"r. c #DBE3E9",
"Fo c #7DC595",
"P  c #E7EAEB",
"p  c #DEE7EC",
"z@ c #DEE7ED",
"Do c #EAE5EC",
".. c #E1E8EE",
"ho c #F0EDEE",
"mO c #EDEFF0",
"(o c #0D9B42",
"To c #0D9842",
"i  c #E7EDF1",
"+. c #EAEEF2",
"RO c #9DCE47",
"r  c #F0F3F5",
"aX c #6B808B",
"vO c #88C74B",
"wO c #627D8C",
"F. c #F6F8FA",
"i@ c #91CA4C",
"G. c #FCFDFD",
"t+ c #5C7E90",
"FO c #6B808F",
"y. c #5F7F91",
"8o c #229C52",
"A@ c #7D8F9A",
"V+ c #68889B",
"cO c #6ABD58",
"2O c #6DBE59",
"UO c #C2E2B4",
"uo c #7191A2",
"LX c #8397A3",
"lo c #2EA65C",
"Uo c #43B05E",
"no c #31A75E",
"6@ c #7796A7",
"a. c #7A97A8",
"1+ c #8FA1AE",
"<+ c #92A5B0",
"N@ c #A3D36A",
"!+ c #86A1B2",
"TO c #70BF6A",
"tO c #8FA7B5",
"C. c #89A5B6",
"y@ c #7EBF16",
"t@ c #7BBE17",
"Bo c #8FAABA",
"|+ c #7EBF19",
">@ c #7BBE1A",
"x+ c #7EBF1B",
"Vo c #98B0BF",
"r@ c #7EBF1D",
"X@ c #78BD23",
"uO c #A7BBC8",
"2  c #B9C4CB",
"Zo c #AABFCC",
"so c #ADC0CC",
"6  c #C2CACF",
"WX c #CED7D3",
"&X c #B0C4D1",
"(. c #B3C5D2",
"P+ c #ECF6DD",
"~. c #B9CAD5",
"~o c #B9CAD6",
"(  c #BCCBD6",
"jO c #A0D287",
"H+ c #E0ECDF",
"f. c #C2D0D9",
"dX c #46606D",
"[+ c #F2F8E4",
"8O c #DADEE0",
"6X c #4C6572",
"|  c #D1DBE0",
"(X c #D4DCE1",
"2+ c #D7DAE1",
"WO c #E3EDE6",
"s  c #CEDAE2",
"I+ c #DAF0E9",
"2@ c #B2DB92",
"FX c #DDE2E6",
"L. c #D4DFE6",
"b@ c #EFF4EB",
"$. c #D7E0E7",
"3o c #E0E3E8",
"X. c #DDE5EB",
"J+ c #75BF40",
"No c #EFEEED",
"b  c #E3EAEF",
"Eo c #F2E9EE",
"jX c #ECF0F1",
"8. c #E6EBF0",
"n  c #E9EFF2",
"B  c #ECF0F3",
"nX c #617886",
"7. c #F2F5F7",
"zX c #647C89",
"8X c #617B89",
"3@ c #F8F7FA",
"o+ c #FEFFFE",
"+@ c #6A8492",
"HO c #6CBF52",
"s@ c #768B96",
"n. c #648596",
"0o c #219D51",
"(O c #768B98",
"i. c #648598",
"qo c #249E54",
"eO c #738D9C",
"m. c #6A8A9C",
"lO c #75BF5A",
"bO c #6FC05F",
"N. c #7694A5",
"@X c #7C99AB",
"R. c #88A3B3",
"K@ c #A3AFB6",
";+ c #9DADB8",
"Oo c #48AA6C",
"A+ c #94ADBB",
"}X c #A0B1BD",
"oO c #A0B4BF",
"-o c #9DB3BF",
"XX c #AFB9BF",
"xo c #51B675",
".@ c #83C221",
"'  c #A0B7C5",
"Q. c #A3B8C6",
"%  c #BBC3C7",
"m+ c #86C323",
"*  c #BBC3C8",
"4  c #B5C1C8",
"tX c #B2C0C8",
"z+ c #86C324",
" @ c #86C325",
"Xo c #CDE1D2",
"T+ c #86C326",
"g  c #ACBECB",
"1X c #BEC7CC",
"5+ c #86C327",
"*@ c #7ABF29",
"f  c #B8C8D3",
"F  c #B5C7D3",
"8  c #CDD2D5",
"9  c #CAD1D5",
"c  c #C7D0D5",
"W+ c #E2F1E0",
"g+ c #CAD4DB",
"d  c #C4D2DC",
"^. c #CAD7DF",
"#@ c #CAD7E0",
"^  c #CDD8E0",
"s+ c #D6DEE3",
"#o c #75BC8D",
"E+ c #77BE39",
"U  c #D3DDE5",
"0. c #D9E2E8",
"oo c #7EC294",
"N+ c #ABDA98",
"9. c #DFE7EC",
"&O c #E2DFEA",
"e+ c #EEEFF0",
"7O c #EEE6ED",
"}O c #95CB44",
"SO c #5D7583",
"k. c #E8EDF1",
"mo c #0E9542",
"{  c #EEF2F4",
"x@ c #EEF2F6",
"N  c #F4F7F8",
"y  c #F4F7F9",
"t  c #F7F8FA",
"~X c #57798C",
"eo c #179547",
"#+ c #77C14D",
"%+ c #5D7E90",
":X c #FAFCFE",
"Wo c #608294",
"_+ c #BDE0AC",
"]O c #758997",
"5@ c #6C899B",
"C+ c #698B9C",
":O c #6BBD57",
"d@ c #78909E",
";O c #65BB59",
"!o c #6F90A1",
"1O c #71BF5C",
">O c #74C05C",
"n@ c #CFE9B6",
"PX c #8497A3",
"9O c #909EA5",
"4O c #5FB960",
"Ho c #38AC60",
"T. c #7E9BAC",
"G@ c #96A6B0",
"g@ c #87A1B1",
">+ c #93A5B1",
"YX c #99A4B1",
"M@ c #A4D36B",
"4. c #90A7B4",
"#X c #8AA5B5",
"s. c #93ABB9",
"E. c #90AABA",
"Y+ c #7FBF19",
"n+ c #7FBF1A",
"|X c #9FB2BD",
".  c #AEB7BD",
"L  c #A8B8C2",
"A. c #9FB5C4",
"$o c #C9E1D0",
"o  c #BDC5C9",
"j@ c #A5BAC9",
"/o c #CCE2D3",
"`+ c #73BB26",
"+  c #C3CACE",
"H  c #ABBFCD",
"7+ c #EAF5D9",
">  c #C6CED2",
"m@ c #B3DB85",
"GO c #CCD6D6",
"Ao c #B7C9D4",
"Po c #68BF86",
"5. c #C3D0D7",
"i+ c #C3D0D8",
"<O c #71C287",
"RX c #D2D5D8",
"l@ c #BDCED9",
">X c #C9D2D9",
"D+ c #C6D4DC",
"P. c #C6D4DD",
"5X c #4D6571",
":o c #CFDAE1",
"q. c #CFDAE2",
"sX c #506976",
"I  c #D8E0E4",
"!  c #DBE4E9",
",o c #DEE5EA",
"to c #EDEDEC",
"K. c #DEE5EB",
"&@ c #B3DB98",
"AO c #59727F",
"5o c #80C496",
"lX c #56717F",
"nO c #EAECEE",
"W  c #EAEFF2",
"MX c #5F7785",
"B+ c #F0EBF2",
"5O c #3DAC46",
"E  c #F0F4F6",
"AX c #627887",
"9+ c #67BA48",
"xX c #657C89",
"U+ c #67BA49",
"c@ c #F9FAFB",
"_. c #5F8091",
"z. c #FFFFFF",
"@@ c #628496",
"/O c #748A97",
"OX c #6E8897",
"*o c #628497",
"`. c #68899B",
"kX c #83959F",
"yo c #6E8EA0",
"JX c #8094A1",
"'. c #7192A3",
"HX c #8699A5",
"B. c #7A98A9",
"L@ c #9EAAB0",
"3+ c #88CB6A",
"J@ c #A1AEB6",
"~+ c #8CA7B8",
"K  c #92A9B8",
"GX c #A4B2BB",
"]X c #A4B5BF",
"gX c #B3BDC2",
"Co c #A1B7C5",
"-@ c #84C223",
"3  c #B6C1C8",
"e@ c #7EC025",
"$  c #BFC7CB",
";. c #A7BCCA",
"jo c #5BB67B",
"oX c #B9C5CC",
">. c #B0C2CE",
"_  c #ADC1CE",
"q  c #C2CBD2",
"v  c #BCC9D2",
"/X c #BFCDD5",
/* pixels */
"                . X o O + @ # $ % &                             ",
"        * = - ; = : > , < 1 2 3 4 3 5 6 ; 7                     ",
"    8 9 0 q w e r t y u i p a s d f g h j k l z                 ",
"  x c v a b n m t M N B V C Z A S D F G H J K L P               ",
"  I b A U Y V B T R E W Q ! ~ ^ / ( ) _ ` ' ] [ {               ",
"  } |  ...X.o.O.+.@.n V #.$.%.&.*.=.-.;.:.>.,.<.1.              ",
"  2.3.4.5.#.6.7.T r @.8.9.0.~ q.w.e.s 0.O. .r.) t.              ",
"  y.u.i.p.a.s.d.f.g.h.j.k.6.u R l.z.x.c.Q ~ v.b.'               ",
"  y.n.m.M.N.B.V.C.Z.A.S.=.&.D.k.F.G.H.J.K.L.P.I.'               ",
"  U.n.m.M.Y.T.R.E.W.Q.!.~.^.0.k.F.G./.k.K.L.P.(.).              ",
"    _.`.'.].[.R.E.W.Q.!.~.^.0.k.F.G./.{.}.|. X.X                ",
"    XXoXOX+X@X#X$X%X;.&X*X=X-X;Xz.z.:Xk.>X,X<X1X                ",
"  2X3X4X5X6X7X8X9X0XqXwXeXrXtXyXuXiXpXaXsXdXfXgXhX              ",
"  jXkXlXzXxXcXvXbXbXnXmXnXbXMXNXBXVXVXCXmXZXAXSXr               ",
"  DXFXGXHXJXKXLXPXIXIXIXIXUXUXIXLXKXYXTXRXEXWXQX!X              ",
"  ~X^X/X(X)X_X`X'X]X[X{X}X}X}X|X o.oXoooOo+o+o@o#o$o%o          ",
"  y.&o*o=o-o;o:o>o,oo.<o1oo.2o3o4o5o6o7o8o9o0oqowoeoroto        ",
"  y.n.m.youoiopoE.aosodoP.fogohojokolozoxococovobonomoMoNo      ",
"  y.n.m.M.Y.T.R.BoVoCoZoAoSoDoFoGoHoJoKoLoz.z.PoIoUoYoToRoEo    ",
"    WoQo!oY.T.R.E.W.Q.!.~o^o/o(o)o_o`o'o]oz.z.[o{o}o|o O.OXO    ",
"      oOOOY.+O@O#O$O%O_ ~o&O*O=O-O;O:O>O,Oz.z.<O1O2O3O4O5O6O7O  ",
"    8O9O0OqOwOeOrOtOyOuOiOpOaOsOdOfOgOhOjOz.z.kOlOzOxOcOvObOnO  ",
"  mOMONOBOVOCOZOAOSODOzXFOGOHOJOKOLOz.POIOz.z.kOUOz.YOTOROEOWO  ",
"  QO!O~O^O/O(O)O)O_O`O'O]O[O{O}O|O +z.z..+X+o+O+z.z.++>O@+#+$+  ",
"  %+&+*+=+-+;+:+>+,+,+<+1+2+3+4+5+6+7+z.z.z.z.z.z.8+9+0+q+w+e+  ",
"  r+t+y+u+i+p+a+s+d+f+g+h+j+k+l+z+x+c+v+z.z.z.z.8+b+n+m+M+N+B+  ",
"  y.n.V+C+Z+[.A+S+doD+%.F+G+H+J+K+L+x+c+P+z.z.I+U+Y+T+R+E+W+    ",
"  y.n.m.M.Y.Q+!+~+^+/+(+F So)+_+`+'+L+x+]+[+{+}+|+ @.@X@o@O@    ",
"  +@@@m.M.Y.T.R.E.W.Q.!.~.#@$@%@&@*@=@-@x+;@:@>@,@<@1@2@3@      ",
"    4@5@M.6@7@8@E.W.Q.!.~.^.0.9@0@q@w@e@r@t@y@=@u@i@p@a@        ",
"        s@d@f@g@E.h@j@k@l@=Xz@x@c@v@b@n@m@M@N@B@V@C@Z@          ",
"                A@S@D@F@G@H@J@K@L@9O    P@I@I@U@Y@              "
};

////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig type definition
//

IMPLEMENT_CLASS(frmDeviceConfig, wxFrame)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig event table definition
//

BEGIN_EVENT_TABLE(frmDeviceConfig, wxFrame)

////@begin frmDeviceConfig event table entries
  EVT_CLOSE( frmDeviceConfig::OnCloseWindow )
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
////@end frmDeviceConfig event table entries

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

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig constructors
//

frmDeviceConfig::frmDeviceConfig() {
    Init();
}

frmDeviceConfig::frmDeviceConfig(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig creator
//

bool frmDeviceConfig::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    ////@begin frmDeviceConfig creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    ////@end frmDeviceConfig creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmDeviceConfig destructor
//

frmDeviceConfig::~frmDeviceConfig() {
    ////@begin frmDeviceConfig destruction
    ////@end frmDeviceConfig destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmDeviceConfig::Init() {
    ////@begin frmDeviceConfig member initialisation
  m_comboNodeID = NULL;
  m_bLevel2 = NULL;
  m_BtnActivateInterface = NULL;
  m_choiceBook = NULL;
  m_panel0 = NULL;
  m_gridRegisters = NULL;
  m_gridAbstractions = NULL;
  m_gridDM = NULL;
  m_StatusWnd = NULL;
  m_chkFullUppdate = NULL;
  m_chkMdfFromFile = NULL;
  m_ctrlButtonLoadMDF = NULL;
  m_ctrlButtonWizard = NULL;
    ////@end frmDeviceConfig member initialisation

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
    ////@begin frmDeviceConfig content construction
  frmDeviceConfig* itemFrame1 = this;

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
  m_comboNodeID = new wxComboBox;
  m_comboNodeID->Create( itemToolBar25, ID_COMBOBOX4, wxEmptyString, wxDefaultPosition, wxSize(410, -1), m_comboNodeIDStrings, wxCB_DROPDOWN );
  if (frmDeviceConfig::ShowToolTips())
    m_comboNodeID->SetToolTip(_("Set nickname or GUID for node here"));
  m_comboNodeID->SetBackgroundColour(wxColour(255, 255, 210));
  itemToolBar25->AddControl(m_comboNodeID);
  wxBitmapButton* itemBitmapButton37 = new wxBitmapButton;
  itemBitmapButton37->Create( itemToolBar25, ID_CHECK_LEVEL2, itemFrame1->GetBitmapResource(wxT("find.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmDeviceConfig::ShowToolTips())
    itemBitmapButton37->SetToolTip(_("Test if device is present."));
  itemToolBar25->AddControl(itemBitmapButton37);
  itemToolBar25->AddSeparator();
  m_bLevel2 = new wxCheckBox;
  m_bLevel2->Create( itemToolBar25, ID_CHECKBOX_LEVEL22, _("Level II"), wxDefaultPosition, wxDefaultSize, 0 );
  m_bLevel2->SetValue(false);
  itemToolBar25->AddControl(m_bLevel2);
  itemToolBar25->AddSeparator();
  m_BtnActivateInterface = new wxToggleButton;
  m_BtnActivateInterface->Create( itemToolBar25, ID_TOGGLEBUTTON1, _("Connected"), wxDefaultPosition, wxSize(120, -1), 0 );
  m_BtnActivateInterface->SetValue(true);
  if (frmDeviceConfig::ShowToolTips())
    m_BtnActivateInterface->SetToolTip(_("Acticate/Deactivate the interface"));
  m_BtnActivateInterface->SetForegroundColour(wxColour(255, 255, 255));
  m_BtnActivateInterface->SetBackgroundColour(wxColour(165, 42, 42));
  m_BtnActivateInterface->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
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

  wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer58->Add(itemBoxSizer61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_StatusWnd = new wxHtmlWindow;
  m_StatusWnd->Create( itemPanel44, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(680, 180), wxHW_SCROLLBAR_AUTO|wxRAISED_BORDER|wxWANTS_CHARS|wxHSCROLL|wxVSCROLL );
  itemBoxSizer61->Add(m_StatusWnd, 0, wxALIGN_RIGHT|wxALL, 0);

  wxBoxSizer* itemBoxSizer63 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer58->Add(itemBoxSizer63, 0, wxALIGN_TOP|wxALL, 5);

  m_chkFullUppdate = new wxCheckBox;
  m_chkFullUppdate->Create( itemPanel44, ID_CHECKBOX_FULL_UPDATE, _("Full Uppdate"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkFullUppdate->SetValue(false);
  itemBoxSizer63->Add(m_chkFullUppdate, 0, wxALIGN_LEFT|wxALL, 5);

  m_chkMdfFromFile = new wxCheckBox;
  m_chkMdfFromFile->Create( itemPanel44, ID_CHECKBOX_MDF_FROM_FILE, _("Use local MDF"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkMdfFromFile->SetValue(false);
  itemBoxSizer63->Add(m_chkMdfFromFile, 0, wxALIGN_LEFT|wxALL, 5);

  wxButton* itemButton66 = new wxButton;
  itemButton66->Create( itemPanel44, ID_BUTTON16, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton66->SetDefault();
  itemBoxSizer63->Add(itemButton66, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  m_ctrlButtonLoadMDF = new wxButton;
  m_ctrlButtonLoadMDF->Create( itemPanel44, ID_BUTTON17, _("Load defaults"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlButtonLoadMDF->Enable(false);
  itemBoxSizer63->Add(m_ctrlButtonLoadMDF, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  itemBoxSizer63->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlButtonWizard = new wxButton;
  m_ctrlButtonWizard->Create( itemPanel44, ID_BUTTON19, _("Wizard"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ctrlButtonWizard->Enable(false);
  itemBoxSizer63->Add(m_ctrlButtonWizard, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    ////@end frmDeviceConfig content construction

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

void frmDeviceConfig::OnCloseWindow(wxCloseEvent& event) {
    wxBusyCursor wait;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Closing window..."),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL);

    progressDlg.Pulse(_("Closing communication interface..."));

    // Close the communication channel
    disableInterface();

    progressDlg.Update(100);
    progressDlg.Pulse(_("Done!"));

    // Hide window
    Show(false);

    // Destroy the window
    Destroy();

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enableInterface
//

bool frmDeviceConfig::enableInterface(void) {
    bool rv = true;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Opening interface..."),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL);

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

                if (fetchIterfaceGUID()) {

                    progressDlg.Pulse(_("Interface GUID found."));

                    // Fill the combo
                    wxString str;
                    cguid guid = m_ifguid;
                    for (int i = 1; i < 256; i++) {
                        guid.setLSB(i);
                        guid.toString(str);
                        m_comboNodeID->Append(str);
                    }

                    guid.setLSB(1);
                    guid.toString(str);
                    m_comboNodeID->SetValue(str);

                } else {
                    m_comboNodeID->SetValue(_("Enter full GUID of remote node here"));
                }

            } else {

                // No interface selected
                m_comboNodeID->SetValue(_("Enter full GUID of remote node here"));

            }

        }
    } else {
        progressDlg.Pulse(_("Failed to open Interface."));
        wxMessageBox(_("Failed to Open Interface."));

        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
        return false;
    }

    // Move to top of zorder
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

bool frmDeviceConfig::disableInterface(void) {
    m_csw.doCmdClose();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemExitClick
//

void frmDeviceConfig::OnMenuitemExitClick(wxCommandEvent& event) {
    Close();
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpHelpClick
//

void frmDeviceConfig::OnMenuitemVscpHelpClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpFaqClick
//

void frmDeviceConfig::OnMenuitemVscpFaqClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpShortcutsClick
//

void frmDeviceConfig::OnMenuitemVscpShortcutsClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpThanksClick
//

void frmDeviceConfig::OnMenuitemVscpThanksClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpCreditsClick
//

void frmDeviceConfig::OnMenuitemVscpCreditsClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpVscpSiteClick
//

void frmDeviceConfig::OnMenuitemVscpVscpSiteClick(wxCommandEvent& event) {
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(true); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpAboutClick
//

void frmDeviceConfig::OnMenuitemVscpAboutClick(wxCommandEvent& event) {
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnBitmapbuttonTestDeviceClick
//

void frmDeviceConfig::OnBitmapbuttonTestDeviceClick(wxCommandEvent& event) {
    
    unsigned char nodeid;

    ::wxBeginBusyCursor();

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Testing if device is present"),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());

        if ((0 == nodeid) || (nodeid > 254)) {
            wxMessageBox(_("Invalid Node ID! Must be between 1-254"));
            ::wxEndBusyCursor();
            return;
        }

        unsigned char val;
        if (m_csw.readLevel1Register(nodeid, 0xd0, &val)) {
            wxMessageBox(_("Device found!"));
        } else {
            wxMessageBox(_("Device was not found! Check nodeid."));
        }

    } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

        // Get the destination GUID
        cguid destGUID;
        destGUID.getFromString(m_comboNodeID->GetValue());

        unsigned char val;
        if (m_csw.readLevel2Register(m_ifguid,
                m_bLevel2->GetValue() ? 0xffffffd0 : 0xd0,
                &val,
                &destGUID,
                &progressDlg,
                m_bLevel2->GetValue())) {
            wxMessageBox(_("Device found!"));
        } else {
            wxMessageBox(_("Device was not found! Check interface GUID + nodeid."));
        }

    }

    ::wxEndBusyCursor();
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnInterfaceActivate
//

void frmDeviceConfig::OnInterfaceActivate(wxCommandEvent& event) {
    wxBusyCursor wait;

    if (!m_BtnActivateInterface->GetValue()) {
        disableInterface();
        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
    } else {
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

void frmDeviceConfig::OnRegisterEdited(wxGridEvent& event) {
    uint8_t val;
    wxString strBuf;
    wxString str;

    if (2 == event.GetCol()) {
        str = m_gridRegisters->GetCellValue(event.GetRow(), event.GetCol());
        val = readStringValue(str);
        strBuf.Printf(_("0x%02lx"), val);
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

void frmDeviceConfig::OnCellLeftClick(wxGridEvent& event) {
    // Save clicked cell/row
    m_lastLeftClickCol = event.GetCol();
    m_lastLeftClickRow = event.GetRow();

    if (ID_GRID_REGISTERS == event.GetId()) {
        // Select the row
        m_gridRegisters->SelectRow(m_lastLeftClickRow);
    } else if (ID_GRID_ABSTRACTIONS == event.GetId()) {
        // Select the row
        m_gridAbstractions->SelectRow(m_lastLeftClickRow);
    } else if (ID_GRID_DM == event.GetId()) {
        // Select the row
        m_gridDM->SelectRow(m_lastLeftClickRow);
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmDeviceConfig::ShowToolTips() {
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmDeviceConfig::GetBitmapResource(const wxString& name) {
    // Bitmap retrieval
    ////@begin frmDeviceConfig bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("open.xpm"))
  {
    wxBitmap bitmap(open_xpm);
    return bitmap;
  }
  else if (name == _T("save.xpm"))
  {
    wxBitmap bitmap(save_xpm);
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
  else if (name == _T("paste.xpm"))
  {
    wxBitmap bitmap(paste_xpm);
    return bitmap;
  }
  else if (name == _T("Print.xpm"))
  {
    wxBitmap bitmap(print_xpm);
    return bitmap;
  }
  else if (name == _T("find.xpm"))
  {
    wxBitmap bitmap(find_xpm);
    return bitmap;
  }
  return wxNullBitmap;
    ////@end frmDeviceConfig bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmDeviceConfig::GetIconResource(const wxString& name) {
    // Icon retrieval
////@begin frmDeviceConfig icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  else if (name == _T("icons/png/32x32/database_process.png"))
  {
    wxIcon icon(database_process_xpm);
    return icon;
  }
  else if (name == _T("icons/png/32x32/database_accept.png"))
  {
    wxIcon icon(database_accept_xpm);
    return icon;
  }
  else if (name == _T("icons/png/32x32/database_down.png"))
  {
    wxIcon icon(database_down_xpm);
    return icon;
  }
  return wxNullIcon;
////@end frmDeviceConfig icon retrieval
}


//////////////////////////////////////////////////////////////////////////////
// writeChangedLevel1Registers
//

bool frmDeviceConfig::writeChangedLevel1Registers(unsigned char nodeid) {
    int i;
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;
    uint16_t page = 0xffff;
    uint16_t newpage;
    uint8_t reg;

    // Nothing to the first time.
    if (m_bFirstRead) return true;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Writing modified registers"),
            256,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    // *********************************
    // Write data from changed registers
    // *********************************

    progressDlg.Pulse(_("Looking for changed registers..."));

    for (i = 0; i < m_gridRegisters->GetNumberRows(); i++) {

        if (!progressDlg.Update(i)) {
            rv = false;
            break;
        }

        if (*wxRED == m_gridRegisters->GetCellTextColour(i, 2)) {

            progressDlg.Pulse(wxString::Format(_("Writing register %d"), i));

            reg = getRegFromCell(i);
            newpage = getPageFromCell(i);
            uint16_t savepage = m_csw.getRegisterPage(this, nodeid, &m_ifguid);

            if (newpage != savepage) {
                if (!m_csw.setRegisterPage(nodeid, newpage)) {
                    wxMessageBox(_("Failed to write/set new page."));
                    break;
                }
            }

            val = readStringValue(m_gridRegisters->GetCellValue(i, 2));
            if (m_csw.writeLevel1Register(nodeid, reg, &val)) {

                // Update display
                strBuf = getFormattedValue(val);
                m_gridRegisters->SetCellValue(i, 2, strBuf);
                m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, i, 2);
                m_gridRegisters->SetCellTextColour(i, 2, *wxBLUE);
                m_gridRegisters->SelectRow(i);
                m_gridRegisters->MakeCellVisible(i, 2);
                m_gridRegisters->Update();

                // restore page if needed
                if (newpage != savepage) {
                    if (!m_csw.setRegisterPage(nodeid, savepage)) {
                        wxMessageBox(_("Failed to restore page."));
                        break;
                    }
                }

                updateDmGridConditional(reg, newpage);
                updateAbstractionGridConditional(reg, newpage);

            } else {
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

bool frmDeviceConfig::writeChangedLevel2Registers(void) {
    int i;
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;
    uint32_t reg = 0;
    uint16_t newpage = 0;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Writing Registers"),
            256,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    // *********************************
    // Write data from changed registers
    // *********************************

    progressDlg.Pulse(_("Writing changed registers!"));

    // Get the destination GUID
    cguid destGUID;
    uint8_t nodeid;
    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());

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

            progressDlg.Pulse(wxString::Format(_("Writing register %d"), i));

            reg = getRegFromCell(i);
            newpage = getPageFromCell(i);
            uint16_t savepage = m_csw.getRegisterPage(this,
                    (USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                    nodeid : destGUID.getLSB(),
                    &m_ifguid,
                    &destGUID,
                    m_bLevel2->GetValue());

            // Set new page
            if (newpage != savepage) {
                if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                        nodeid : destGUID.getLSB(),
                        newpage,
                        &m_ifguid,
                        &destGUID,
                        m_bLevel2->GetValue())) {
                    wxMessageBox(_("Failed to write/set new page."));
                    break;
                }
            }

            val = readStringValue(m_gridRegisters->GetCellValue(i, 2));

            if (!m_csw.writeLevel2Register(m_ifguid,
                    reg,
                    &val,
                    destGUID,
                    &progressDlg,
                    m_bLevel2->GetValue())) {
                wxMessageBox(_("Failed to update data."));
            }

            // Update display
            strBuf = getFormattedValue(val);
            m_gridRegisters->SetCellValue(i, 2, strBuf);
            m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, i, 2);
            m_gridRegisters->SetCellTextColour(i, 2, *wxBLUE);
            m_gridRegisters->SelectRow(i);
            m_gridRegisters->MakeCellVisible(i, 2);
            m_gridRegisters->Update();

            updateDmGridConditional(reg, newpage);
            updateAbstractionGridConditional(reg, newpage);

        } // Changed register
    } // for

    return rv;
}



//////////////////////////////////////////////////////////////////////////////
// writeStatusInfo
//

void frmDeviceConfig::writeStatusInfo(void) {
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
    reverseGUID(guid);
    writeGuidArrayToString(m_stdRegisters.getGUID(), str);
    strHTML += str;
    strHTML += _("<br>");



    strHTML += _("<b>MDF URL</b>: ");
    strHTML += _("<a href=\"");
    strHTML += m_stdRegisters.getMDF();
    strHTML += _("\">");
    strHTML += m_stdRegisters.getMDF();
    strHTML += _("</a>");
    strHTML += _("<br>");

    strHTML += _("<b>Alarm:</b> ");
    if (m_stdRegisters.getAlarm()) {
        strHTML += _("Yes");
    } else {
        strHTML += _("No");
    }
    strHTML += _("<br>");


    strHTML += _("<b>Node Control Flags:</b> ");
    if (m_stdRegisters.getNodeControl() & 0x10) {
        strHTML += _("[Register Write Protect] ");
    } else {
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

    } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

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
        } else {
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

void frmDeviceConfig::OnComboNodeIDSelected(wxCommandEvent& event) {
    // Clear all content
    clearAllContent();

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnComboNodeIDUpdated
//

void frmDeviceConfig::OnComboNodeIDUpdated(wxCommandEvent& event) {
    // Clear all content
    clearAllContent();

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////////
// clearAllContent
//

void frmDeviceConfig::clearAllContent(void) {
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

void frmDeviceConfig::fillStandardRegisters() {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
            _("89"));
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    writeGuidArrayToString(m_stdRegisters.getGUID(), str);
    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 3, _("GUID Byte 15, LSB\nGUID=") + str);

    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);


    m_gridRegisters->AppendRows(1);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D1");
    } else {
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
    } else {
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
    strBuf.Printf(_("0x%02lx"), 0xd3);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D3");
    } else {
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
    } else {
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
    strBuf.Printf(_("0x%02lx"), 0xd5);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D5");
    } else {
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
    } else {
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
    } else {
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
    strBuf.Printf(_("0x%02lx"), 0xd8);
    if (!m_bLevel2->GetValue()) {
        strBuf = _("D8");
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
    } else {
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
            strBuf.Printf(_("%02lX"), 0xe0 + i);
        } else {
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
        } else if (31 == i) {
            str = _(", LSB");
        } else {
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
    str = m_stdRegisters.getMDF();
    m_gridRegisters->SetCellValue(nFirstMDFrow, 3, _("Module Description File URL, MSB\n") + str);

    // Make all parts of the row visible
    m_gridRegisters->AutoSizeRow(nFirstMDFrow);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonUpdateClick
//

void frmDeviceConfig::OnButtonUpdateClick(wxCommandEvent& event) {
    
    wxString strPath;
    uint8_t nodeid = 0;
    cguid destGUID;

    ::wxBeginBusyCursor();

    // Check if we should do a full update
    if (m_chkFullUppdate->GetValue()) {

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

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Fetching status and MDF"),
            256,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    // Get nickname
    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        nodeid = readStringValue(m_comboNodeID->GetValue());
    } 
    else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
        destGUID.getFromString(m_comboNodeID->GetValue());
    }

    if (m_bFirstRead) {

        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

            // Fetch MDF from local file or server
            if (m_chkMdfFromFile->GetValue()) {

                wxStandardPaths stdpaths;

                // Get MDF from local file
                wxFileDialog dlg(this,
                        _("Choose file to load MDF from "),
                        stdpaths.GetUserDataDir(),
                        _(""),
                        _("Module Description Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));

                if (wxID_OK == dlg.ShowModal()) {
                    strPath = dlg.GetPath();
                }

            } 
            else {
                // Get MDF from device
                progressDlg.Pulse(_("Fetching MDF path from device."));
                strPath = m_csw.getMDFfromDevice1(nodeid);
            }

            // We need it to continue
            if (0 == strPath.Length()) {
                ::wxMessageBox(_("Empty MDF path returned."), _("VSCP Works"), wxICON_ERROR);
                ::wxEndBusyCursor();
                return;
            }

            // Load and parse the MDF
            if (!progressDlg.Pulse(_("Loading and parsing MDF."))) {
                ::wxEndBusyCursor();
                return;
            }
            
            m_mdf.load(strPath, m_chkMdfFromFile->GetValue());

            // Get the standard registers
            if (!progressDlg.Pulse(_("Reading standard registers."))) {
                ::wxEndBusyCursor();
                return;
            }

            // First part of standard registers
            m_csw.readLevel1Registers(this,
                    m_stdRegisters.m_reg,
                    nodeid, 0x80, 26);

            if (false == progressDlg.Pulse()) {
                ::wxEndBusyCursor();
                return;
            }

            // Second part of standard registers
            m_csw.readLevel1Registers(this,
                    (m_stdRegisters.m_reg + 0xD0 - 0x80),
                    nodeid, 0xD0, 48);

            // Get the application registers
            progressDlg.Pulse(_("Reading application registers."));
            if (false == progressDlg.Update(230)) {
                ::wxEndBusyCursor();
                return;
            }

            SortedArrayLong pageArray;
            uint32_t n = m_mdf.getPages(pageArray);

            if (false == progressDlg.Update(235)) {
                ::wxEndBusyCursor();
                return;
            }

            uint8_t tt = m_stdRegisters.getNickname();
            n = m_mdf.getNumberOfRegisters(0);

            if (false == progressDlg.Update(240)) {
                ::wxEndBusyCursor();
                return;
            }

            uint8_t val;
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

                    progressDlg.Pulse(_("Reading standard registers."));
                    progress += progress_count;
                    progressDlg.Update(progress);

                    if (false == progressDlg.Update(256 - m_mdf.m_list_register.GetCount() + cnt)) {
                        ::wxEndBusyCursor();
                        return;
                    }

                    // Add a new row
                    m_gridRegisters->AppendRows(1);

                    // Register
                    strBuf.Printf(_("%04X:%02X"), reg->m_nPage, reg->m_nOffset);
                    progressDlg.Pulse(_("Reading page:register: ") + strBuf);

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);

                    if (m_csw.readLevel1Register(nodeid, reg->m_nOffset, &val)) {
                        reg->m_value = val;
                        getFormattedValue(val);
                        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                                2,
                                getFormattedValue(val));
                        m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
                        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);
                    } 
                    else {
                        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                                2,
                                _("Read error"));
                        m_gridRegisters->SetCellAlignment(m_gridRegisters->GetNumberRows() - 1, 2, wxALIGN_CENTRE, wxALIGN_CENTRE);
                        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);
                    }

                    str.Printf(reg->m_strName +
                            _("\n") +
                            reg->m_strDescription);
                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
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

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
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
            }

            if (false == progressDlg.Update(255)) {
                ::wxEndBusyCursor();
                return;
            }

            // Fill grid with standard registers
            fillStandardRegisters();

            if (false == progressDlg.Update(240)) {
                ::wxEndBusyCursor();
                return;
            }

            // Write status
            writeStatusInfo();

            if (false == progressDlg.Update(255)) {
                ::wxEndBusyCursor();
                return;
            }
            
            m_bFirstRead = false;

        } 
        else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

            // Fetch MDF from local file or server
            if (m_chkMdfFromFile->GetValue()) {

                wxStandardPaths stdpaths;

                // Get MDF from local file
                wxFileDialog dlg(this,
                        _("Choose file to load MDF from "),
                        stdpaths.GetUserDataDir(),
                        _(""),
                        _("Module Description Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));

                if (wxID_OK == dlg.ShowModal()) {
                    strPath = dlg.GetPath();
                }

            } 
            else {
                // Get MDF from device
                progressDlg.Pulse(_("Fetching MDF path from device."));
                strPath = m_csw.getMDFfromDevice2( progressDlg,
                        m_ifguid,
                        destGUID,
                        m_bLevel2->GetValue());
            }

            // We need it to continue
            if (0 == strPath.Length()) {
                ::wxMessageBox(_("Empty MDF path returned."), _("VSCP Works"), wxICON_ERROR);
                ::wxEndBusyCursor();
                return;
            }

            // Load and parse the MDF
            if (!progressDlg.Pulse(_("Loading and parsing MDF."))) {
                ::wxEndBusyCursor();
                return;
            }

            if (!m_mdf.load(strPath, m_chkMdfFromFile->GetValue())) {
                // We try to continue anyway
            }

            // Get the standard registers
            if (!progressDlg.Pulse(_("Reading standard registers."))) {
                ::wxEndBusyCursor();
                return;
            }

            // First part of standard registers
            if (!m_csw.readLevel2Registers(this,
                    m_stdRegisters.m_reg,
                    m_ifguid,
                    m_bLevel2->GetValue() ? 0xFFFFFF80 : 0x80,
                    26,
                    &destGUID,
                    &progressDlg,
                    m_bLevel2->GetValue())) return;

            if (false == progressDlg.Pulse()) return;

            // Second part of standard registers
            if (!m_csw.readLevel2Registers(this,
                    (m_stdRegisters.m_reg + (0xD0 - 0x80)),
                    m_ifguid,
                    m_bLevel2->GetValue() ? 0xFFFFFFD0 : 0xD0,
                    48,
                    &destGUID,
                    &progressDlg,
                    m_bLevel2->GetValue())) return;

            // Get the application registers
            if (!progressDlg.Pulse(_("Reading application registers."))) return;

            SortedArrayLong pageArray;
            uint32_t n = m_mdf.getPages(pageArray);

            if (!progressDlg.Pulse()) return;

            uint8_t tt = m_stdRegisters.getNickname();
            n = m_mdf.getNumberOfRegisters(0);

            if (!progressDlg.Pulse()) return;

            uint8_t val;
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

                    //if ( !progressDlg.Update( _("Reading standard registers %0X02"), 256 - m_mdf.m_list_register.GetCount() + cnt  ) ) return;

                    // Add a new row
                    m_gridRegisters->AppendRows(1);

                    // Register
                    strBuf.Printf(_("%04X:%X"), reg->m_nPage, reg->m_nOffset);
                    progressDlg.Pulse(_("Reading page:register: ") + strBuf);

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1, 0, strBuf);
                    m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 0);
                    m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 0, fontBold);
                    m_gridRegisters->SetReadOnly(m_gridRegisters->GetNumberRows() - 1, 0);

                    if (m_csw.readLevel2Register(m_ifguid,
                            m_bLevel2->GetValue() ? (0xFFFFFFE0 + reg->m_nOffset) : reg->m_nOffset,
                            &val,
                            &destGUID,
                            &progressDlg,
                            m_bLevel2->GetValue())) {
                        reg->m_value = val;
                        getFormattedValue(val);
                        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                                2,
                                getFormattedValue(val));
                        m_gridRegisters->SetCellAlignment(wxALIGN_CENTRE, m_gridRegisters->GetNumberRows() - 1, 2);
                        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);
                    } else {
                        m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
                                2,
                                _("Read error"));
                        m_gridRegisters->SetCellAlignment(m_gridRegisters->GetNumberRows() - 1, 2, wxALIGN_CENTRE, wxALIGN_CENTRE);
                        m_gridRegisters->SetCellFont(m_gridRegisters->GetNumberRows() - 1, 2, fontBold);
                    }

                    str.Printf(reg->m_strName +
                            _("\n") +
                            reg->m_strDescription);
                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
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

                    m_gridRegisters->SetCellValue(m_gridRegisters->GetNumberRows() - 1,
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
            }

            if (!progressDlg.Pulse(_("Fill standard register information."))) return;

            // Fill grid with standard registers
            fillStandardRegisters();

            if (!progressDlg.Pulse(_("Fill status information."))) return;

            // Write status
            writeStatusInfo();

            if (!progressDlg.Pulse()) return;

            m_bFirstRead = false;
        }

        if (!progressDlg.Pulse(_("Update DM grid."))) return;

        // Update the DM grid    
        updateDmGrid();

        if (!progressDlg.Pulse(_("Update abstraction grid."))) return;

        // Update abstractions
        updateAbstractionGrid();

        if (!progressDlg.Pulse(_("Done."))) return;

        // Enable load defaults buttons
        m_ctrlButtonLoadMDF->Enable(true);

    }        // next read
    else {

        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel1Registers(nodeid);
        } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {
            writeChangedLevel2Registers();
        }
    }

    ::wxEndBusyCursor();

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonLoadDefaultsClick
//

void frmDeviceConfig::OnButtonLoadDefaultsClick(wxCommandEvent& event) {
    wxString strBuf;
    wxString str;
    uint8_t val;
    uint8_t nodeid = 0;
    uint32_t row = 0;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Fetching status and MDF"),
            256,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    // Get nickname
    nodeid = readStringValue(m_comboNodeID->GetValue());

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

            progressDlg.Pulse(_("Reading standard registers."));
            progress += progress_count;
            progressDlg.Update(progress);

            if (wxNOT_FOUND == reg->m_strDefault.Find(_("UNDEF"))) {

                if (false == progressDlg.Update(256 - m_mdf.m_list_register.GetCount() + cnt)) return;

                val = readStringValue(reg->m_strDefault);
                strBuf = getFormattedValue(val);
                m_gridRegisters->SelectRow(row);
                m_gridRegisters->SetCellValue(row, 2, strBuf);
                m_gridRegisters->MakeCellVisible(row, 2);
                m_gridRegisters->SetCellTextColour(row, 2, *wxRED);
                m_gridRegisters->Update();
            }

            // Make all parts of the row visible
            m_gridRegisters->AutoSizeRow(m_gridRegisters->GetNumberRows() - 1);

            row++;
        }

        if (false == progressDlg.Update(240)) return;

    }

    OnButtonUpdateClick(event);

    // Update the DM grid    
    updateDmGrid();

    if (false == progressDlg.Update(250)) return;

    // Update abstractions
    updateAbstractionGrid();


    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonWizardClick
//

void frmDeviceConfig::OnButtonWizardClick(wxCommandEvent& event) {
    wxMessageBox(_("The wizard is not implemented yet!"));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCellRightClick
//

void frmDeviceConfig::OnCellRightClick(wxGridEvent& event) {
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

    } else if (ID_GRID_ABSTRACTIONS == event.GetId()) {



    } else if (ID_GRID_DM == event.GetId()) {

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

void frmDeviceConfig::readValueSelectedRow(wxCommandEvent& WXUNUSED(event)) {
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());
    }

    if (m_gridRegisters->GetNumberRows()) {
        wxArrayInt selrows = m_gridRegisters->GetSelectedRows();

        if (selrows.GetCount()) {

            for (int i = selrows.GetCount() - 1; i >= 0; i--) {

                uint8_t val;

                uint16_t page;
                page = getPageFromCell(selrows[i]);

                uint32_t reg;
                reg = getRegFromCell(selrows[i]);

                if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

                    uint16_t savepage = m_csw.getRegisterPage(this, nodeid, &m_ifguid);

                    if (m_csw.setRegisterPage(nodeid, page) &&
                            m_csw.readLevel1Register(nodeid, reg, &val)) {

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->Update();

                        // Set original page
                        m_csw.setRegisterPage(nodeid, savepage);

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);
                    } else {
                        wxMessageBox(_("Failed to read value."));
                    }
                } else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

                    // Get the destination GUID
                    cguid destGUID;
                    destGUID.getFromString(m_comboNodeID->GetValue());

                    if (m_csw.readLevel2Register(m_ifguid,
                            reg,
                            &val,
                            &destGUID,
                            NULL,
                            m_bLevel2->GetValue())) {
                        // Update display
                        strBuf.Printf(_("0x%02lx"), val);
                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } else {
                        wxMessageBox(_("Failed to read value."));
                    }
                } // Interface
            }

        } else {
            wxMessageBox(_("No rows selected!"));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// writeValueSelectedRow
//

void frmDeviceConfig::writeValueSelectedRow(wxCommandEvent& WXUNUSED(event)) {
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;


    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());

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
                } else {
                    uint8_t val =
                            readStringValue(m_gridRegisters->GetCellValue(selrows[i], 2));

                    uint16_t page;
                    page = getPageFromCell(selrows[i]);

                    uint32_t reg;
                    reg = getRegFromCell(selrows[i]);

                    uint16_t savepage = m_csw.getRegisterPage(this,
                            (USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                            nodeid : destGUID.getLSB(),
                            &m_ifguid,
                            &destGUID,
                            m_bLevel2->GetValue());

                    // Set new page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                page,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
                        }
                    }

                    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written

                        m_csw.writeLevel1Register(nodeid, reg, &val);

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);

                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxBLUE);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.writeLevel2Register(m_ifguid,
                                reg,
                                &val,
                                destGUID,
                                NULL,
                                m_bLevel2->GetValue());
                        // Update display
                        strBuf.Printf(_("0x%02lx"), val);
                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);

                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    }

                    // Restore page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                savepage,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
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

void frmDeviceConfig::undoValueSelectedRow(wxCommandEvent& WXUNUSED(event)) {
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());
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
                } else {

                    uint8_t val;

                    uint8_t reg = getRegFromCell(i);
                    uint32_t page = getPageFromCell(i);


                    CMDF_Register *mdfRegs = m_mdf.getMDFRegs(reg, page);
                    if (NULL == mdfRegs) {
                        wxMessageBox(_("Register is unkown. Report possible iternal problem."));
                        continue;
                    }

                    val = mdfRegs->m_value;

                    uint16_t savepage = m_csw.getRegisterPage(this,
                            (USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                            nodeid : destGUID.getLSB(),
                            &m_ifguid,
                            &destGUID,
                            m_bLevel2->GetValue());

                    // Set new page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                page,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
                        }
                    }

                    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.writeLevel1Register(nodeid, reg, &val);

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxRED);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.writeLevel2Register(m_ifguid,
                                reg,
                                &val,
                                destGUID,
                                NULL,
                                m_bLevel2->GetValue());
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

                    // Restore page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                savepage,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
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
// defaultValueSelectedRow
//

void frmDeviceConfig::defaultValueSelectedRow(wxCommandEvent& WXUNUSED(event)) {
    wxString strBuf;
    uint8_t nodeid = 0;

    wxBusyCursor wait;

    // Get the destination GUID
    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    // Select the row
    m_gridRegisters->SelectRow(m_lastLeftClickRow);

    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {
        // Get Interface id
        nodeid = readStringValue(m_comboNodeID->GetValue());
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
                } else {

                    uint8_t val;

                    uint8_t reg = getRegFromCell(i);
                    uint32_t page = getPageFromCell(i);

                    CMDF_Register *mdfRegs = m_mdf.getMDFRegs(reg, page);
                    if (NULL == mdfRegs) {
                        wxMessageBox(_("Register is unkown. Report possible iternal problem."));
                        continue;
                    }

                    val = readStringValue(mdfRegs->m_strDefault);

                    uint16_t savepage = m_csw.getRegisterPage(this,
                            (USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                            nodeid : destGUID.getLSB(),
                            &m_ifguid,
                            &destGUID,
                            m_bLevel2->GetValue());

                    // Set new page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                page,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
                        }
                    }

                    if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.writeLevel1Register(nodeid, reg, &val);

                        // Update display
                        strBuf = getFormattedValue(val);

                        m_gridRegisters->SetCellValue(selrows[i], 2, strBuf);
                        m_gridRegisters->SelectRow(selrows[i]);
                        m_gridRegisters->MakeCellVisible(selrows[i], 2);
                        m_gridRegisters->SetCellTextColour(selrows[i], 2, *wxBLUE);
                        m_gridRegisters->Update();

                        updateDmGridConditional(reg, page);
                        updateAbstractionGridConditional(reg, page);

                    } else {

                        // We don't test for errors here as some registers have reserved bits
                        // etc and therefore will not read the same as written
                        m_csw.writeLevel2Register(m_ifguid,
                                reg,
                                &val,
                                destGUID,
                                NULL,
                                m_bLevel2->GetValue());

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

                    // Restore page
                    if (page != savepage) {
                        if (!m_csw.setRegisterPage((USE_DLL_INTERFACE == m_csw.getDeviceType()) ?
                                nodeid : destGUID.getLSB(),
                                savepage,
                                &m_ifguid,
                                &destGUID,
                                m_bLevel2->GetValue())) {
                            wxMessageBox(_("Failed to write/set new page."));
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
// OnLeftDClick
//

void frmDeviceConfig::OnLeftDClick(wxGridEvent& event) {
    uint8_t reg;

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    if (ID_GRID_REGISTERS == event.GetId()) {
        m_gridRegisters->SelectRow(event.GetRow());
    } else if (ID_GRID_ABSTRACTIONS == event.GetId()) {

        uint8_t pos;
        wxString strValue;
        wxString strBuf;
        uint8_t val = 0;

        DialogAbstractionEdit dlg(this);
        strValue = m_gridAbstractions->GetCellValue(event.GetRow(), 3);
        dlg.TransferDataToWindow(m_mdf.m_list_abstraction[ event.GetRow() ],
                strValue);

        m_gridAbstractions->SelectRow(event.GetRow());

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
                            readStringValue(m_comboNodeID->GetValue()),
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
                    } else if (wxNOT_FOUND != strValue.Find(_("FALSE"))) {
                        bval = false;
                    } else if (lv) {
                        bval = true;
                    } else {
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
                            readStringValue(m_comboNodeID->GetValue()),
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
                            readStringValue(m_comboNodeID->GetValue()),
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
                    val = readStringValue(strValue);

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
                            readStringValue(m_comboNodeID->GetValue()),
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
                    val = readStringValue(strValue);

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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {


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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {

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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {


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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {


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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {

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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {


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
                            readStringValue(m_comboNodeID->GetValue()),
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

                        } else {

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
                            readStringValue(m_comboNodeID->GetValue()),
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

        }

    } else if (ID_GRID_DM == event.GetId()) {

        wxString str;
        DialogEditLevelIDMrow dlg(this);

        m_gridDM->SelectRow(event.GetRow());

        // Originating address
        str = m_gridDM->GetCellValue(event.GetRow(), 0);
        dlg.m_oaddr->ChangeValue(str);

        // flags
        uint8_t flags = readStringValue(m_gridDM->GetCellValue(event.GetRow(), 1));
        (flags & 0x80) ? dlg.m_chkEnableDMRow->SetValue(true) : dlg.m_chkEnableDMRow->SetValue(false);
        (flags & 0x40) ? dlg.m_chkCheckOAddr->SetValue(true) : dlg.m_chkCheckOAddr->SetValue(false);
        (flags & 0x20) ? dlg.m_chkHardOAddr->SetValue(true) : dlg.m_chkHardOAddr->SetValue(false);
        (flags & 0x10) ? dlg.m_chkMatchZone->SetValue(true) : dlg.m_chkMatchZone->SetValue(false);
        (flags & 0x08) ? dlg.m_chkMatchSubzone->SetValue(true) : dlg.m_chkMatchSubzone->SetValue(false);

        // Class Mask
        reg = readStringValue(m_gridDM->GetCellValue(event.GetRow(), 2)) +
                ((flags & 0x02) << 9);
        str = wxString::Format(_("%d"), reg);
        dlg.m_classMask->ChangeValue(str);

        // Class Filter
        reg = readStringValue(m_gridDM->GetCellValue(event.GetRow(), 3)) +
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

        reg = readStringValue(m_gridDM->GetCellValue(event.GetRow(), 6));

        MDF_ACTION_LIST::iterator iter;
        for (iter = m_mdf.m_dmInfo.m_list_action.begin();
                iter != m_mdf.m_dmInfo.m_list_action.end(); ++iter) {
            CMDF_Action *action = *iter;
            int idx = dlg.m_comboAction->Append(action->m_strName);
            dlg.m_comboAction->SetClientData(idx, (void *)action->m_nCode); // Yes - ponter conversion
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
            strBuf.Printf(_("0x%02lx"), readStringValue(dlg.m_oaddr->GetValue()));
            m_gridRegisters->SetCellValue(m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // class mask
            uint16_t class_mask = readStringValue(dlg.m_classMask->GetValue());
            strBuf.Printf(_("0x%02lx"), class_mask & 0xff);
            m_gridRegisters->SetCellValue(2 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // class filter
            uint16_t class_filter = readStringValue(dlg.m_classFilter->GetValue());
            strBuf.Printf(_("0x%02lx"), class_filter & 0xff);
            m_gridRegisters->SetCellValue(3 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // type mask
            strBuf.Printf(_("0x%02lx"), readStringValue(dlg.m_typeMask->GetValue()));
            m_gridRegisters->SetCellValue(4 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // type filter
            strBuf.Printf(_("0x%02lx"), readStringValue(dlg.m_typeFilter->GetValue()));
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
            strBuf.Printf(_("0x%02lx"), flags);
            m_gridRegisters->SetCellValue(1 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // Action
            int idx = 0;
            if (wxNOT_FOUND != (idx = dlg.m_comboAction->GetSelection())) {
                uint32_t data = (uintptr_t) dlg.m_comboAction->GetClientData(idx);
                strBuf.Printf(_("0x%02lx"), data);
            } else {
                strBuf.Printf(_("0x%02lx"), 0);
            }
            m_gridRegisters->SetCellValue(6 + m_mdf.m_dmInfo.m_nStartOffset +
                    m_mdf.m_dmInfo.m_nRowSize * event.GetRow(),
                    2,
                    strBuf);

            // Action Parameter
            strBuf.Printf(_("0x%02lx"), readStringValue(dlg.m_actionParam->GetValue()));
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
                                readStringValue(dlg.m_oaddr->GetValue()));
                        m_gridRegisters->SetCellValue(
                                row + 1,
                                2,
                                strBuf);

                        m_gridRegisters->SetCellTextColour(
                                row + 1,
                                2, *wxRED);

                    } else {

                        for (int i = 0; i < m_mdf.m_dmInfo.m_nRowSize; i++) {

                            // Index
                            strBuf = getFormattedValue(readStringValue(
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
            } else {
                wxMessageBox(_("It's a problem with the MDF for this device. DM rows not in register definition."));
            }

            // Update registers
            OnButtonUpdateClick(event);

        }

    } else if (ID_GRID_ABSTRACTIONS == event.GetId()) {

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

void frmDeviceConfig::updateAbstractionGrid(void) {
    wxString strBuf;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Updating abstraction grid"),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    wxFont defaultFont = m_gridRegisters->GetDefaultCellFont();
    wxFont fontBold = defaultFont;
    fontBold.SetStyle(wxFONTSTYLE_NORMAL);
    fontBold.SetWeight(wxFONTWEIGHT_BOLD);

    progressDlg.Pulse();

    // Delete grid rows if there are any
    m_gridAbstractions->ClearGrid();
    if (m_gridAbstractions->GetNumberRows()) {
        m_gridAbstractions->DeleteRows(0, m_gridAbstractions->GetNumberRows());
    }

    progressDlg.Pulse();

    MDF_ABSTRACTION_LIST::iterator iter;
    for (iter = m_mdf.m_list_abstraction.begin();
            iter != m_mdf.m_list_abstraction.end(); ++iter) {

        CMDF_Abstraction *abstraction = *iter;

        // Add a row
        m_gridAbstractions->AppendRows(1);

        // Name
        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                0,
                _(" ") + abstraction->m_strName);
        m_gridAbstractions->SetCellAlignment(wxALIGN_LEFT,
                m_gridAbstractions->GetNumberRows() - 1,
                0);
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 0);


        // Type
        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                1,
                abstraction->m_strName);
        m_gridAbstractions->SetCellAlignment(wxALIGN_CENTER,
                m_gridAbstractions->GetNumberRows() - 1,
                1);
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 1);


        // Access
        strBuf = _("");
        if (abstraction->m_nAccess & MDF_ACCESS_READ) {
            strBuf = _("r");
        }
        if (abstraction->m_nAccess & MDF_ACCESS_WRITE) {
            strBuf += _("w");
        }

        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                2,
                strBuf);
        m_gridAbstractions->SetCellAlignment(wxALIGN_CENTER,
                m_gridAbstractions->GetNumberRows() - 1,
                2);
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 2);

        
        // Description
        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                4,
                _(" ") + abstraction->m_strDescription);

        m_gridAbstractions->SetCellAlignment(wxALIGN_LEFT,
                m_gridAbstractions->GetNumberRows() - 1,
                4);
        m_gridAbstractions->SetReadOnly(m_gridAbstractions->GetNumberRows() - 1, 4);
        m_gridRegisters->AutoSizeRow(m_gridAbstractions->GetNumberRows() - 1);

        wxString strValue;
        wxString strType;
        int pos = 0; // Current character

        progressDlg.Pulse();

        strType = abstraction->getAbstractionValueType();
        strValue = m_csw.getAbstractionValueAsString(this,
                m_stdRegisters.getNickname(),
                abstraction,
                &m_ifguid,
                &destGUID,
                &progressDlg,
                m_bLevel2->GetValue());

        progressDlg.Pulse();

        // Value
        m_gridAbstractions->SetCellValue(m_gridAbstractions->GetNumberRows() - 1,
                3,
                strValue);

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

void frmDeviceConfig::updateDmGrid(void) {
    wxString strBuf;
    uint8_t buf[ 512 ];
    uint8_t *prow = buf;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Updating DM grid"),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    m_gridDM->ClearGrid();

    for (int i = 0; i < m_mdf.m_dmInfo.m_nRowCount; i++) {

        progressDlg.Pulse();

        if (m_csw.getDMRow(this,
                m_stdRegisters.getNickname(),
                &m_mdf.m_dmInfo,
                i,
                prow,
                &m_ifguid,
                &destGUID,
                m_bLevel2->GetValue())) {

            progressDlg.Pulse();

            // Add a row
            if (!m_gridDM->AppendRows(1)) continue;

            // O-addr
            strBuf = getFormattedValue(prow[ 0 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 0, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 0);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 0);

            // Flags
            strBuf = getFormattedValue(prow[ 1 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 1, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 1);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 1);

            // Class Mask
            strBuf = getFormattedValue(prow[ 2 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 2, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 2);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 2);

            // Class Filter
            strBuf = getFormattedValue(prow[ 3 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 3, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 3);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 3);

            // Type Mask
            strBuf = getFormattedValue(prow[ 4 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 4, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 4);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 4);

            // Type Filter
            strBuf = getFormattedValue(prow[ 5 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 5, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 5);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 5);

            // Action
            strBuf = getFormattedValue(prow[ 6 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 6, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 6);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 6);

            // Action Parameter
            strBuf = getFormattedValue(prow[ 7 ]);
            m_gridDM->SetCellValue(m_gridDM->GetNumberRows() - 1, 7, strBuf);
            m_gridDM->SetCellAlignment(wxALIGN_CENTRE, m_gridDM->GetNumberRows() - 1, 7);
            m_gridDM->SetReadOnly(m_gridDM->GetNumberRows() - 1, 7);

            m_gridDM->Update();

            progressDlg.Pulse();
        } else {
            wxMessageBox(_("Unable to read decision matrix row!"));
            break;
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateDmGridConditional
//

void frmDeviceConfig::updateDmGridConditional(uint16_t reg, uint32_t page) {
    int row = getRegisterGridRow(reg, page);
    if (-1 == row) return;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Updating DM grid conditional"),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    if (m_mdf.m_dmInfo.m_bIndexed) {

        // Can't be first row
        if (0 == row) return;

        // Must write to data part for a change
        // to take place
        if ((reg = (m_mdf.m_dmInfo.m_nStartOffset + 1)) &&
                (page == m_mdf.m_dmInfo.m_nStartPage)) {

            progressDlg.Pulse();

            // Fetch index from grid
            uint8_t index = readStringValue(m_gridRegisters->GetCellValue(row - 1, 2));

            // Fetch value from grid
            uint8_t value = readStringValue(m_gridRegisters->GetCellValue(row, 2));
            wxString strValue = getFormattedValue(value);

            // Write it to the grid
            m_gridDM->SetCellValue(strValue, index / m_mdf.m_dmInfo.m_nRowSize, index % 8);

            progressDlg.Pulse();

        }

    } else {

        // Must write to data part for a change
        // to take place
        if ((reg >= (m_mdf.m_dmInfo.m_nStartOffset)) &&
                (reg < (m_mdf.m_dmInfo.m_nStartOffset + (m_mdf.m_dmInfo.m_nRowCount * m_mdf.m_dmInfo.m_nRowSize))) &&
                (page == m_mdf.m_dmInfo.m_nStartPage)) {

            progressDlg.Pulse();

            // Fetch value from grid
            uint8_t value = readStringValue(m_gridRegisters->GetCellValue(row, 2));
            wxString strValue = getFormattedValue(value);

            // Write it to the grid
            m_gridDM->SetCellValue(strValue,
                    (reg - m_mdf.m_dmInfo.m_nStartOffset) / m_mdf.m_dmInfo.m_nRowSize,
                    (reg - m_mdf.m_dmInfo.m_nStartOffset) % 8);

            progressDlg.Pulse();

        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updateAbstractionGridConditional
//

void frmDeviceConfig::updateAbstractionGridConditional(uint16_t reg, uint32_t page) {
    uint16_t rowAbstraction = 0;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Updating Abstraction grid conditional"),
            100,
            this,
            wxPD_ELAPSED_TIME |
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT);

    cguid destGUID;
    destGUID.getFromString(m_comboNodeID->GetValue());

    int row = getRegisterGridRow(reg, page);
    if (-1 == row) return;

    // Iterate through abstractions to se if this regitser is used
    MDF_ABSTRACTION_LIST::iterator iter;
    for (iter = m_mdf.m_list_abstraction.begin();
            iter != m_mdf.m_list_abstraction.end(); ++iter) {

        progressDlg.Pulse();

        CMDF_Abstraction *abstraction = *iter;
        bool bIndexed = abstraction->m_bIndexed;
        uint32_t regAbstraction = abstraction->m_nOffset;
        uint16_t pageAbstraction = abstraction->m_nPage;

        uint32_t width;

        if (abstraction->m_nType == type_string) {
            width = abstraction->m_nWidth;
        } else if (abstraction->m_nType == type_bitfield) {
            width = abstraction->m_nWidth;
        } else if (abstraction->m_nType == type_boolval) {
            width = 1;
        } else if (abstraction->m_nType == type_int8_t) {
            width = 1;
        } else if (abstraction->m_nType == type_uint8_t) {
            width = 1;
        } else if (abstraction->m_nType == type_int16_t) {
            width = 2;
        } else if (abstraction->m_nType == type_uint16_t) {
            width = 2;
        } else if (abstraction->m_nType == type_int32_t) {
            width = 4;
        } else if (abstraction->m_nType == type_uint32_t) {
            width = 4;
        } else if (abstraction->m_nType == type_int64_t) {
            width = 8;
        } else if (abstraction->m_nType == type_uint64_t) {
            width = 8;
        } else if (abstraction->m_nType == type_double) {
            width = 8;
        } else if (abstraction->m_nType == type_float) {
            width = 4;
        } else if (abstraction->m_nType == type_date) {
            width = 4;
        } else if (abstraction->m_nType == type_time) {
            width = 3;
        } else if (abstraction->m_nType == type_guid) {
            width = 16;
        } else {
            width = 0;
        }

        if (bIndexed) {

            progressDlg.Pulse();

            if ((reg == regAbstraction + 1) &&
                    (page == pageAbstraction)) {

                progressDlg.Pulse();

                // Register is datapart of an abstraction (the second byte)
                // and the abstraction should be updated.

                // Fetch index from grid
                uint8_t index = readStringValue(m_gridRegisters->GetCellValue(row - 1, 2));

                progressDlg.Pulse();

                // Fetch value from grid
                //uint8_t value = readStringValue( m_gridRegisters->GetCellValue( row, 2 ) );
                //wxString strValue = getFormattedValue( value );
                wxString strValue;
                strValue = m_csw.getAbstractionValueAsString(this,
                        m_stdRegisters.getNickname(),
                        abstraction,
                        &m_ifguid,
                        &destGUID,
                        &progressDlg,
                        m_bLevel2->GetValue());

                progressDlg.Pulse();

                // Write it to the grid
                m_gridAbstractions->SetCellValue(strValue,
                        rowAbstraction,
                        3);
            }

        } else {

            if ((reg >= regAbstraction) &&
                    (reg < (regAbstraction + width)) &&
                    (page == pageAbstraction)) {

                progressDlg.Pulse();

                // Fetch value from grid
                //uint8_t value = readStringValue( m_gridRegisters->GetCellValue( row, 2 ) );
                //wxString strValue = getFormattedValue( value );
                wxString strValue;
                strValue = m_csw.getAbstractionValueAsString(this,
                        m_stdRegisters.getNickname(),
                        abstraction,
                        &m_ifguid,
                        &destGUID,
                        &progressDlg,
                        m_bLevel2->GetValue());
                // Write it to the grid
                m_gridAbstractions->SetCellValue(strValue,
                        rowAbstraction,
                        3);

                progressDlg.Pulse();

            }

        }

        rowAbstraction++;

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dmEnableSelectedRow
//

void frmDeviceConfig::dmEnableSelectedRow(wxCommandEvent& event) {
    wxString strBuf;

    // Select the row
    m_gridDM->SelectRow(m_lastLeftClickRow);

    wxArrayInt selrows = m_gridDM->GetSelectedRows();

    if (selrows.GetCount()) {

        for (int i = selrows.GetCount() - 1; i >= 0; i--) {

            uint8_t flags = readStringValue(m_gridDM->GetCellValue(selrows[i], 2));
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

void frmDeviceConfig::dmDisableSelectedRow(wxCommandEvent& event) {
    wxString strBuf;

    // Select the row
    m_gridDM->SelectRow(m_lastLeftClickRow);

    wxArrayInt selrows = m_gridDM->GetSelectedRows();

    if (selrows.GetCount()) {

        for (int i = selrows.GetCount() - 1; i >= 0; i--) {

            uint8_t flags = readStringValue(m_gridDM->GetCellValue(selrows[i], 2));
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

void frmDeviceConfig::dmRowWizard(wxCommandEvent& event) {
    wxMessageBox(_("Still needs to be coded..."));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SAVE_REGSITERS
//

void frmDeviceConfig::OnMenuitemSaveRegistersClick(wxCommandEvent& event) {

    wxString str;
    wxStandardPaths stdpaths;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save register set to "),
            stdpaths.GetUserDataDir(),
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

void frmDeviceConfig::OnMenuitemSaveSelectedRegistersClick(wxCommandEvent& event) {

    wxString str;
    wxStandardPaths stdpaths;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save register set to "),
            stdpaths.GetUserDataDir(),
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

void frmDeviceConfig::OnMenuitemLoadRegistersClick(wxCommandEvent& event) {
    wxStandardPaths stdpaths;
    wxXmlDocument doc;

    // First find a path to save register set data to
    wxFileDialog dlg(this,
            _("Choose file to load register set from "),
            stdpaths.GetUserDataDir(),
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

        // start processing the XML file
        if (doc.GetRoot()->GetName() != wxT("registerset")) {
            return;
        }

        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            long reg;
            uint8_t value;
            if (child->GetName() == wxT("register")) {

                wxString id =
                        child->GetPropVal(wxT("id"), wxT("-1"));
                reg = readStringValue(id);
                if (-1 == reg) continue;

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {
                    if (subchild->GetName() == wxT("value")) {
                        wxString strSaveValue = m_gridRegisters->GetCellValue(reg, 2);
                        wxString str = subchild->GetNodeContent();
                        value = readStringValue(str);
                        str.Printf(_("0x%02lx"), value);
                        if (str != strSaveValue) {
                            m_gridRegisters->SetCellValue(reg, 2, str);
                            m_gridRegisters->SetCellTextColour(reg, 2, *wxRED);
                        }
                    }
                    subchild = subchild->GetNext();
                }

            }

            child = child->GetNext();

        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getRegFromCell
//

uint32_t frmDeviceConfig::getRegFromCell(int row) {
    wxString token1, token2;
    wxString str = m_gridRegisters->GetCellValue(row, 0);

    wxStringTokenizer tkz(str, _(":"));
    if (tkz.HasMoreTokens()) {
        token1 = tkz.GetNextToken(); // Page
    } else {
        token1 = str;
    }

    // For a standard register this one will fail
    if (tkz.HasMoreTokens()) {
        token2 = tkz.GetNextToken(); // Register
    } else {
        token2 = token1;
    }

    return readStringValue(_("0x") + token2.Trim());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getPageFromCell
//

uint16_t frmDeviceConfig::getPageFromCell(int row) {
    wxString str = m_gridRegisters->GetCellValue(row, 0);
    if (wxNOT_FOUND != str.Find(_(":"))) {
        return readStringValue(_("0x") + m_gridRegisters->GetCellValue(row, 0).Trim());
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getFormattedValue
//

wxString frmDeviceConfig::getFormattedValue(uint8_t val) {
    if (VSCP_DEVCONFIG_NUMBERBASE_HEX ==
            g_Config.m_Numberbase) {
        return wxString::Format(_("0x%02X"), val);
    } else {
        return wxString::Format(_("0x%02lx"), val);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getRegisterGridRow
//

int frmDeviceConfig::getRegisterGridRow(uint32_t reg, uint16_t page) {
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

bool frmDeviceConfig::fetchIterfaceGUID(void) {
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

                    // Save the name
                    //m_vscpif.m_strInterfaceName. = strName;

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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemAddGuidsClick
//

void frmDeviceConfig::OnMenuitemAddGuidsClick(wxCommandEvent& event) {
    wxString str = ::wxGetTextFromUser(_("Enter full GUID to add :"),
            _("VSCP Works"));
    m_guidarray.Add(str);
    m_comboNodeID->Append(str);

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveGuidsClick
//

void frmDeviceConfig::OnMenuitemSaveGuidsClick(wxCommandEvent& event) {
    wxString str;
    wxStandardPaths stdpaths;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to GUID's to"),
            stdpaths.GetUserDataDir(),
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

void frmDeviceConfig::OnMenuitemLoadGuidsClick(wxCommandEvent& event) {
    wxStandardPaths stdpaths;
    wxXmlDocument doc;

    // First find a path to save register set data to
    wxFileDialog dlg(this,
            _("Choose file to load register set from "),
            stdpaths.GetUserDataDir(),
            _("guidset"),
            _("GUID set Files (*.guid)|*.guid|All files (*.*)|*.*"));

    if (wxID_OK == dlg.ShowModal()) {

        m_comboNodeID->Clear();

        wxFFileInputStream *pFileStream = new wxFFileInputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to open GUID set file."));
            return;
        }

        char buf[ 2048 ];
        wxString str;

        while (!pFileStream->Eof()) {

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

void frmDeviceConfig::OnHtmlwindowCellClicked(wxHtmlLinkEvent& event) {
    if (event.GetLinkInfo().GetHref().StartsWith(_("http://"))) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}

