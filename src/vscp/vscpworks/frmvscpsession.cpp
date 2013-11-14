/////////////////////////////////////////////////////////////////////////////
//  Name:        frmvscpsession.cpp
//  Purpose:     
//  Author:      Ake Hedman
//  Modified by: 
//  Created:     Sat 30 Jun 2007 14:08:14 CEST
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
#pragma implementation "frmvscpsession.h"
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
////@end includes

#include <wx/app.h>
#include <wx/listimpl.cpp>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/dynlib.h>
#include <wx/progdlg.h>
#include "dlgconfiguration.h"

#include "vscpworks.h"
#include "vscptxobj.h"
#include "dlgvscptrmit.h"
#include "dlgabout.h"
#include "frmvscpsession.h"
#include "../common/canal.h"
#include "../common/vscp.h"
#include "../common/vscphelper.h"

extern appConfiguration g_Config; // Configuration data

extern VSCPInformation g_vscpinfo; // VSCP class type information

WX_DEFINE_LIST(eventOutQueue);
WX_DEFINE_LIST(TXLIST);

DEFINE_EVENT_TYPE(wxVSCP_IN_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_OUT_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_CTRL_LOST_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_PREP_CONNECT_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_CONNECTED_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_LOST_EVENT)

extern appConfiguration g_Config;



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

/* XPM */
static char * delete_xpm[] = {
"16 16 107 2",
"    c None",
".   c #1B0202",
"+   c #51758F",
"@   c #5A89A6",
"#   c #6B98B8",
"$   c #6591AE",
"%   c #538DB3",
"&   c #4581AA",
"*   c #3A749C",
"=   c #8EA9BC",
"-   c #D6DFE7",
";   c #2B232B",
">   c #C3DDF1",
",   c #B6D5EE",
"'   c #A5CCEA",
")   c #97C4E7",
"!   c #85BBE2",
"~   c #5F9BC8",
"{   c #72A8D2",
"]   c #F1F4F7",
"^   c #47667C",
"/   c #3B2A2D",
"(   c #888B94",
"_   c #67A1CF",
":   c #FFFFFF",
"<   c #A69FA1",
"[   c #64626A",
"}   c #4F444A",
"|   c #BED7EA",
"1   c #7EA6C0",
"2   c #8AAFCE",
"3   c #2D232A",
"4   c #E2EFF8",
"5   c #2E2229",
"6   c #53687E",
"7   c #7EAED3",
"8   c #57768D",
"9   c #5D809A",
"0   c #383B47",
"a   c #3D5C74",
"b   c #C0BCBE",
"c   c #605051",
"d   c #9498A1",
"e   c #D1E5F5",
"f   c #A1B9CE",
"g   c #6C788A",
"h   c #4E5261",
"i   c #434C5C",
"j   c #515F73",
"k   c #3E3E4A",
"l   c #6C95B4",
"m   c #6689A5",
"n   c #6A8CAA",
"o   c #3C3B47",
"p   c #577189",
"q   c #8BB1D0",
"r   c #F4F9FD",
"s   c #72676A",
"t   c #463537",
"u   c #443538",
"v   c #3D2C30",
"w   c #72747E",
"x   c #8FA0B3",
"y   c #27161A",
"z   c #31272F",
"A   c #668CA8",
"B   c #81B5DB",
"C   c #FAFCFE",
"D   c #DCDCDE",
"E   c #726465",
"F   c #9F9B9F",
"G   c #433032",
"H   c #8D8D91",
"I   c #B6C3CE",
"J   c #7AA8CC",
"K   c #3B3A46",
"L   c #7CAED2",
"M   c #4A5466",
"N   c #414453",
"O   c #5B768F",
"P   c #749BB4",
"Q   c #635151",
"R   c #5F4E4F",
"S   c #DFE1E3",
"T   c #F7FBFD",
"U   c #B5B3B8",
"V   c #594A4C",
"W   c #594C4F",
"X   c #9FA4AD",
"Y   c #8AA4B6",
"Z   c #89868C",
"`   c #332D36",
" .  c #77A3C5",
"..  c #5E768F",
"+.  c #5D5053",
"@.  c #595E6D",
"#.  c #616B77",
"$.  c #594C4E",
"%.  c #FDFDFE",
"&.  c #D3D5DA",
"*.  c #868083",
"=.  c #362D32",
"-.  c #1D0303",
";.  c #463F46",
">.  c #717F8D",
",.  c #7995A8",
"'.  c #768D9D",
"  . + @ # $ $ % & * = -         ",
"  . ; > > , ' ) ! ~ { ~ ]     . ",
"    ^ / ( > , ' ) _ : ) # <     ",
"    = [ } | > , ' 1 : : 2 3     ",
"    = 4 5 6 7 ! ! _ 8 9 0 a     ",
"    = 4 b c d e > f g h i %     ",
"    = 4 ! j k l m n o p q %     ",
"    = 4 r r s t u v w x ' $     ",
"    = 4 ! ! 7 y z A B ! , $     ",
"    = 4 C D E F G H I e > #     ",
"    = 4 J 5 K L M N O B e P     ",
"    = 4 Q R S T r U V W X P     ",
"    Y Z ` L ! ! ! !  ...+.@.    ",
"    #.$.: : : %.C T r &.*.=.-.  ",
"    ;.>.= = = = = = = Y ,.'.  -.",
"                                "};

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
static char *redo_xpm[] = {
/* width height num_colors chars_per_pixel */
"    16    15        3            1",
/* colors */
". c #000080",
"# c None",
"a c #808080",
/* pixels */
"################",
"################",
"################",
"################",
"###a....########",
"##a.####..###.##",
"##.#######.#..##",
"##.########...##",
"##.#######....##",
"##a.#####.....##",
"###.a###########",
"################",
"################",
"################",
"################"
};

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
// frmVSCPSession type definition
//

IMPLEMENT_CLASS(frmVSCPSession, wxFrame)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession event table definition
//

BEGIN_EVENT_TABLE(frmVSCPSession, wxFrame)

////@begin frmVSCPSession event table entries
  EVT_CLOSE( frmVSCPSession::OnCloseWindow )
  EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_ACTIVATE, frmVSCPSession::OnInterfaceActivate )
  EVT_MENU( ID_MENUITEM_VSCP_LOAD_MSG_LIST, frmVSCPSession::LoadRXEventList )
  EVT_MENU( ID_MENUITEM_VSCP_SAVE_MSG_LIST, frmVSCPSession::SaveRXEventList )
  EVT_MENU( ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET, frmVSCPSession::OnTxLoadClick )
  EVT_MENU( ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET, frmVSCPSession::OnTxSaveClick )
  EVT_MENU( ID_MENUITEM_VSCP_SESSION_EXIT, frmVSCPSession::OnMenuitemVscpSessionExitClick )
  EVT_MENU( ID_MENUITEM4, frmVSCPSession::ClearRxList )
  EVT_MENU( ID_MENUITEM5, frmVSCPSession::ClearTxList )
  EVT_MENU( ID_MENUITEM_VSCP_LOG, frmVSCPSession::OnMenuitemVscpViewLogClick )
  EVT_MENU( ID_MENUITEM_VSCP_COUNT, frmVSCPSession::OnMenuitemVscpViewCountClick )
  EVT_MENU( ID_MENUITEM_READ_REGISTER, frmVSCPSession::OnMenuitemSetAutoreplyClick )
  EVT_MENU( ID_MENUITEM_WRITE_REGISTER, frmVSCPSession::OnMenuitemSetBurstCountClick )
  EVT_MENU( ID_MENUITEM_READ_ALL_REGISTERS, frmVSCPSession::OnMenuitemSetFilterClick )
  EVT_MENU( ID_MENUITEM_GET_GUID, frmVSCPSession::OnMenuitemSetAutoreplyClick )
  EVT_MENU( ID_MENUITEM_GET_MDF_URL, frmVSCPSession::OnMenuitemSetAutoreplyClick )
  EVT_MENU( ID_MENUITEM_GET_MDF, frmVSCPSession::OnMenuitemSetAutoreplyClick )
  EVT_MENU( ID_MENUITEM_RX_GRID_WIDTH, frmVSCPSession::OnMenuitemSaveRxCellWidth )
  EVT_MENU( ID_MENUITEM_SAVE_TX_GRID_WIDTH, frmVSCPSession::OnMenuitemSaveTxCellWidth )
  EVT_MENU( ID_MENUITEM1, frmVSCPSession::OnMenuitemSetAutoreplyClick )
  EVT_MENU( ID_MENUITEM2, frmVSCPSession::OnMenuitemSetBurstCountClick )
  EVT_MENU( ID_MENUITEM3, frmVSCPSession::OnMenuitemSetFilterClick )
  EVT_MENU( ID_MENUITEM, frmVSCPSession::OnMenuitemConfigurationClick )
  EVT_MENU( ID_MENUITEM_VSCP_HELP, frmVSCPSession::OnMenuitemVscpHelpClick )
  EVT_MENU( ID_MENUITEM_VSCP_FAQ, frmVSCPSession::OnMenuitemVscpFaqClick )
  EVT_MENU( ID_MENUITEM_VSCP_SHORTCUTS, frmVSCPSession::OnMenuitemVscpShortcutsClick )
  EVT_MENU( ID_MENUITEM_VSCP_THANKS, frmVSCPSession::OnMenuitemVscpThanksClick )
  EVT_MENU( ID_MENUITEM_VSCP_CREDITS, frmVSCPSession::OnMenuitemVscpCreditsClick )
  EVT_MENU( ID_MENUITEM_VSCP_VSCP_SITE, frmVSCPSession::OnMenuitemVscpVscpSiteClick )
  EVT_MENU( ID_MENUITEM_VSCP_ABOUT, frmVSCPSession::OnMenuitemVscpAboutClick )
  EVT_GRID_CELL_LEFT_CLICK( frmVSCPSession::OnCellReceiveLeftClick )
  EVT_GRID_CELL_RIGHT_CLICK( frmVSCPSession::OnGridCellReceiveRightClick )
  EVT_GRID_LABEL_LEFT_DCLICK( frmVSCPSession::OnGridLabelLeftDClick )
  EVT_GRID_SELECT_CELL( frmVSCPSession::OnSelectCell )
  EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW_RCVINFO, frmVSCPSession::OnHtmlwindowRcvinfoLinkClicked )
  EVT_BUTTON( ID_BITMAPBUTTON_TX_ADD, frmVSCPSession::OnTxAddClick )
  EVT_BUTTON( ID_BITMAPBUTTON_TX_EDIT, frmVSCPSession::OnTxEditClick )
  EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_DELETE, frmVSCPSession::OnTxDeleteClick )
  EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_LOAD, frmVSCPSession::OnTxLoadClick )
  EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_SAVE, frmVSCPSession::OnTxSaveClick )
  EVT_GRID_CELL_LEFT_CLICK( frmVSCPSession::OnCellTxLeftClick )
  EVT_GRID_CELL_RIGHT_CLICK( frmVSCPSession::OnCellTxRightClick )
  EVT_GRID_CELL_LEFT_DCLICK( frmVSCPSession::OnGridLeftDClick )
  EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND, frmVSCPSession::OnTxSendClick )
////@end frmVSCPSession event table entries

EVT_MENU(Menu_Popup_TX_Transmit, frmVSCPSession::OnTxSendClick)
EVT_MENU(Menu_Popup_TX_Add, frmVSCPSession::OnTxAddClick)
EVT_MENU(Menu_Popup_TX_Edit, frmVSCPSession::OnTxEditClick)
EVT_MENU(Menu_Popup_TX_Delete, frmVSCPSession::OnTxDeleteClick)
EVT_MENU(Menu_Popup_TX_Clone, frmVSCPSession::OnTxCloneRow)

EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_IN_EVENT, frmVSCPSession::eventReceive)
EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_OUT_EVENT, frmVSCPSession::eventTransmit)
EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_CTRL_LOST_EVENT, frmVSCPSession::eventLostCtrlIf)
EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_PREP_CONNECT_EVENT, frmVSCPSession::eventPrepareConnect)
EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_CONNECTED_EVENT, frmVSCPSession::eventConnected)
EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_LOST_EVENT, frmVSCPSession::eventLostRcvIf)


END_EVENT_TABLE()



// ----------------------------------------------------------------------------
// RXGridCellAttrProvider
// ----------------------------------------------------------------------------

RXGridCellAttrProvider::RXGridCellAttrProvider()
{
    // * * * Even rows * * *
    m_attrForEvenRows = new wxGridCellAttr;
    m_attrForEvenRows->SetBackgroundColour(*wxWHITE);

    m_attrEvenColDirection = new wxGridCellAttr;
    m_attrEvenColDirection->SetAlignment(wxALIGN_CENTRE, wxALIGN_TOP);
    m_attrEvenColDirection->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColDirection->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColDirection->SetFont();

    m_attrEvenColClass = new wxGridCellAttr;
    m_attrEvenColClass->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColClass->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColClass->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColClass->SetFont();

    m_attrEvenColType = new wxGridCellAttr;
    m_attrEvenColType->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColType->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColType->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColType->SetFont();

    m_attrEvenColNote = new wxGridCellAttr;
    m_attrEvenColNote->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColNote->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColNote->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColNote->SetFont();  


    // * * * Odd rows * * * 

    m_attrForOddRows = new wxGridCellAttr;
    m_attrForOddRows->SetBackgroundColour(*wxLIGHT_GREY);

    m_attrOddColDirection = new wxGridCellAttr;
    m_attrOddColDirection->SetAlignment(wxALIGN_CENTRE, wxALIGN_TOP);
    m_attrOddColDirection->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColDirection->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColDirection->SetFont();

    m_attrOddColClass = new wxGridCellAttr;
    m_attrOddColClass->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColClass->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColClass->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColClass->SetFont();

    m_attrOddColType = new wxGridCellAttr;
    m_attrOddColType->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColType->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColType->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColType->SetFont();

    m_attrOddColNote = new wxGridCellAttr;
    m_attrOddColNote->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColNote->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColNote->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColNote->SetFont();
}

RXGridCellAttrProvider::~RXGridCellAttrProvider()
{
    m_attrForOddRows->DecRef();

    m_attrEvenColDirection->DecRef();
    m_attrEvenColClass->DecRef();
    m_attrEvenColType->DecRef();
    m_attrEvenColNote->DecRef();

    m_attrOddColDirection->DecRef();
    m_attrOddColClass->DecRef();
    m_attrOddColType->DecRef();
    m_attrOddColNote->DecRef();
}

wxGridCellAttr *RXGridCellAttrProvider::GetAttr(int row, int col,
        wxGridCellAttr::wxAttrKind kind /* = wxGridCellAttr::Any */) const
{
    wxGridCellAttr *attr = wxGridCellAttrProvider::GetAttr(row, col, kind);

    if (g_Config.m_VscpRcvFrameRxbPyamas && (row % 2)) {

        // Odd rows    
        if (!attr) {

            switch (col) {

            case 0:
                attr = m_attrOddColDirection;
                attr->IncRef();
                break;

            case 1:
                attr = m_attrOddColClass;
                attr->IncRef();
                break;

            case 2:
                attr = m_attrOddColType;
                attr->IncRef();
                break;

            case 3:
                attr = m_attrOddColNote;
                attr->IncRef();
                break;

            default:
                attr = m_attrForOddRows;
                attr->IncRef();
                break;

            }

        } else {
            if (!attr->HasBackgroundColour()) {
                wxGridCellAttr *attrNew = attr->Clone();
                attr->DecRef();
                attr = attrNew;
                attr->SetBackgroundColour(*wxLIGHT_GREY);
            }
        }
    } else {
        if (!attr) {

            switch (col) {

            case 0:
                attr = m_attrEvenColDirection;
                attr->IncRef();
                break;

            case 1:
                attr = m_attrEvenColClass;
                attr->IncRef();
                break;

            case 2:
                attr = m_attrEvenColType;
                attr->IncRef();
                break;

            case 3:
                attr = m_attrEvenColNote;
                attr->IncRef();
                break;

            default:
                attr = m_attrForEvenRows;
                attr->IncRef();
                break;

            }
        }
    }

    return attr;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//

ctrlObj::ctrlObj()
{
    m_bQuit = false; // Dont even think of quiting yet...
    m_errorControl = 0; // No error
    m_errorReceive = 0;
    m_pgridTable = NULL; // No valid grid yet
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//

ctrlObj::~ctrlObj()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession constructors
//

frmVSCPSession::frmVSCPSession()
{
    Init();
}

frmVSCPSession::frmVSCPSession(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession creator
//

bool frmVSCPSession::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    ////@begin frmVSCPSession creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
    ////@end frmVSCPSession creation

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession destructor
//

frmVSCPSession::~frmVSCPSession()
{
    ////@begin frmVSCPSession destruction
    ////@end frmVSCPSession destruction

    // Remove possible items in TXLIST
    TXLIST::iterator iter;
    for (iter = m_CtrlObject.m_txList.begin(); iter != m_CtrlObject.m_txList.end(); ++iter) {
        VscpTXObj *obj = *iter;
        if (NULL != obj) {
            if (NULL != obj->m_Event.pdata) {
                delete obj->m_Event.pdata;
            }
            delete obj;
        }
    }

    m_CtrlObject.m_txList.Clear();

    // Stop the worker threads
    stopWorkerThreads();

    m_pgridTable->clearEventList();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmVSCPSession::Init()
{
    ////@begin frmVSCPSession member initialisation
  m_BtnActivateInterface = NULL;
  m_pPanel = NULL;
  m_ctrlGridReceive = NULL;
  m_ctrlRcvHtmlInfo = NULL;
  m_btnAdd = NULL;
  m_btnEdit = NULL;
  m_btnDelete = NULL;
  m_btnLoadSet = NULL;
  m_btnSaveSet = NULL;
  m_ctrlGridTransmission = NULL;
  m_btnSend = NULL;
  m_btnActivate = NULL;
  m_btnClear = NULL;
    ////@end frmVSCPSession member initialisation

    m_CtrlObject.m_bQuit = false;
    m_CtrlObject.m_interfaceType = INTERFACE_VSCP;
    memset(m_CtrlObject.m_GUID, 0, 16);

    m_pTXWorkerThread = NULL;
    m_pRXWorkerThread = NULL;
    m_pDeviceWorkerThread = NULL;

    m_pProgressDlg = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_CLOSE_WINDOW event handler for ID_FRMVSCPSESSION
//

void frmVSCPSession::OnCloseWindow(wxCloseEvent& event)
{
    wxBusyCursor wait;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Clearing VSCP events..."),
            100,
            this,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

    // Cleanup
    progressDlg.Update(30);
    stopWorkerThreads();

    progressDlg.Update(85);
    m_pgridTable->clearEventList();

    progressDlg.Update(100);
    progressDlg.Pulse(_("Done!"));

    // Hide window
    Show(false);

    // Destroy the window
    Destroy();

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// startWorkerThreads
//

void frmVSCPSession::startWorkerThreads(frmVSCPSession *pFrm)
{
    wxBusyCursor wait;

    if (INTERFACE_VSCP == m_CtrlObject.m_interfaceType) {

        /////////////////////////////////////////////////////////////////////////////
        // Load controlobject control handler
        /////////////////////////////////////////////////////////////////////////////
        m_pTXWorkerThread = new TXWorkerThread;

        if (NULL != m_pTXWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *) pFrm;
            m_pTXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pTXWorkerThread->Create())) {
                m_pTXWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pTXWorkerThread->Run())) {
                    ::wxGetApp().logMsg(_("Unable to run controlobject device thread."),
                            DAEMON_LOGMSG_CRITICAL);
                }
            } else {
                ::wxGetApp().logMsg(_("Unable to create controlobject device thread."),
                        DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            ::wxGetApp().logMsg(_("Unable to allocate memory for controlobject device thread."), DAEMON_LOGMSG_CRITICAL);
        }


        /////////////////////////////////////////////////////////////////////////////
        // Load controlobject client message handler
        /////////////////////////////////////////////////////////////////////////////
        m_pRXWorkerThread = new RXWorkerThread;

        if (NULL != m_pRXWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *) pFrm;
            m_pRXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pRXWorkerThread->Create())) {
                m_pRXWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pRXWorkerThread->Run())) {
                    ::wxGetApp().logMsg(_("Unable to run controlobject client thread."),
                            DAEMON_LOGMSG_CRITICAL);
                }
            } else {
                ::wxGetApp().logMsg(_("Unable to create controlobject client thread."),
                        DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            ::wxGetApp().logMsg(_("Unable to allocate memory for controlobject client thread."), DAEMON_LOGMSG_CRITICAL);
        }
    } else {
        /////////////////////////////////////////////////////////////////////////////
        // Load device handler
        /////////////////////////////////////////////////////////////////////////////
        m_pDeviceWorkerThread = new deviceThread;

        if (NULL != m_pDeviceWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *) pFrm;
            m_pDeviceWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pDeviceWorkerThread->Create())) {
                m_pDeviceWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pDeviceWorkerThread->Run())) {
                    ::wxGetApp().logMsg(_("Unable to run controlobject device thread."), DAEMON_LOGMSG_CRITICAL);
                }
            } else {
                ::wxGetApp().logMsg(_("Unable to create controlobject device thread."), DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            ::wxGetApp().logMsg(_("Unable to allocate memory for controlobject device thread."), DAEMON_LOGMSG_CRITICAL);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stopWorkerThreads
//

void frmVSCPSession::stopWorkerThreads(void)
{
    wxBusyCursor wait;

    m_BtnActivateInterface->SetValue(false);
    m_BtnActivateInterface->SetLabel(_("Disconnected"));
    m_BtnActivateInterface->Update();

    m_CtrlObject.m_bQuit = true;

    if (INTERFACE_VSCP == m_CtrlObject.m_interfaceType) {

        if (NULL != m_pTXWorkerThread) {
            m_pTXWorkerThread->Wait();
            delete m_pTXWorkerThread;
            m_pTXWorkerThread = NULL;
        }

        if (NULL != m_pRXWorkerThread) {
            m_pRXWorkerThread->Wait();
            delete m_pRXWorkerThread;
            m_pRXWorkerThread = NULL;
        }

    } else {
        if (NULL != m_pDeviceWorkerThread) {
            m_pDeviceWorkerThread->Wait();
            delete m_pDeviceWorkerThread;
            m_pDeviceWorkerThread = NULL;
        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOGGLEBUTTON_ACTIVATE
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET

void frmVSCPSession::OnInterfaceActivate(wxCommandEvent& event)
{
    wxBusyCursor wait;

    if (!m_BtnActivateInterface->GetValue()) {
        m_CtrlObject.m_bQuit = true;
        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
    } else {
        m_CtrlObject.m_bQuit = false;
        startWorkerThreads(this); // Start worker threads
        m_BtnActivateInterface->SetValue(true);
        m_BtnActivateInterface->SetLabel(_("Connected"));
    }

    event.Skip();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for frmVSCPSession
//

void frmVSCPSession::CreateControls()
{


    ////@begin frmVSCPSession content construction
  frmVSCPSession* itemFrame1 = this;

  wxMenuBar* menuBar = new wxMenuBar;
  wxMenu* itemMenu16 = new wxMenu;
  itemMenu16->Append(ID_MENUITEM_VSCP_LOAD_MSG_LIST, _("Load VSCP events from file..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu16->Append(ID_MENUITEM_VSCP_SAVE_MSG_LIST, _("Save VSCP events to file..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu16->AppendSeparator();
  itemMenu16->Append(ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET, _("Load transmission set from file..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu16->Append(ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET, _("Save transmission set to file..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu16->AppendSeparator();
  itemMenu16->Append(ID_MENUITEM_VSCP_SESSION_EXIT, _("Exit"), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu16, _("File"));
  wxMenu* itemMenu24 = new wxMenu;
  itemMenu24->Append(ID_MENUITEM_VSCP_CUT, _("Cut"), wxEmptyString, wxITEM_NORMAL);
  itemMenu24->Append(ID_MENUITEM_VSCP_COPY, _("Copy"), wxEmptyString, wxITEM_NORMAL);
  itemMenu24->Append(ID_MENUITEM_VSCP_PASTE, _("Paste"), wxEmptyString, wxITEM_NORMAL);
  itemMenu24->AppendSeparator();
  itemMenu24->Append(ID_MENUITEM4, _("Clear receive list"), wxEmptyString, wxITEM_NORMAL);
  itemMenu24->Append(ID_MENUITEM5, _("Clear transmission list"), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu24, _("Edit"));
  wxMenu* itemMenu31 = new wxMenu;
  itemMenu31->Append(ID_MENUITEM_VSCP_LOG, _("Message Log"), wxEmptyString, wxITEM_RADIO);
  itemMenu31->Check(ID_MENUITEM_VSCP_LOG, true);
  itemMenu31->Append(ID_MENUITEM_VSCP_COUNT, _("Message Count"), wxEmptyString, wxITEM_RADIO);
  itemMenu31->AppendSeparator();
  menuBar->Append(itemMenu31, _("View"));
  wxMenu* itemMenu35 = new wxMenu;
  itemMenu35->Append(ID_MENUITEM_READ_REGISTER, _("Read Regiister..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_WRITE_REGISTER, _("Write Register..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_READ_ALL_REGISTERS, _("Read all registers..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu35->AppendSeparator();
  itemMenu35->Append(ID_MENUITEM_GET_GUID, _("Get GUID for node..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_GET_MDF_URL, _("Get MDF URL..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu35->Append(ID_MENUITEM_GET_MDF, _("Get MDF..."), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu35, _("VSCP"));
  wxMenu* itemMenu43 = new wxMenu;
  itemMenu43->Append(ID_MENUITEM_RX_GRID_WIDTH, _("Save RX Grid widths as standard"), wxEmptyString, wxITEM_NORMAL);
  itemMenu43->Append(ID_MENUITEM_SAVE_TX_GRID_WIDTH, _("Save TX Grid widths as standard"), wxEmptyString, wxITEM_NORMAL);
  itemMenu43->AppendSeparator();
  itemMenu43->Append(ID_MENUITEM1, _("Auto Reply Settings..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu43->Append(ID_MENUITEM2, _("Set Burst Count..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu43->Append(ID_MENUITEM3, _("Set Filter..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu43->AppendSeparator();
  itemMenu43->Append(ID_MENUITEM, _("Configuration..."), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu43, _("Configuration"));
  wxMenu* itemMenu52 = new wxMenu;
  menuBar->Append(itemMenu52, _("Tools"));
  wxMenu* itemMenu53 = new wxMenu;
  itemMenu53->Append(ID_MENUITEM_VSCP_HELP, _("VSCP Works Help"), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->Append(ID_MENUITEM_VSCP_FAQ, _("Frequently Asked Questions"), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->Append(ID_MENUITEM_VSCP_SHORTCUTS, _("Keyboard shortcuts"), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->AppendSeparator();
  itemMenu53->Append(ID_MENUITEM_VSCP_THANKS, _("Thanks..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->Append(ID_MENUITEM_VSCP_CREDITS, _("Credits..."), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->AppendSeparator();
  itemMenu53->Append(ID_MENUITEM_VSCP_VSCP_SITE, _("Go to VSCP site"), wxEmptyString, wxITEM_NORMAL);
  itemMenu53->AppendSeparator();
  itemMenu53->Append(ID_MENUITEM_VSCP_ABOUT, _("About"), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu53, _("Help"));
  itemFrame1->SetMenuBar(menuBar);

  wxToolBar* itemToolBar2 = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR_VSCP_SESSION );
  wxBitmap itemtool3Bitmap(itemFrame1->GetBitmapResource(wxT("open.xpm")));
  wxBitmap itemtool3BitmapDisabled;
  itemToolBar2->AddTool(ID_MENUITEM_VSCP_LOAD_MSG_LIST, wxEmptyString, itemtool3Bitmap, itemtool3BitmapDisabled, wxITEM_NORMAL, _("Fetch data from file"), wxEmptyString);
  wxBitmap itemtool4Bitmap(itemFrame1->GetBitmapResource(wxT("save.xpm")));
  wxBitmap itemtool4BitmapDisabled;
  itemToolBar2->AddTool(ID_MENUITEM_VSCP_SAVE_MSG_LIST, wxEmptyString, itemtool4Bitmap, itemtool4BitmapDisabled, wxITEM_NORMAL, _("Save data to file"), wxEmptyString);
  itemToolBar2->AddSeparator();
  wxBitmap itemtool6Bitmap(itemFrame1->GetBitmapResource(wxT("cut.xpm")));
  wxBitmap itemtool6BitmapDisabled;
  itemToolBar2->AddTool(ID_TOOL_VSCP_CUT, wxEmptyString, itemtool6Bitmap, itemtool6BitmapDisabled, wxITEM_NORMAL, _("Remove selected row(s)"), wxEmptyString);
  wxBitmap itemtool7Bitmap(itemFrame1->GetBitmapResource(wxT("copy.xpm")));
  wxBitmap itemtool7BitmapDisabled;
  itemToolBar2->AddTool(ID_TOOL_VSCP_COPY, wxEmptyString, itemtool7Bitmap, itemtool7BitmapDisabled, wxITEM_NORMAL, _("Copy selected row(s) \nto the clipboard"), wxEmptyString);
  wxBitmap itemtool8Bitmap(itemFrame1->GetBitmapResource(wxT("paste.xpm")));
  wxBitmap itemtool8BitmapDisabled;
  itemToolBar2->AddTool(ID_TOOL_VSCP_PASTE, wxEmptyString, itemtool8Bitmap, itemtool8BitmapDisabled, wxITEM_NORMAL, _("Paste row(s) from clipboard"), wxEmptyString);
  itemToolBar2->AddSeparator();
  wxBitmap itemtool10Bitmap(itemFrame1->GetBitmapResource(wxT("Print.xpm")));
  wxBitmap itemtool10BitmapDisabled;
  itemToolBar2->AddTool(ID_TOOL_VSCP_PRINT, wxEmptyString, itemtool10Bitmap, itemtool10BitmapDisabled, wxITEM_NORMAL, _("Print selected or all row(s)"), wxEmptyString);
  itemToolBar2->AddSeparator();
  itemToolBar2->AddSeparator();
  m_BtnActivateInterface = new wxToggleButton;
  m_BtnActivateInterface->Create( itemToolBar2, ID_TOGGLEBUTTON_ACTIVATE, _("Connected"), wxDefaultPosition, wxSize(120, -1), 0 );
  m_BtnActivateInterface->SetValue(true);
  if (frmVSCPSession::ShowToolTips())
    m_BtnActivateInterface->SetToolTip(_("Acticate/Deactivate the interface"));
  m_BtnActivateInterface->SetForegroundColour(wxColour(255, 255, 255));
  m_BtnActivateInterface->SetBackgroundColour(wxColour(165, 42, 42));
  m_BtnActivateInterface->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
  itemToolBar2->AddControl(m_BtnActivateInterface);
  itemToolBar2->Realize();
  itemFrame1->SetToolBar(itemToolBar2);

  m_pPanel = new wxPanel;
  m_pPanel->Create( itemFrame1, ID_PANEL_VSCP_SESSION, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  wxBoxSizer* itemBoxSizer65 = new wxBoxSizer(wxVERTICAL);
  m_pPanel->SetSizer(itemBoxSizer65);

  wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer65->Add(itemBoxSizer66, 0, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer66->Add(itemBoxSizer67, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlGridReceive = new wxGrid;
  m_ctrlGridReceive->Create( m_pPanel, ID_VSCP_GRID_RECEIVE, wxDefaultPosition, wxSize(550, 300), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_ctrlGridReceive->SetName(_T("vscprcveventgrid"));
  m_ctrlGridReceive->SetBackgroundColour(wxColour(204, 244, 244));
  m_ctrlGridReceive->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
  m_ctrlGridReceive->SetDefaultColSize(100);
  m_ctrlGridReceive->SetDefaultRowSize(18);
  m_ctrlGridReceive->SetColLabelSize(18);
  m_ctrlGridReceive->SetRowLabelSize(30);
  itemBoxSizer67->Add(m_ctrlGridReceive, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

  m_ctrlRcvHtmlInfo = new wxHtmlWindow;
  m_ctrlRcvHtmlInfo->Create( m_pPanel, ID_HTMLWINDOW_RCVINFO, wxDefaultPosition, wxSize(300, 300), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemBoxSizer67->Add(m_ctrlRcvHtmlInfo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer65->Add(itemBoxSizer70, 0, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer70->Add(itemBoxSizer71, 0, wxGROW|wxALL, 0);

  wxBoxSizer* itemBoxSizer72 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer71->Add(itemBoxSizer72, 0, wxALIGN_TOP|wxALL, 5);

  m_btnAdd = new wxBitmapButton;
  m_btnAdd->Create( m_pPanel, ID_BITMAPBUTTON_TX_ADD, itemFrame1->GetBitmapResource(wxT("New1.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnAdd->SetToolTip(_("Add transmission line"));
  itemBoxSizer72->Add(m_btnAdd, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnEdit = new wxBitmapButton;
  m_btnEdit->Create( m_pPanel, ID_BITMAPBUTTON_TX_EDIT, itemFrame1->GetBitmapResource(wxT("copy.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnEdit->SetToolTip(_("Edit selected line"));
  itemBoxSizer72->Add(m_btnEdit, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnDelete = new wxBitmapButton;
  m_btnDelete->Create( m_pPanel, ID_BITMAPBUTTONID_MENUITEM_TX_DELETE, itemFrame1->GetBitmapResource(wxT("delete.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnDelete->SetToolTip(_("Delete selected line"));
  itemBoxSizer72->Add(m_btnDelete, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  itemBoxSizer72->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_btnLoadSet = new wxBitmapButton;
  m_btnLoadSet->Create( m_pPanel, ID_BITMAPBUTTONID_MENUITEM_TX_LOAD, itemFrame1->GetBitmapResource(wxT("open.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnLoadSet->SetToolTip(_("Load transmission set from file..."));
  itemBoxSizer72->Add(m_btnLoadSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnSaveSet = new wxBitmapButton;
  m_btnSaveSet->Create( m_pPanel, ID_BITMAPBUTTONID_MENUITEM_TX_SAVE, itemFrame1->GetBitmapResource(wxT("filesaveas.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnSaveSet->SetToolTip(_("Save transmission set to file..."));
  itemBoxSizer72->Add(m_btnSaveSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  wxBoxSizer* itemBoxSizer79 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer71->Add(itemBoxSizer79, 5, wxALIGN_TOP|wxALL, 0);

  m_ctrlGridTransmission = new wxGrid;
  m_ctrlGridTransmission->Create( m_pPanel, ID_VSCP_GRID_TRANSMISSION, wxDefaultPosition, wxSize(700, 190), wxHSCROLL|wxVSCROLL );
  if (frmVSCPSession::ShowToolTips())
    m_ctrlGridTransmission->SetToolTip(_("Right click for menu / double click to transmit"));
  m_ctrlGridTransmission->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
  m_ctrlGridTransmission->SetDefaultColSize(100);
  m_ctrlGridTransmission->SetDefaultRowSize(18);
  m_ctrlGridTransmission->SetColLabelSize(18);
  m_ctrlGridTransmission->SetRowLabelSize(30);
  m_ctrlGridTransmission->CreateGrid(1, 6, wxGrid::wxGridSelectRows);
  itemBoxSizer79->Add(m_ctrlGridTransmission, 5, wxGROW|wxALL, 2);

  wxBoxSizer* itemBoxSizer81 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer71->Add(itemBoxSizer81, 0, wxALIGN_TOP|wxALL, 5);

  m_btnSend = new wxBitmapButton;
  m_btnSend->Create( m_pPanel, ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND, itemFrame1->GetBitmapResource(wxT("redo.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  if (frmVSCPSession::ShowToolTips())
    m_btnSend->SetToolTip(_("Transmit event from selected row(s)"));
  itemBoxSizer81->Add(m_btnSend, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  itemBoxSizer81->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

  m_btnActivate = new wxBitmapButton;
  m_btnActivate->Create( m_pPanel, ID_BITMAPBUTTON12, itemFrame1->GetBitmapResource(wxT("Print.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  m_btnActivate->Show(false);
  itemBoxSizer81->Add(m_btnActivate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

  m_btnClear = new wxBitmapButton;
  m_btnClear->Create( m_pPanel, ID_BITMAPBUTTON13, itemFrame1->GetBitmapResource(wxT("open.xpm")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  m_btnClear->Show(false);
  itemBoxSizer81->Add(m_btnClear, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    ////@end frmVSCPSession content construction

    m_ctrlGridReceive->EnableEditing(false);

    // Create the grid table
    m_pgridTable = new BigGridTable();

    // Set gridtable
    m_ctrlGridReceive->SetTable(m_pgridTable, true, wxGrid::wxGridSelectRows);

    m_pgridTable->SetAttrProvider(new RXGridCellAttrProvider);

    m_ctrlGridReceive->SetCellHighlightPenWidth(0); // No ugly cell selection

    /*
      m_ctrlGridReceive->SetCellBackgroundColour( g_Config.m_VscpRcvFrameBgColour );
      m_ctrlGridReceive->SetGridLineColour( g_Config.m_VscpRcvFrameLineColour );
      m_ctrlGridReceive->SetCellTextColour( g_Config.m_VscpRcvFrameTextColour );
     */
    m_ctrlGridReceive->SetRowLabelSize(50);

    m_ctrlGridReceive->SetDefaultRowSize(g_Config.m_VscpRcvFrameRowLineHeight);

    // Dir
    m_ctrlGridReceive->SetColSize(0, g_Config.m_VscpRcvFieldWidth[0]);
    m_ctrlGridReceive->SetColLabelValue(0, g_Config.m_VscpRcvFieldText[0]);

    // Class
    m_ctrlGridReceive->SetColSize(1, g_Config.m_VscpRcvFieldWidth[1]);
    m_ctrlGridReceive->SetColLabelValue(1, g_Config.m_VscpRcvFieldText[1]);

    // Type
    m_ctrlGridReceive->SetColSize(2, g_Config.m_VscpRcvFieldWidth[2]);
    m_ctrlGridReceive->SetColLabelValue(2, g_Config.m_VscpRcvFieldText[2]);


    // Notes
    m_ctrlGridReceive->SetColSize(3, g_Config.m_VscpRcvFieldWidth[3]);
    m_ctrlGridReceive->SetColLabelValue(3, g_Config.m_VscpRcvFieldText[3]);

    // Make all content visible
    m_ctrlGridReceive->AutoSizeRows();

    // *** 


    m_ctrlGridTransmission->EnableEditing(false); // No uncontrolled editing
    m_ctrlGridTransmission->SetSelectionMode(wxGrid::wxGridSelectRows);
    m_ctrlGridTransmission->SetCellHighlightPenWidth(0); // No ugly cell selection

    m_ctrlGridTransmission->SetColSize(0, g_Config.m_VscpTrmitFieldWidth[0]);
    m_ctrlGridTransmission->SetColLabelValue(0, g_Config.m_VscpTrmitFieldText[0]);
    m_ctrlGridTransmission->SetColSize(1, g_Config.m_VscpTrmitFieldWidth[1]);
    m_ctrlGridTransmission->SetColLabelValue(1, g_Config.m_VscpTrmitFieldText[1]);
    m_ctrlGridTransmission->SetColSize(2, g_Config.m_VscpTrmitFieldWidth[2]);
    m_ctrlGridTransmission->SetColLabelValue(2, g_Config.m_VscpTrmitFieldText[2]);
    m_ctrlGridTransmission->SetColSize(3, g_Config.m_VscpTrmitFieldWidth[3]);
    m_ctrlGridTransmission->SetColLabelValue(3, g_Config.m_VscpTrmitFieldText[3]);
    m_ctrlGridTransmission->SetColSize(4, g_Config.m_VscpTrmitFieldWidth[4]);
    m_ctrlGridTransmission->SetColLabelValue(4, g_Config.m_VscpTrmitFieldText[4]);
    m_ctrlGridTransmission->SetColSize(5, g_Config.m_VscpTrmitFieldWidth[5]);
    m_ctrlGridTransmission->SetColLabelValue(5, g_Config.m_VscpTrmitFieldText[5]);
    m_ctrlGridTransmission->DeleteRows(0);

    // Make all content visible
    m_ctrlGridTransmission->AutoSizeRows();

    // Asssign a vadlid copy of the gridtable to the shared object.
    m_CtrlObject.m_pgridTable = m_pgridTable;

}


//m_ctrlRcvHtmlInfo = new HtmlWindow();
//m_ctrlRcvHtmlInfo->Create( m_pPanel, ID_HTMLWINDOW_RCVINFO, wxDefaultPosition, wxSize(300, 300), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
//itemBoxSizer67->Add(m_ctrlRcvHtmlInfo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_CtrlGridReceive
//

void frmVSCPSession::OnGridCellReceiveRightClick(wxGridEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_CtrlGridReceive
//

void frmVSCPSession::OnGridLabelLeftDClick(wxGridEvent& event)
{
    event.Skip();
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_ABOUT
//

void frmVSCPSession::OnMenuitemVscpAboutClick(wxCommandEvent& event)
{
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmVSCPSession::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmVSCPSession::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    ////@begin frmVSCPSession bitmap retrieval
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
  else if (name == _T("New1.xpm"))
  {
    wxBitmap bitmap(new_xpm);
    return bitmap;
  }
  else if (name == _T("delete.xpm"))
  {
    wxBitmap bitmap(delete_xpm);
    return bitmap;
  }
  else if (name == _T("filesaveas.xpm"))
  {
    wxBitmap bitmap(filesaveas_xpm);
    return bitmap;
  }
  else if (name == _T("redo.xpm"))
  {
    wxBitmap bitmap(redo_xpm);
    return bitmap;
  }
  return wxNullBitmap;
    ////@end frmVSCPSession bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmVSCPSession::GetIconResource(const wxString& name)
{
    // Icon retrieval
    ////@begin frmVSCPSession icon retrieval
  wxUnusedVar(name);
  if (name == _T("fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
    ////@end frmVSCPSession icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SESSION_EXIT
//

void frmVSCPSession::OnMenuitemVscpSessionExitClick(wxCommandEvent& event)
{
    Close();
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_SELECT_CELL event handler for ID_CtrlGridReceive
//

void frmVSCPSession::OnSelectCell(wxGridEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOAD_MSG_LIST
//

void frmVSCPSession::LoadRXEventList(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;
    wxXmlDocument doc;


    // If the grid contains row ask if we should clear it before
    // loading new data
    if (m_pgridTable->m_rxList.GetCount()) {
        if (wxYES == wxMessageBox(_("Should current rows be removed before loading new set from file?"),
                _("Remove rows?"),
                wxYES_NO | wxICON_QUESTION)) {
            m_ctrlGridReceive->DeleteRows(0, m_ctrlGridReceive->GetNumberRows());

            RXLIST::iterator iter;
            for (iter = m_pgridTable->m_rxList.begin(); iter != m_pgridTable->m_rxList.end(); ++iter) {
                VscpRXObj *obj = *iter;
                if (NULL != obj) delete obj;
            }

            m_pgridTable->m_rxList.Clear();

        }
    }

    // First find a path to read RX data from
    wxFileDialog dlg(this,
            _("Choose file to load events from "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("rxevents"),
            _("RX Data Files (*.txd)|*.rxd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;
        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Prepare to loading VSCP events..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);


        if (!doc.Load(dlg.GetPath())) {
            return;
        }

        progressDlg.Pulse(_("Checking VSCP events file..."));

        // start processing the XML file xxxx
        if (doc.GetRoot()->GetName() != wxT("vscprxdata")) {
            return;
        }

        progressDlg.Pulse(_("Loading VSCP events file..."));

        //progressDlg.Show();
        m_ctrlGridReceive->BeginBatch();


        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if (child->GetName() == wxT("event")) {

                // Allocate a new transmission set
                VscpRXObj *pObj = new VscpRXObj;
                if (NULL == pObj) {
                    wxMessageBox(_("Unable to create event object."));
                    return;
                }

                pObj->m_pEvent = NULL;

                pObj->m_pEvent = new vscpEvent;
                if (NULL == pObj->m_pEvent) {
                    delete pObj;
                    wxMessageBox(_("Unable to create VSCP event storage."));
                    return;
                }

                pObj->m_pEvent->pdata = NULL;

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {

                    if (subchild->GetName() == wxT("dir")) {
                        str = subchild->GetNodeContent();
                        if (wxNOT_FOUND != str.Find(_("rx"))) {
                            pObj->m_nDir = VSCP_EVENT_DIRECTION_RX;
                        } else {
                            pObj->m_nDir = VSCP_EVENT_DIRECTION_TX;
                        }
                    } else if (subchild->GetName() == wxT("time")) {
                        str = subchild->GetNodeContent();
                        pObj->m_time.ParseDateTime(str);
                    } else if (subchild->GetName() == wxT("class")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->vscp_class = readStringValue(str);
                    } else if (subchild->GetName() == wxT("type")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->vscp_type = readStringValue(str);
                    } else if (subchild->GetName() == wxT("head")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->head = (uint8_t) readStringValue(str);
                    } else if (subchild->GetName() == wxT("guid")) {
                        str = subchild->GetNodeContent();
                        getGuidFromString(pObj->m_pEvent, str);
                    } else if (subchild->GetName() == wxT("data")) {
                        str = subchild->GetNodeContent();
                        getVscpDataFromString(pObj->m_pEvent, str);
                    } else if (subchild->GetName() == wxT("timestamp")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->timestamp = readStringValue(str);
                    } else if (subchild->GetName() == wxT("period")) {
                        pObj->m_wxStrNote = subchild->GetNodeContent();
                    }


                    subchild = subchild->GetNext();

                } // while subchild

                // Update progress dialog
                progressDlg.Pulse(_("Loading VSCP events..."));

                // Append to the RX list
                m_pgridTable->writeEvent(pObj, true);

            }

            child = child->GetNext();

        } // while child

        progressDlg.Update(100);

        m_ctrlGridReceive->EndBatch();

        // If selected mow the added row into seight
        if (g_Config.m_bAutoscollRcv) {
            m_ctrlGridReceive->MakeCellVisible(m_pgridTable->m_rxList.GetCount() - 1, 0);
        }

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SAVE_MSG_LIST
//

void frmVSCPSession::SaveRXEventList(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;

    // First find a path to save RX data to
    wxFileDialog dlg(this,
            _("Please choose a file to save received events to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("rxevents"),
            _("RX Data Files (*.txd)|*.rxd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;

        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Saving VSCP events..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // RX data start
        pFileStream->Write("<vscprxdata>\n", strlen("<vscprxdata>\n"));

        RXLIST::iterator iter;
        for (iter = m_pgridTable->m_rxList.begin(); iter != m_pgridTable->m_rxList.end(); ++iter) {

            VscpRXObj *pobj = *iter;
            pFileStream->Write("<event>\n", strlen("<event>\n"));


            pFileStream->Write("<dir>", strlen("<dir>"));
            if (VSCP_EVENT_DIRECTION_RX == pobj->m_nDir) {
                pFileStream->Write("rx", strlen("rx"));
            } else {
                pFileStream->Write("tx", strlen("tx"));
            }
            pFileStream->Write("</dir>\n", strlen("</dir>\n"));

            pFileStream->Write("<time>", strlen("<time>"));
            str = pobj->m_time.FormatISODate() + _(" ") + pobj->m_time.FormatISOTime();
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</time>\n", strlen("</time>\n"));

            pFileStream->Write("<head>", strlen("<head>"));
            str.Printf(_("%d"), pobj->m_pEvent->head);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</head>\n", strlen("</head>\n"));

            pFileStream->Write("<class>", strlen("<class>"));
            str.Printf(_("%d"), pobj->m_pEvent->vscp_class);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</class>\n", strlen("</class>\n"));

            pFileStream->Write("<type>", strlen("<type>"));
            str.Printf(_("%d"), pobj->m_pEvent->vscp_type);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</type>\n", strlen("</type>\n"));

            pFileStream->Write("<guid>", strlen("<guid>"));
            writeGuidToString(pobj->m_pEvent, str);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</guid>\n", strlen("</guid>\n"));

            pFileStream->Write("<sizedata>", strlen("<sizedata>")); // Not used by read routine	
            str.Printf(_("%d"), pobj->m_pEvent->sizeData);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

            if (0 != pobj->m_pEvent->sizeData) {
                pFileStream->Write("<data>", strlen("<data>"));
                writeVscpDataToString(pobj->m_pEvent, str);
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</data>\n", strlen("</data>\n"));
            }

            pFileStream->Write("<timestamp>", strlen("<timestamp>"));
            str.Printf(_("%lu"), pobj->m_pEvent->timestamp);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</timestamp>\n", strlen("</timestamp>\n"));

            pFileStream->Write("<note>", strlen("<note>"));
            pFileStream->Write(pobj->m_wxStrNote.mb_str(), strlen(pobj->m_wxStrNote.mb_str()));
            pFileStream->Write("</note>\n", strlen("</note>\n"));

            pFileStream->Write("</event>\n", strlen("</event>\n"));

        }

        // Transmission set end
        pFileStream->Write("</vscprxdata>\n", strlen("</vscprxdata>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_GRID_TRANSMISSION
//

void frmVSCPSession::OnCellReceiveLeftClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        m_ctrlGridTransmission->SelectRow(event.GetRow());
    } else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        m_ctrlGridReceive->SelectRow(event.GetRow());
        VscpRXObj *pRecord = m_pgridTable->readEvent(event.GetRow());
        if (NULL != pRecord) {
            fillRxHtmlInfo(pRecord);
        }
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  fillRxHtmlInfo
//

void frmVSCPSession::fillRxHtmlInfo(VscpRXObj *pRecord)
{
    wxString str, wrkstr;

    // Check pointers
    if (NULL == pRecord) return;
    if (NULL == pRecord->m_pEvent) return;

    str = _("<html><body><h3>VSCP Event</h3>");
    str += _("<small><font color=\"#993399\">");
    if (VSCP_EVENT_DIRECTION_RX == pRecord->m_nDir) {
        str += _("Received event");
    } else {
        str += _("Transmitted event");
    }
    str += _("</font></small><br>");
    str += _("<b>Time: </b>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    str += pRecord->m_time.FormatISODate() + _(" ") + pRecord->m_time.FormatISOTime();
    str += _("</tt></font><br>");
    str += _("<br>");
    str += _("<b>Class: </b>");
    str += _("<font color=\"rgb(0, 102, 0);\">");

    // Class
    wrkstr = _("");
    if (g_Config.m_UseSymbolicNames) {
        wxString strClass =
                g_vscpinfo.getClassDescription(pRecord->m_pEvent->vscp_class);
        if (0 == strClass.Length()) strClass = _("Unknown class");
        wrkstr = wxString::Format(_("%s <br><tt>0x%04X, %d</tt>"),
                strClass.c_str(),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
    } else {
        wrkstr = wxString::Format(_("<tt>0x%04X, %d</tt>"),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
    }
    str += wrkstr;
    str += _("</font>");
    str += _("<br><b>");
    str += _("Type:");
    str += _("</b> ");
    str += _("<font color=\"rgb(0, 102, 0);\">");

    // Type
    wrkstr = _("");
    if (g_Config.m_UseSymbolicNames) {
        wxString strType =
                g_vscpinfo.getTypeDescription(pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_type);
        if (0 == strType.Length()) strType = _("Unknown type");
        wrkstr = wxString::Format(_("%s <br><tt>0x%04X, %d </tt>"),
                strType.c_str(),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
    } else {
        wrkstr = wxString::Format(_("<tt>0x%04X, %d</tt>"),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
    }
    str += wrkstr;
    str += _("</font>");
    str += _("<br><br>");

    // Data
    str += _("<b>Data count:</b> ");
    str += _("<color color=\"rgb(0, 0, 153);\"><tt>");
    str += wxString::Format(_("%d"), pRecord->m_pEvent->sizeData);
    ;
    str += _("</tt></font><br>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    writeVscpDataToString(pRecord->m_pEvent, wrkstr, true);
    str += wrkstr;
    str += _("</tt></font><br>");
    str += _("<font color=\"rgb(0, 0, 153);\">");
    wrkstr = getRealTextData(pRecord->m_pEvent);
    makeHtml(wrkstr);
    str += wrkstr;
    str += _("</font><br><br>");
    str += _("<b>From GUID:</b><br>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    writeGuidToString4Rows(pRecord->m_pEvent, wrkstr);
    str += wrkstr;
    str += _("</tt></font><br><br>");
    str += _("<b>Head:</b> ");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    str += wxString::Format(_("%02X "), pRecord->m_pEvent->head);

    if (pRecord->m_pEvent->head & VSCP_HEADER_HARD_CODED) {
        str += _(" [Hard coded] ");
    }

    if (pRecord->m_pEvent->head & VSCP_HEADER_NO_CRC) {
        str += _(" [No CRC Calculation] ");
    }

    str += _("</font></tt><br>");
    str += _("<b>Timestamp:</b> ");
    str += _("<font color=\"rgb(0, 0, 153);\">");
    str += wxString::Format(_("%08X"), pRecord->m_pEvent->timestamp);
    str += _("</font>");
    str += _("<br>");
    str += _("<br></body></html>");

    m_ctrlRcvHtmlInfo->SetPage(str);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  addToTxGrid
//

bool frmVSCPSession::addToTxGrid(VscpTXObj *pObj, int selrow)
{
    int row;
    wxString str;

    // Check pointer
    if (NULL == pObj) return false;

    if (-1 == selrow) {
        m_ctrlGridTransmission->AppendRows(1);
        row = m_ctrlGridTransmission->GetNumberRows();
    } else {
        row = selrow + 1;
    }

    // Active checkbox
    m_ctrlGridTransmission->SetCellRenderer(row - 1, 0, new wxGridCellBoolRenderer);
    m_ctrlGridTransmission->SetCellEditor(row - 1, 0, new wxGridCellBoolEditor);
    m_ctrlGridTransmission->SetCellAlignment(wxALIGN_CENTRE, row - 1, 0);

    // Name
    m_ctrlGridTransmission->SetCellValue(row - 1, 1, pObj->m_wxStrName);
    m_ctrlGridTransmission->SetCellRenderer(row - 1, 2, new wxGridCellStringRenderer);

    // Period
    m_ctrlGridTransmission->SetCellValue(row - 1, 2, wxString::Format(_("%d"), pObj->m_period));
    m_ctrlGridTransmission->SetCellRenderer(row - 1, 2, new wxGridCellStringRenderer);
    m_ctrlGridTransmission->SetCellAlignment(wxALIGN_CENTRE, row - 1, 2);

    // Count
    m_ctrlGridTransmission->SetCellValue(row - 1, 3, wxString::Format(_("%d"), pObj->m_count));
    m_ctrlGridTransmission->SetCellRenderer(row - 1, 3, new wxGridCellStringRenderer);
    m_ctrlGridTransmission->SetCellAlignment(wxALIGN_CENTRE, row - 1, 3);

    // Trigger
    m_ctrlGridTransmission->SetCellValue(row - 1, 4, wxString::Format(_("%d"), pObj->m_trigger));
    m_ctrlGridTransmission->SetCellRenderer(row - 1, 4, new wxGridCellStringRenderer);
    m_ctrlGridTransmission->SetCellAlignment(wxALIGN_CENTRE, row - 1, 4);

    // Event
    str.Printf(_("class=%d type=%d dsize=%d \ndata="),
            pObj->m_Event.vscp_class,
            pObj->m_Event.vscp_type,
            pObj->m_Event.sizeData);

    for (int i = 0; i < pObj->m_Event.sizeData; i++) {
        str += wxString::Format(_("%02X "), pObj->m_Event.pdata[i]);
        if (i && !(i % 8)) {
            str += _("\n\t\t");
        }
    }

    m_ctrlGridTransmission->SetCellValue(row - 1, 5, str);
    m_ctrlGridTransmission->AutoSizeRow(row - 1);

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID_TRANSMISSION
//

void frmVSCPSession::OnCellTxRightClick(wxGridEvent& event)
{
    wxMenu menu;

    wxPoint pos = ClientToScreen(event.GetPosition());

    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {

        menu.Append(Menu_Popup_TX_Transmit, _T("Transmit event from selected row(s)"));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_TX_Add, _T("Add transmission row..."));
        menu.Append(Menu_Popup_TX_Edit, _T("Edit transmission row..."));
        menu.Append(Menu_Popup_TX_Delete, _T("Delete transmission row..."));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_TX_Clone, _T("Clone transmission row..."));
        menu.AppendSeparator();
        menu.AppendCheckItem(Menu_Popup_TX_Periodic, _T("Enable periodic transmission"));
        menu.Check(Menu_Popup_TX_Periodic, true);
        //menu.Append(Menu_Popup_Submenu, _T("&Submenu"), CreateDummyMenu(NULL));
        //menu.Append(Menu_Popup_ToBeDeleted, _T("To be &deleted"));
        //menu.AppendCheckItem(Menu_Popup_ToBeChecked, _T("To be &checked"));
        //menu.Append(Menu_Popup_ToBeGreyed, _T("To be &greyed"),
        //              _T("This menu item should be initially greyed out"));
        //menu.AppendSeparator();
        //menu.Append(Menu_File_Quit, _T("E&xit"));

        //menu.Delete(Menu_Popup_ToBeDeleted);
        //menu.Check(Menu_Popup_ToBeChecked, true);
        //menu.Enable(Menu_Popup_ToBeGreyed, false);
    } else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        menu.Append(Menu_Popup_TX_Transmit, _T("Edit row..."));
        menu.Append(Menu_Popup_TX_Transmit, _T("Add note..."));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_TX_Transmit, _T("Copy row"));
        menu.Append(Menu_Popup_TX_Transmit, _T("Cut row"));
        menu.Append(Menu_Popup_TX_Transmit, _T("Paste row"));
        menu.AppendSeparator();
        menu.Append(Menu_Popup_TX_Transmit, _T("Transmit selected row(s)"));
        menu.Append(Menu_Popup_TX_Transmit, _T("Edit and transmit selected row(s)"));
        menu.Append(Menu_Popup_TX_Transmit, _T("Save row(s) as transmission object(s)"));
        menu.AppendSeparator();
        menu.Append(ID_MENUITEM_VSCP_LOAD_MSG_LIST, _T("Load VSCP events from file..."));
        menu.Append(ID_MENUITEM_VSCP_SAVE_MSG_LIST, _T("Save VSCP events to file..."));
    }

    PopupMenu(&menu/*, pos.x, pos.y*/);

    event.Skip(false);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_GRID_TRANSMISSION
//

void frmVSCPSession::OnCellTxLeftClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        m_ctrlGridTransmission->SelectRow(event.GetRow());
    } else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        m_ctrlGridReceive->SelectRow(event.GetRow());
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_VSCP_GRID_TRANSMISSION
//

void frmVSCPSession::OnGridLeftDClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        wxCommandEvent event; // Not uses so we dont care about actual data
        OnTxSendClick(event);
    } else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        ;
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOG
//

void frmVSCPSession::OnMenuitemVscpViewLogClick(wxCommandEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_COUNT
//

void frmVSCPSession::OnMenuitemVscpViewCountClick(wxCommandEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CLEAR_RX
//

void frmVSCPSession::ClearRxList(wxCommandEvent& event)
{
    if (wxYES == wxMessageBox(_("Are you sure that all receive events should be removed?"),
            _("Remove receive rows?"),
            wxYES_NO | wxICON_QUESTION)) {

        wxBusyCursor wait;

        // Remove elements from the receive list
        m_pgridTable->clearEventList();

        // Clear the grid
        if (m_ctrlGridReceive->GetNumberRows()) {
            m_ctrlGridReceive->DeleteRows(0, m_ctrlGridReceive->GetNumberRows());
        }
        //m_ctrlGridReceive->ForceRefresh();
        //m_ctrlGridReceive->MakeCellVisible( 0, 0 );
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_CLEAR_TX
//

void frmVSCPSession::ClearTxList(wxCommandEvent& event)
{
    if (wxYES == wxMessageBox(_("Are you sure that all transmission events should be removed?"),
            _("Remove transmission rows?"),
            wxYES_NO | wxICON_QUESTION)) {
        TXLIST::iterator iter;
        for (iter = m_CtrlObject.m_txList.begin();
                iter != m_CtrlObject.m_txList.end();
                ++iter) {
            VscpTXObj *pobj = *iter;
            if (NULL != pobj) {
                if (NULL != pobj->m_Event.pdata) delete pobj->m_Event.pdata;
                delete pobj;
            }
        }

        // Clear the list
        m_CtrlObject.m_txList.Clear();

        // Clear the grid
        m_ctrlGridTransmission->DeleteRows(0, m_ctrlGridTransmission->GetNumberRows());

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND
//

void frmVSCPSession::OnTxSendClick(wxCommandEvent& event)
{
    if (m_ctrlGridTransmission->GetNumberRows()) {
        wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
        if (selrows.GetCount()) {
            for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                TXLIST::compatibility_iterator node = m_CtrlObject.m_txList.Item(selrows[i]);
                VscpTXObj *obj = node->GetData();

                m_CtrlObject.m_mutexOutQueue.Lock();
                vscpEvent *pEvent = new vscpEvent;
                if (NULL != pEvent) {
                    copyVSCPEvent(pEvent, &obj->m_Event);
                    pEvent->head = 0x00;
                    pEvent->timestamp = 0x00L;
                    m_CtrlObject.m_outQueue.Append(pEvent);
                    m_CtrlObject.m_semOutQue.Post();
                }
                m_CtrlObject.m_mutexOutQueue.Unlock();

            }
        } else {
            wxMessageBox(_("Must select one or more rows to transmit event(s)."));
        }
    } else {
        wxMessageBox(_("Needs selected transmission row(s) to transmit event(s)."));
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON_TX_ADD
//

void frmVSCPSession::OnTxAddClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    dlgVSCPTrmitElement dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

        VscpTXObj *pObj = new VscpTXObj;

        if (NULL != pObj) {
            if (dlg.getEventData(pObj)) {

                // Append to the TX list
                m_CtrlObject.m_txList.Append(pObj);

                // Add to TX grid
                addToTxGrid(pObj);

            } else {
                wxMessageBox(_("Failed to get data from dialog."));
            }
        } else {
            wxMessageBox(_("Failed to allocate data."));
        }
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON_TX_EDIT
//

void frmVSCPSession::OnTxEditClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    dlgVSCPTrmitElement dlg(this);

    wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
    if (m_ctrlGridTransmission->GetNumberRows() && selrows.GetCount()) {


        for (unsigned short i = 0; i < selrows.GetCount(); i++) {

            TXLIST::compatibility_iterator node = m_CtrlObject.m_txList.Item(selrows.Item(i));
            VscpTXObj *pObj = node->GetData();

            dlg.writeEventData(pObj);

            if (wxID_OK == dlg.ShowModal()) {

                dlg.getEventData(pObj);

                // Replace info on TX grid
                addToTxGrid(pObj, selrows.Item(i));

            }
        }
    } else {
        wxMessageBox(_("No row selected so unable to edit."));
    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_TX_DELETE
//

void frmVSCPSession::OnTxDeleteClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    if (m_ctrlGridTransmission->GetNumberRows()) {
        wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
        if (selrows.GetCount()) {
            if (wxYES == wxMessageBox(_("Selected rows will be removed. Shure that is what is intended?"),
                    _("Remove rows?"),
                    wxYES_NO | wxICON_QUESTION)) {
                for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                    m_ctrlGridTransmission->DeleteRows(selrows.Item(i), 1);
                    TXLIST::compatibility_iterator node = m_CtrlObject.m_txList.Item(i);
                    VscpTXObj *obj = node->GetData();
                    if (NULL != obj) delete obj;
                    m_CtrlObject.m_txList.DeleteNode(node);
                }
            }
        } else {
            wxMessageBox(_("At least one row must be selected."));
        }
    } else {
        wxMessageBox(_("There must be rows to delete to use this function."));
    }

    event.Skip(false);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_TX_LOAD
//

void frmVSCPSession::OnTxLoadClick(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;
    wxXmlDocument doc;

    wxBusyCursor wait;

    // If the grid contains row ask if we should clear it before
    // loading new data
    if (m_ctrlGridTransmission->GetNumberRows()) {
        if (wxYES == wxMessageBox(_("Should current transmission rows be removed before loading new set from file?"),
                _("Remove rows?"),
                wxYES_NO | wxICON_QUESTION)) {
            m_ctrlGridTransmission->DeleteRows(0, m_ctrlGridTransmission->GetNumberRows());

            TXLIST::iterator iter;
            for (iter = m_CtrlObject.m_txList.begin(); iter != m_CtrlObject.m_txList.end(); ++iter) {
                VscpTXObj *obj = *iter;
                if (NULL != obj->m_Event.pdata) {
                    delete obj->m_Event.pdata;
                }
                delete obj;
            }

            m_CtrlObject.m_txList.Clear();

        }
    }

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to load transmission set from "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("txset"),
            _("TX Data Files (*.txd)|*.txd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {


        if (!doc.Load(dlg.GetPath())) {
            return;
        }

        // start processing the XML file
        if (doc.GetRoot()->GetName() != wxT("vscptxset")) {
            return;
        }

        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if (child->GetName() == wxT("set")) {

                // Allocate a new transmission set
                VscpTXObj *pObj = new VscpTXObj;
                if (NULL == pObj) {
                    wxMessageBox(_("Unable to create transmission object."));
                    return;
                }

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {

                    if (subchild->GetName() == wxT("active")) {
                        str = subchild->GetNodeContent();
                        if (wxNOT_FOUND != str.Find(_("true"))) {
                            pObj->m_bActive = true;
                        } else {
                            pObj->m_bActive = false;
                        }
                    } else if (subchild->GetName() == wxT("name")) {
                        pObj->m_wxStrName = subchild->GetNodeContent();
                    } else if (subchild->GetName() == wxT("class")) {
                        str = subchild->GetNodeContent();
                        pObj->m_Event.vscp_class = readStringValue(str);
                    } else if (subchild->GetName() == wxT("type")) {
                        str = subchild->GetNodeContent();
                        pObj->m_Event.vscp_type = readStringValue(str);
                    } else if (subchild->GetName() == wxT("priority")) {
                        str = subchild->GetNodeContent();
                        setVscpPriority(&pObj->m_Event, readStringValue(str));
                    } else if (subchild->GetName() == wxT("guid")) {
                        wxString property = subchild->GetAttribute(wxT("default"), wxT("false"));
                        if (property.IsSameAs(_("true"), false)) {
                            pObj->m_bUseDefaultGUID = true;
                        } else {
                            pObj->m_bUseDefaultGUID = false;
                        }
                        str = subchild->GetNodeContent();
                        getGuidFromString(&pObj->m_Event, str);
                    } else if (subchild->GetName() == wxT("data")) {
                        str = subchild->GetNodeContent();
                        getVscpDataFromString(&pObj->m_Event, str);
                    } else if (subchild->GetName() == wxT("count")) {
                        str = subchild->GetNodeContent();
                        pObj->m_count = readStringValue(str);
                    } else if (subchild->GetName() == wxT("period")) {
                        str = subchild->GetNodeContent();
                        pObj->m_period = readStringValue(str);
                    } else if (subchild->GetName() == wxT("trigger")) {
                        str = subchild->GetNodeContent();
                        pObj->m_trigger = readStringValue(str);
                    }

                    subchild = subchild->GetNext();

                } // while subchild

                // Append to the TX list
                m_CtrlObject.m_txList.Append(pObj);

                // Add to TX grid
                addToTxGrid(pObj);

            }

            child = child->GetNext();

        } // while child

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_TX_SAVE
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET

void frmVSCPSession::OnTxSaveClick(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save transmission set to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("txset"),
            _("TX Data Files (*.txd)|*.txd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (wxID_OK == dlg.ShowModal()) {

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create transmission set file."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // Transmission set start
        pFileStream->Write("<vscptxset>\n", strlen("<vscptxset>\n"));

        TXLIST::iterator iter;
        for (iter = m_CtrlObject.m_txList.begin(); iter != m_CtrlObject.m_txList.end(); ++iter) {

            VscpTXObj *obj = *iter;
            pFileStream->Write("<set>\n", strlen("<set>\n"));

            pFileStream->Write("<active>", strlen("<active>"));
            if (obj->m_bActive) {
                pFileStream->Write("true", strlen("true"));
            } else {
                pFileStream->Write("false", strlen("false"));
            }

            pFileStream->Write("</active>\n", strlen("</active>\n"));

            pFileStream->Write("<name>", strlen("<name>"));
            pFileStream->Write(obj->m_wxStrName.mb_str(), strlen(obj->m_wxStrName.mb_str()));
            pFileStream->Write("</name>\n", strlen("</name>\n"));

            pFileStream->Write("<class>", strlen("<class>"));
            str.Printf(_("%d"), obj->m_Event.vscp_class);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</class>\n", strlen("</class>\n"));

            pFileStream->Write("<type>", strlen("<type>"));
            str.Printf(_("%d"), obj->m_Event.vscp_type);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</type>\n", strlen("</type>\n"));

            pFileStream->Write("<priority>", strlen("<priority>"));
            str.Printf(_("%d"), getVscpPriority(&obj->m_Event));
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</priority>\n", strlen("</priority>\n"));

            pFileStream->Write("<guid default=\"", strlen("<guid default=\""));
            if (obj->m_bUseDefaultGUID) {
                pFileStream->Write("true", strlen("true"));
            } else {
                pFileStream->Write("false", strlen("false"));
            }

            pFileStream->Write("\" >", strlen("\" >"));
            writeGuidToString(&obj->m_Event, str);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</guid>\n", strlen("</guid>\n"));

            pFileStream->Write("<sizedata>", strlen("<sizedata>")); // Not used by read routine	
            str.Printf(_("%d"), obj->m_Event.sizeData);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

            if (0 != obj->m_Event.sizeData) {
                pFileStream->Write("<data>", strlen("<data>"));
                writeVscpDataToString(&obj->m_Event, str);
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</data>\n", strlen("</data>\n"));
            }

            pFileStream->Write("<count>", strlen("<count>"));
            str.Printf(_("%lu"), obj->m_count);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</count>\n", strlen("</count>\n"));

            pFileStream->Write("<period>", strlen("<period>"));
            str.Printf(_("%lu"), obj->m_period);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</period>\n", strlen("</period>\n"));

            pFileStream->Write("<trigger>", strlen("<trigger>"));
            str.Printf(_("%lu"), obj->m_trigger);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</trigger>\n", strlen("</trigger>\n"));

            pFileStream->Write("</set>\n", strlen("</set>\n"));
        }

        // Transmission set end
        pFileStream->Write("</vscptxset>\n", strlen("</vscptxset>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTxCloneRow
//

void frmVSCPSession::OnTxCloneRow(wxCommandEvent& event)
{
    dlgVSCPTrmitElement dlg(this);

    wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
    if (m_ctrlGridTransmission->GetNumberRows() && selrows.GetCount()) {

        for (unsigned short i = 0; i < selrows.GetCount(); i++) {

            TXLIST::compatibility_iterator node = m_CtrlObject.m_txList.Item(selrows.Item(i));
            VscpTXObj *pObj = node->GetData();

            VscpTXObj *pNewObj = new VscpTXObj;
            pNewObj->m_Event.pdata = NULL;
            pNewObj->m_pworkerThread = NULL; // Worker thread is note cloded
            pNewObj->m_bActive = false; // Object is inactive
            pNewObj->m_bUseDefaultGUID = pObj->m_bUseDefaultGUID;
            pNewObj->m_count = pObj->m_count;
            pNewObj->m_period = pObj->m_period;
            pNewObj->m_trigger = pObj->m_trigger;
            pNewObj->m_wxStrName = pObj->m_wxStrName;
            memcpy(&pNewObj->m_Event, &pObj->m_Event, sizeof( vscpEvent));
            if (pObj->m_Event.sizeData) {
                pNewObj->m_Event.pdata = new unsigned char[ pObj->m_Event.sizeData ];
                memcpy(pNewObj->m_Event.pdata, pObj->m_Event.pdata, pObj->m_Event.sizeData);
            } else {
                pNewObj->m_Event.pdata = NULL;
            }

            // Append to the TX list
            m_CtrlObject.m_txList.Append(pNewObj);

            // Add to TX grid
            addToTxGrid(pNewObj);
        }
    } else {
        wxMessageBox(_("No row selected so unable to clone any rows."));
    }

    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_VSCP_SITE
//

void frmVSCPSession::OnMenuitemVscpVscpSiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_HELP
//

void frmVSCPSession::OnMenuitemVscpHelpClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_FAQ
//

void frmVSCPSession::OnMenuitemVscpFaqClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SHORTCUTS
//

void frmVSCPSession::OnMenuitemVscpShortcutsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_THANKS
//

void frmVSCPSession::OnMenuitemVscpThanksClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_CREDITS
//

void frmVSCPSession::OnMenuitemVscpCreditsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventReceive
//

void frmVSCPSession::eventReceive(wxCommandEvent &event)
{
    VscpRXObj *pRecord = (VscpRXObj *) event.GetClientData();
    if (NULL != pRecord) {
        m_pgridTable->writeEvent(pRecord);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventTransmit
//

void frmVSCPSession::eventTransmit(wxCommandEvent &event)
{
    VscpRXObj *pRecord = (VscpRXObj *) event.GetClientData();
    if (NULL != pRecord) {
        m_pgridTable->writeEvent(pRecord);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventPrepareConnect
//

void frmVSCPSession::eventPrepareConnect(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if (m_BtnActivateInterface->GetValue()) {
        if (NULL != m_pProgressDlg) {
            m_pProgressDlg->Update(100);
            m_pProgressDlg = NULL;
        }

        m_pProgressDlg = new wxProgressDialog(_("TCP/IP session"),
                _("Connecting to server..."),
                100,
                this,
                wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        m_pProgressDlg->Update(80);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventConnected
//

void frmVSCPSession::eventConnected(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if (m_BtnActivateInterface->GetValue()) {
        if (NULL != m_pProgressDlg) {
            m_pProgressDlg->Update(100);
            m_pProgressDlg = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventLostCtrlIf
//

void frmVSCPSession::eventLostCtrlIf(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning

    if (NULL != m_pProgressDlg) {
        m_pProgressDlg->Update(100);
        m_pProgressDlg = NULL;
        wxMessageBox(_("Uanble to connect to server!"));
    }

    if (m_BtnActivateInterface->GetValue()) {
        stopWorkerThreads();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventReceive
//

void frmVSCPSession::eventLostRcvIf(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if (m_BtnActivateInterface->GetValue()) {
        stopWorkerThreads();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM3
//

void frmVSCPSession::OnMenuitemSetFilterClick(wxCommandEvent& event)
{
    wxMessageBox(_("Sorry this functionality is not available yet."));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM2
//

void frmVSCPSession::OnMenuitemSetBurstCountClick(wxCommandEvent& event)
{
    wxMessageBox(_("Sorry this functionality is not available yet."));
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM
//

void frmVSCPSession::OnMenuitemConfigurationClick(wxCommandEvent& event)
{
    dlgConfiguration dlg(this);

    if (wxID_OK == dlg.ShowModal()) {
        dlg.getDialogData();
        m_ctrlGridReceive->ForceRefresh();
        m_ctrlGridTransmission->ForceRefresh();
    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_READ_REGISTER
//

void frmVSCPSession::OnMenuitemSetAutoreplyClick(wxCommandEvent& event)
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_RX_GRID_WIDTH
//

void frmVSCPSession::OnMenuitemSaveRxCellWidth(wxCommandEvent& event)
{
    g_Config.m_VscpRcvFieldWidth[0] = m_ctrlGridReceive->GetColSize(0);
    g_Config.m_VscpRcvFieldWidth[1] = m_ctrlGridReceive->GetColSize(1);
    g_Config.m_VscpRcvFieldWidth[2] = m_ctrlGridReceive->GetColSize(2);
    g_Config.m_VscpRcvFieldWidth[3] = m_ctrlGridReceive->GetColSize(3);
    g_Config.m_VscpRcvFieldWidth[4] = m_ctrlGridReceive->GetColSize(4);
    g_Config.m_VscpRcvFieldWidth[5] = m_ctrlGridReceive->GetColSize(5);
    g_Config.m_VscpRcvFieldWidth[6] = m_ctrlGridReceive->GetColSize(6);
    g_Config.m_VscpRcvFieldWidth[7] = m_ctrlGridReceive->GetColSize(7);
    g_Config.m_VscpRcvFieldWidth[8] = m_ctrlGridReceive->GetColSize(8);
    g_Config.m_VscpRcvFieldWidth[9] = m_ctrlGridReceive->GetColSize(9);
    wxGetApp().writeConfiguration();
    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM1
//

void frmVSCPSession::OnMenuitemSaveTxCellWidth(wxCommandEvent& event)
{
    g_Config.m_VscpTrmitFieldWidth[0] = m_ctrlGridTransmission->GetColSize(0);
    g_Config.m_VscpTrmitFieldWidth[1] = m_ctrlGridTransmission->GetColSize(1);
    g_Config.m_VscpTrmitFieldWidth[2] = m_ctrlGridTransmission->GetColSize(2);
    g_Config.m_VscpTrmitFieldWidth[3] = m_ctrlGridTransmission->GetColSize(3);
    g_Config.m_VscpTrmitFieldWidth[4] = m_ctrlGridTransmission->GetColSize(4);
    g_Config.m_VscpTrmitFieldWidth[5] = m_ctrlGridTransmission->GetColSize(5);
    wxGetApp().writeConfiguration();
    event.Skip();
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// TXWorkerThread
//

TXWorkerThread::TXWorkerThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// transmitWorkerThread
//

TXWorkerThread::~TXWorkerThread()
{

}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to read from devices. Read it/them and send
// it/them to all other devices clients.

void *TXWorkerThread::Entry()
{
    eventOutQueue::compatibility_iterator node;
    vscpEvent *pEvent;
    wxCommandEvent eventConnectionLost(wxVSCP_CTRL_LOST_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventPrepConnection(wxVSCP_RCV_PREP_CONNECT_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnected(wxVSCP_RCV_CONNECTED_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);

    /// TCP/IP Control
    VscpTcpIf tcpifControl;

    // Must be a valid control object pointer
    if (NULL == m_pCtrlObject) return NULL;

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventPrepConnection);

    // Connect to the server with the control interface
    if (!tcpifControl.doCmdOpen(m_pCtrlObject->m_ifVSCP.m_strHost,
            m_pCtrlObject->m_ifVSCP.m_port,
            m_pCtrlObject->m_ifVSCP.m_strUser,
            m_pCtrlObject->m_ifVSCP.m_strPassword)) {
        ::wxGetApp().logMsg(_("VSCP TX thread - Unable to connect to server."), DAEMON_LOGMSG_CRITICAL);
        m_pCtrlObject->m_errorControl = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        return NULL;
    }

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnected);

    // Get channel ID
    if (CANAL_ERROR_SUCCESS !=
            tcpifControl.doCmdGetChannelID((uint32_t *) & m_pCtrlObject->m_txChannelID)) {
        ::wxGetApp().logMsg(_("VSCP TX thread - Unable to get channel ID."), DAEMON_LOGMSG_INFO);
    }

    while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {
        if (wxSEMA_TIMEOUT == m_pCtrlObject->m_semOutQue.WaitTimeout(500)) continue;
        m_pCtrlObject->m_mutexOutQueue.Lock();
        node = m_pCtrlObject->m_outQueue.GetFirst();
        pEvent = node->GetData();
        tcpifControl.doCmdSend(pEvent);
        m_pCtrlObject->m_outQueue.DeleteNode(node);
        deleteVSCPevent(pEvent);
        m_pCtrlObject->m_mutexOutQueue.Unlock();
    } // while

    // Close the interface
    tcpifControl.doCmdClose();

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TXWorkerThread::OnExit()
{

}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::RXWorkerThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::~RXWorkerThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *RXWorkerThread::Entry()
{
    int rv;
    VscpTcpIf tcpifReceive;
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnectionLost(wxVSCP_RCV_LOST_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);

    // Must be a valid control object pointer
    if (NULL == m_pCtrlObject) return NULL;

    // Connect to the server with the control interface
    if (!tcpifReceive.doCmdOpen(m_pCtrlObject->m_ifVSCP.m_strHost,
            m_pCtrlObject->m_ifVSCP.m_port,
            m_pCtrlObject->m_ifVSCP.m_strUser,
            m_pCtrlObject->m_ifVSCP.m_strPassword)) {
        ::wxGetApp().logMsg(_("TCP/IP Receive thread - Unable to connect to server."), DAEMON_LOGMSG_CRITICAL);
        m_pCtrlObject->m_errorReceive = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        return NULL;
    }

    // Find the channel id
    tcpifReceive.doCmdGetChannelID((uint32_t *) & m_pCtrlObject->m_rxChannelID);

    // Set filter
    if (CANAL_ERROR_SUCCESS !=
            tcpifReceive.doCmdFilter(&m_pCtrlObject->m_ifVSCP.m_vscpfilter)) {
        ::wxGetApp().logMsg(_("TCP/IP Receive thread - Failed to set filter."), DAEMON_LOGMSG_INFO);
    }

    // Start Receive Loop
    tcpifReceive.doCmdEnterReceiveLoop();

    vscpEvent event;
    while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
                (rv = tcpifReceive.doCmdBlockReceive(&event, 1000))) {

            if (NULL != m_pCtrlObject->m_pVSCPSessionWnd) {

                VscpRXObj *pRecord = new VscpRXObj;

                if (NULL != pRecord) {

                    vscpEvent *pEvent = new vscpEvent;
                    if (NULL != pEvent) {

                        copyVSCPEvent(pEvent, &event);

                        pRecord->m_pEvent = pEvent;
                        pRecord->m_wxStrNote.Empty();
                        pRecord->m_time = wxDateTime::Now();

                        if (pEvent->obid == m_pCtrlObject->m_txChannelID) {
                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;
                        } else {
                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;
                        }

                        eventReceive.SetClientData(pRecord);
                        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventReceive);

                        // Remove data if any
                        if (NULL != event.pdata) {
                            delete [] event.pdata;
                        }

                    } else {
                        delete pRecord;
                    }

                } // record exists

            } // Session window exist

        } else {
            if (CANAL_ERROR_COMMUNICATION == rv) m_pCtrlObject->m_bQuit = true;
        }

    } // while

    // Close the interface
    tcpifReceive.doCmdClose();

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void RXWorkerThread::OnExit()
{

}





///////////////////////////////////////////////////////////////////////////////
//                               D E V I C E
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// deviceThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

deviceThread::deviceThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pCtrlObject = NULL;
    m_preceiveThread = NULL;
    m_pwriteThread = NULL;
}

deviceThread::~deviceThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnectionLost(wxVSCP_RCV_LOST_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);


    // Must have a valid pointer to the control object
    if (NULL == m_pCtrlObject) return NULL;

    // Load dynamic library
    if (!m_wxdll.Load(m_pCtrlObject->m_ifCANAL.m_strPath, wxDL_LAZY)) {
        ::wxGetApp().logMsg(_T("vscpd: Unable to load dynamic library."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // Now find methods in library

    // * * * * CANAL OPEN * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalOpen =
            (LPFNDLL_CANALOPEN) m_wxdll.GetSymbol(_T("CanalOpen")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalOpen."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL CLOSE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalClose =
            (LPFNDLL_CANALCLOSE) m_wxdll.GetSymbol(_T("CanalClose")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalClose."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GETLEVEL * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetLevel =
            (LPFNDLL_CANALGETLEVEL) m_wxdll.GetSymbol(_T("CanalGetLevel")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetLevel."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL SEND * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSend =
            (LPFNDLL_CANALSEND) m_wxdll.GetSymbol(_T("CanalSend")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalSend."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalDataAvailable =
            (LPFNDLL_CANALDATAAVAILABLE) m_wxdll.GetSymbol(_T("CanalDataAvailable")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalDataAvailable."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }


    // * * * * CANAL RECEIVE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalReceive =
            (LPFNDLL_CANALRECEIVE) m_wxdll.GetSymbol(_T("CanalReceive")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalReceive."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GET STATUS * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetStatus =
            (LPFNDLL_CANALGETSTATUS) m_wxdll.GetSymbol(_T("CanalGetStatus")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetStatus."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GET STATISTICS * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetStatistics =
            (LPFNDLL_CANALGETSTATISTICS) m_wxdll.GetSymbol(_T("CanalGetStatistics")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetStatistics."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL SET FILTER * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSetFilter =
            (LPFNDLL_CANALSETFILTER) m_wxdll.GetSymbol(_T("CanalSetFilter")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalSetFilter."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL SET MASK * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSetMask =
            (LPFNDLL_CANALSETMASK) m_wxdll.GetSymbol(_T("CanalSetMask")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalSetMask."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GET VERSION * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetVersion =
            (LPFNDLL_CANALGETVERSION) m_wxdll.GetSymbol(_T("CanalGetVersion")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetVersion."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetDllVersion =
            (LPFNDLL_CANALGETDLLVERSION) m_wxdll.GetSymbol(_T("CanalGetDllVersion")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetDllVersion."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetVendorString =
            (LPFNDLL_CANALGETVENDORSTRING) m_wxdll.GetSymbol(_T("CanalGetVendorString")))) {
        // Free the library
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetVendorString."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }


    wxLogNull logNo;


    // ******************************
    //     Generation 2 Methods
    // ******************************


    // * * * * CANAL BLOCKING SEND * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalBlockingSend =
            (LPFNDLL_CANALBLOCKINGSEND) m_wxdll.GetSymbol(_T("CanalBlockingSend")))) {
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver."),
                DAEMON_LOGMSG_CRITICAL);
        m_pCtrlObject->m_proc_CanalBlockingSend = NULL;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalBlockingReceive =
            (LPFNDLL_CANALBLOCKINGRECEIVE) m_wxdll.GetSymbol(_T("CanalBlockingReceive")))) {
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver."),
                DAEMON_LOGMSG_CRITICAL);
        m_pCtrlObject->m_proc_CanalBlockingReceive = NULL;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetdriverInfo =
            (LPFNDLL_CANALGETDRIVERINFO) m_wxdll.GetSymbol(_T("CanalGetDriverInfo")))) {
        ::wxGetApp().logMsg(_T("Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver."),
                DAEMON_LOGMSG_CRITICAL);
        m_pCtrlObject->m_proc_CanalGetdriverInfo = NULL;
    }

    //
    // =====================================================================================
    //

    // Open the device
    m_pCtrlObject->m_openHandle =
            m_pCtrlObject->m_proc_CanalOpen((const char *) m_pCtrlObject->m_ifCANAL.m_strConfig.mb_str(wxConvUTF8),
            m_pCtrlObject->m_ifCANAL.m_flags);



    // Check if the driver opened properly
    if (0 == m_pCtrlObject->m_openHandle) {
        ::wxGetApp().logMsg(_T("Driver failed to open."), DAEMON_LOGMSG_CRITICAL);
        return NULL;
    }

    // Get Driver Level
    m_pCtrlObject->m_driverLevel = m_pCtrlObject->m_proc_CanalGetLevel(m_pCtrlObject->m_openHandle);


    // If this is a TCPIP level driver then nothing more then open and close should be done without
    // the driver. No messages should be put on any queues
    if (CANAL_LEVEL_USES_TCPIP == m_pCtrlObject->m_driverLevel) {
        // Just sit and wait until the end
        while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {
            wxSleep(1);
        }
    } else {
        if (NULL != m_pCtrlObject->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            /////////////////////////////////////////////////////////////////////////////
            // Device write worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_pwriteThread = new deviceWriteThread;

            if (m_pwriteThread) {
                m_pwriteThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_pwriteThread->Create())) {
                    m_pwriteThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_pwriteThread->Run())) {
                        ::wxGetApp().logMsg(_("Unable to run device write worker thread."), DAEMON_LOGMSG_CRITICAL);
                    }
                } 
                else {
                    ::wxGetApp().logMsg(_("Unable to create device write worker thread."), DAEMON_LOGMSG_CRITICAL);
                }
            } 
            else {
                ::wxGetApp().logMsg(_("Unable to allocate memory for device write worker thread."), DAEMON_LOGMSG_CRITICAL);
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_preceiveThread = new deviceReceiveThread;

            if (m_preceiveThread) {
                m_preceiveThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_preceiveThread->Create())) {
                    m_preceiveThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_preceiveThread->Run())) {
                        ::wxGetApp().logMsg(_("Unable to run device receive worker thread."), DAEMON_LOGMSG_CRITICAL);
                    }
                } 
                else {
                    ::wxGetApp().logMsg(_("Unable to create device receive worker thread."), DAEMON_LOGMSG_CRITICAL);
                }
            } 
            else {
                ::wxGetApp().logMsg(_("Unable to allocate memory for device receive worker thread."), DAEMON_LOGMSG_CRITICAL);
            }

            // Just sit and wait until the end
            while (!m_pCtrlObject->m_bQuit) {
                wxSleep(1);
            }

            m_preceiveThread->m_bQuit = true;
            m_pwriteThread->m_bQuit = true;
            m_preceiveThread->Wait();
            m_pwriteThread->Wait();
        } else {

            // * * * * Non blocking version * * * *

            bool bActivity;
            while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {

                bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device								   				 //
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if (m_pCtrlObject->m_proc_CanalDataAvailable(m_pCtrlObject->m_openHandle)) {

                    m_pCtrlObject->m_proc_CanalReceive(m_pCtrlObject->m_openHandle, &msg);

                    bActivity = true;

                    vscpEvent *pEvent = new vscpEvent;
                    if (NULL != pEvent) {

                        // Convert CANAL message to VSCP event
                        convertCanalToEvent(pEvent,
                                &msg,
                                m_pCtrlObject->m_GUID);

                        VscpRXObj *pRecord = new VscpRXObj;
                        wxASSERT(NULL != pRecord);

                        pRecord->m_pEvent = pEvent;
                        pRecord->m_wxStrNote.Empty();
                        pRecord->m_time = wxDateTime::Now();

                        pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;

                        eventReceive.SetClientData(pRecord);
                        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventReceive);

                        //} 
                        //else {
                        //  delete pRecord;
                        //}

                    } // record exists

                }


                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //             Send messages (if any) in the outqueue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                if (m_pCtrlObject->m_outQueue.GetCount()) {

                    m_pCtrlObject->m_mutexOutQueue.Lock();

                    eventOutQueue::compatibility_iterator node =
                            m_pCtrlObject->m_outQueue.GetFirst();
                    vscpEvent *pEvent = node->GetData();

                    canalMsg canalMsg;
                    convertEventToCanal(&canalMsg, pEvent);
                    if (CANAL_ERROR_SUCCESS ==
                            m_pCtrlObject->m_proc_CanalSend(m_pCtrlObject->m_openHandle, &canalMsg)) {

                        VscpRXObj *pRecord = new VscpRXObj;
                        if (NULL != pRecord) {

                            pRecord->m_pEvent = pEvent;
                            pRecord->m_wxStrNote.Empty();
                            pRecord->m_time = wxDateTime::Now();

                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;

                            eventReceive.SetClientData(pRecord);
                            wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventReceive);
                        } // record 

                        // Remove the node
                        m_pCtrlObject->m_outQueue.DeleteNode(node);
                    } else {
                        // Give it another try
                        m_pCtrlObject->m_semOutQue.Post();
                    }

                    m_pCtrlObject->m_mutexOutQueue.Unlock();

                }

                if (!bActivity) {
                    ::wxMilliSleep(100);
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

    } // not TCIP Driver

    // Close CANAL channel
    m_pCtrlObject->m_proc_CanalClose(m_pCtrlObject->m_openHandle);

    // Library is unloaded in destructor

    if (NULL != m_preceiveThread) {
        m_preceiveThread->Wait();
        delete m_preceiveThread;
    }

    if (NULL != m_pwriteThread) {
        m_pwriteThread->Wait();
        delete m_pwriteThread;
    }

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceThread::OnExit()
{
    ;
}




// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// deviceReceiveThread
//

deviceReceiveThread::deviceReceiveThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceReceiveThread::~deviceReceiveThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceReceiveThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT,
            frmVSCPSession::ID_FRMVSCPSESSION);
    canalMsg msg;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking receive method must have been found
    if (NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive) return NULL;

    int rv;
    while (!TestDestroy() && !m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
                (rv = m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive(
                m_pMainThreadObj->m_pCtrlObject->m_openHandle, &msg, 500))) {

            vscpEvent *pEvent = new vscpEvent;
            if (NULL != pEvent) {

                // Convert CANAL message to VSCP event
                convertCanalToEvent(pEvent,
                        &msg,
                        m_pMainThreadObj->m_pCtrlObject->m_GUID);

                VscpRXObj *pRecord = new VscpRXObj;
                if (NULL != pRecord) {

                    pRecord->m_pEvent = pEvent;
                    pRecord->m_wxStrNote.Empty();
                    pRecord->m_time = wxDateTime::Now();

                    pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;

                    eventReceive.SetClientData(pRecord);
                    wxPostEvent(m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd,
                            eventReceive);

                }// record exists
                else {
                    delete pEvent;
                }

            }
        }
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceReceiveThread::OnExit()
{
    ;
}





// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// deviceWriteThread
//

deviceWriteThread::deviceWriteThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceWriteThread::~deviceWriteThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceWriteThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    eventOutQueue::compatibility_iterator node;
    vscpEvent *pEvent;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking send method must have been found
    if (NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend) return NULL;

    while (!TestDestroy() && !m_bQuit) {
        if (wxSEMA_TIMEOUT == m_pMainThreadObj->m_pCtrlObject->m_semOutQue.WaitTimeout(500)) continue;
        m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Lock();

        node = m_pMainThreadObj->m_pCtrlObject->m_outQueue.GetFirst();
        pEvent = node->GetData();

        canalMsg canalMsg;
        convertEventToCanal(&canalMsg, pEvent);
        if (CANAL_ERROR_SUCCESS ==
                m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend(m_pMainThreadObj->m_pCtrlObject->m_openHandle, &canalMsg, 300)) {

            VscpRXObj *pRecord = new VscpRXObj;
            if (NULL != pRecord) {

                pRecord->m_pEvent = pEvent;
                pRecord->m_wxStrNote.Empty();
                pRecord->m_time = wxDateTime::Now();

                pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;

                eventReceive.SetClientData(pRecord);
                wxPostEvent(m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd,
                        eventReceive);
            } // record 

            // Remove the node
            m_pMainThreadObj->m_pCtrlObject->m_outQueue.DeleteNode(node);

        } else {
            // Give it another try
            m_pMainThreadObj->m_pCtrlObject->m_semOutQue.Post();
        }

        m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Unlock();

    } // while

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceWriteThread::OnExit()
{
    ;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW_RCVINFO
//

void frmVSCPSession::OnHtmlwindowRcvinfoLinkClicked(wxHtmlLinkEvent& event)
{
    if (event.GetLinkInfo().GetHref().StartsWith(_("http://"))) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}

