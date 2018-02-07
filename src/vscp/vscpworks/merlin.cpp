/////////////////////////////////////////////////////////////////////////////
// Name:        merlin.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/03/2012 21:03:29
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
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "merlin.h"
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

#include "merlin.h"

////@begin XPM images
/* XPM */
static const char *pdf_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 326 2",
"KX c #CD2F28",
"YX c #C8281F",
"qo c #C5180E",
"a  c #D11A12",
"&X c #CF5B55",
"pX c #D5B0AD",
"-o c #C8251C",
"w. c #D4271F",
"WX c #CF5851",
"[  c #D22D25",
"i. c #D2655F",
"#. c #D49490",
"7. c #DCE0DF",
"0X c #D1140C",
"6  c #DD8682",
"P. c #D9D0CF",
"]X c #C9352D",
"_X c #D9CDCB",
"2. c #DFEAE9",
",o c #DFEAEA",
"V. c #D25F58",
"XX c #D06862",
"qX c #CD5852",
"(  c #DFE7E6",
"!X c #DFE7E7",
"jX c #D0655F",
"Y  c #D09D99",
"x  c #CB261D",
">  c #D72820",
"e. c #CD554E",
"-. c #D6B7B4",
"vX c #DADDDC",
"%. c #DFE1E0",
"s  c #D8736E",
"UX c #DADAD9",
",. c #D3342C",
"T  c #D0241B",
">X c #D2534C",
"H  c #D21B13",
"ho c #DB453F",
"}. c #D13A33",
"R. c #DDE4E3",
"QX c #DDE4E4",
"X. c #D05953",
"uX c #D71C14",
";. c #D5251D",
"&. c #DAD4D3",
"LX c #D39E9A",
"B  c #CE271E",
"6. c #D24D46",
"N  c #D01E16",
"TX c #CE5F59",
"A. c #D28580",
">o c #E0EEED",
"b. c #DAD1CF",
".. c #D71910",
"1o c #E0EEEE",
"]. c #D71911",
"!  c #D1342C",
"L. c #D16C66",
"z  c #D01B12",
"$  c #CE241B",
"4X c #D7160D",
"0  c #C7261D",
",  c #D32820",
"j  c #D7BEBC",
"^  c #D01810",
"4o c #D35D56",
"S  c #D3251D",
"2  c #C7231A",
"!. c #D12E27",
"V  c #D7BBB8",
":X c #D2443D",
"SX c #D4ABA7",
"2X c #D27C77",
".X c #D04D47",
"mX c #DEEEEE",
"z. c #D51911",
"2o c #D66761",
"hX c #D31F17",
"lo c #D7453F",
"|  c #D38F8A",
"xo c #D0473F",
"@X c #DEE8E6",
")  c #DEE8E7",
"}X c #D1251D",
"_  c #DEE8E8",
"E  c #D9716C",
"R  c #D38C88",
"4  c #DC7E79",
"%X c #D4A29F",
"'  c #D12219",
"h  c #DEE5E4",
"[X c #CF2B23",
"t  c #DEE5E5",
"p. c #D9DEDC",
"5X c #D81D15",
"(. c #D23830",
"%  c #CA241B",
"l  c #DEE2E1",
"fo c #D5453E",
"N. c #D38681",
"wo c #CC180E",
"iX c #D03E37",
"g  c #DEDFDE",
"6X c #CD2E26",
"l. c #D5B2AF",
"8  c #D03B33",
"=  c #C8271E",
";X c #D2322B",
"5o c #C5170D",
"S. c #D31007",
"M  c #D18985",
"[. c #D22F27",
"C. c #DCE2E1",
"eo c #D1160E",
"F  c #CF1F17",
"*  c #C1261D",
"5  c #DD8884",
" o c #DFEFEE",
"VX c #CE3E36",
"@. c #D61A12",
"@  c #D4231B",
"yX c #D1130A",
"/  c #D5A9A5",
"w  c #D76863",
"e  c #DFECEB",
"rX c #DFECEC",
"r. c #D6140B",
"{  c #D9CCCA",
"d  c #DFE9E8",
"*X c #D02F27",
"K  c #DFE9E9",
"X  c #C6241B",
"DX c #CF4E47",
"tX c #CD5750",
"1X c #D3746F",
"m  c #DFE6E5",
"*o c #DFE6E6",
"v  c #D41A12",
"#  c #D2231B",
"D. c #D2938F",
";o c #CD1C13",
"=o c #CB251C",
"I  c #D17A75",
"to c #D4170E",
"v. c #DFE3E2",
"#X c #DFE3E3",
"q. c #D25852",
"]  c #CE322B",
"*. c #D3362F",
"+o c #D0261D",
"Z. c #DFE0DF",
"co c #D8726C",
"W. c #DFE0E0",
"|X c #D8726D",
"   c None",
"J  c #D28D89",
"7o c #CE2C23",
"#o c #D21A12",
"aX c #DAD6D4",
"{. c #D13931",
"U. c #DAD6D5",
"u  c #CB1C13",
"zo c #D4463E",
"<  c #CE2920",
">. c #DAD3D1",
"<o c #E0F0F0",
"sX c #DDE0DF",
"<. c #D7180F",
"~  c #D08D89",
"k  c #D7C0BD",
"po c #C4180E",
"JX c #D01A11",
"/. c #D24942",
"%o c #D4B0AD",
"3o c #C7251C",
"K. c #D7150D",
"kX c #D4ADAA",
"B. c #D71209",
"`. c #D51B13",
"Q  c #D16560",
"p  c #CC261D",
"F. c #DEEDEC",
"-X c #D32119",
"^. c #D04942",
"ZX c #D4A7A3",
"_. c #D31E15",
"eX c #D9736E",
"zX c #CD716C",
"dX c #DEE7E6",
"9X c #D1241C",
"u. c #D31B13",
"W  c #DEE7E7",
":  c #D62820",
"jo c #DC453F",
"{X c #D8C7C5",
"U  c #D12119",
"k. c #E1F1F1",
"'. c #D81910",
"f  c #DEDEDD",
"O  c #CF241B",
"cX c #CD2D25",
"9. c #DEDEDE",
"~. c #D61F17",
";  c #D42820",
"ko c #DA453F",
".o c #C9746E",
"RX c #D3B7B4",
"=X c #D61C13",
"j. c #D5AEAB",
"s. c #D4251C",
"P  c #D4251D",
"|. c #D03730",
"~X c #DCE1DF",
"M. c #DFEEED",
"PX c #DFEEEE",
"HX c #CF1B12",
"NX c #D11209",
"T. c #DCDEDD",
":o c #D76761",
"(X c #D1827D",
"&o c #D9CECC",
"y. c #DFEBEA",
"+X c #DFEBEB",
"). c #D22820",
"i  c #C6261D",
"go c #D8453F",
"'X c #C42F27",
"bX c #DA746F",
"do c #D1473F",
"oo c #C9332A",
"Q. c #D6BEBC",
"3. c #D5A5A1",
"n  c #DFE8E7",
"9o c #D25D56",
"E. c #DFE8E8",
"4. c #D41C14",
"L  c #D2251D",
"9  c #C6231A",
"OX c #DCD8D7",
"m. c #D1443D",
"a. c #D33B34",
"C  c #DFE5E4",
"O. c #DFE5E5",
" . c #D41911",
"8X c #D25A54",
"D  c #D6B8B5",
"^X c #D6B8B6",
"o  c #CB241B",
"xX c #D4160D",
":. c #DFE2E1",
"}  c #DFE2E2",
"7X c #D0605A",
"Oo c #D6B5B2",
"1. c #D59C98",
"wX c #DDE8E7",
"c  c #D0251D",
"b  c #D8716C",
"5. c #D9170E",
"0o c #C73027",
"3  c #D13B33",
"7  c #DB7E79",
"CX c #D21910",
"o. c #DDE5E4",
"0. c #DDE5E5",
"I. c #DAD5D4",
"EX c #D01F16",
"$X c #DDE2E1",
"yo c #D2160E",
".  c #C2261D",
"MX c #CE605A",
"io c #CB180E",
"AX c #E0EFEF",
"G. c #DAD2D1",
"d. c #D7C2C0",
"ao c #C53027",
"1  c #C7271E",
",X c #D3615B",
")X c #D39995",
"8o c #C4170D",
"$o c #DDDCDB",
"FX c #D01911",
"J. c #CF3830",
"=. c #D51D15",
"oX c #D39693",
"uo c #D0160E",
"x. c #D2453E",
"n. c #D12C24",
"$. c #D51A12",
"+  c #D3231B",
"G  c #D12C25",
"@o c #CC251C",
"GX c #D0130A",
"y  c #D66863",
"/X c #D3908B",
"IX c #D97570",
"fX c #CF6A64",
"lX c #D5BFBC",
"t. c #D3908C",
"`X c #DBDCDB",
"+. c #D7B6B4",
"so c #D9726C",
"f. c #DEE9E9",
"&  c #C5241B",
"nX c #D9726D",
"Y. c #DBD9D7",
"Xo c #C2140A",
"c. c #DBD9D8",
"<X c #D46B66",
"g. c #D2746F",
"6o c #CF2C23",
"r  c #DEE6E5",
"Z  c #D31A12",
"H. c #D5B9B7",
"q  c #CC1C13",
"3X c #D0423B",
"`  c #D2716B",
"ro c #D3170E",
"-  c #CF2920",
"BX c #CF615B",
" X c #D03F38",
"h. c #D3140B",
"8. c #DEE0DF",
"A  c #CF261D",
"gX c #D11D15",
/* pixels */
"    . X o O + @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ # $ % & *     ",
"  . = - ; : : : : > > > > > > > > > > > > > : : : : : , < 1 *   ",
". 2 3 4 5 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 5 7 8 9 * ",
"0 q w e r t t t t t t t r e e t t t t t t t t t t t t r e y u i ",
"p a s d f f f f f f f g h j k l f f f f f f f f f f f f d s z x ",
"c v b n f f f f f f f m M N B V C f f f f f f f f f f f n b Z A ",
"S v b n f f f f f f C D F G H J K f f f f f f f f f f f n b v L ",
"P Z b n f f f f f f d I U Y T R K f f f f f f f f f f f n b Z P ",
"P Z E n f f f f f f W Q ! ~ ^ / ( f f f f f f f f f f f n b Z P ",
"P v E ) f f f f f f _ ` ' ] [ { } f f f f f f f f f f f n E v P ",
"P v E ) f f f f f f K |  ...X.o.g f f f f f f f f f f f ) E v P ",
"P v E ) f f f f f f O.+.+ @.#.K f f f f f f f f f f f f ) E v P ",
"P $.E ) f f f f f f %.&.*.=.-.m f f f f f f f f f f f f ) E v P ",
";.$.E ) f f f f f f :.>.,.<.1.2.f f f f f f f f f f f f ) E $.P ",
";.$.E ) f f f f f f n 3.4.5.6.7.8.f f f f f f f f f f f ) E $.;.",
";.$.E ) f f f f f 9.0.q.w.e.r.t.y.f f f f f f f f f f f ) E $.;.",
";.$.E ) f f f f f C j u.i.p.a.s.d.( f f f f f f f f f f ) E $.;.",
";.$.E ) f f f f f f.g.h.j.k.l.z.x.c.v.f f f f f f f f f ) E $.;.",
";.$.E ) f f f f } b.n.m.7.:.M.N.B.V.C.C %.%.Z.8.g f f f ) E $.P ",
";.$.E ) f f f 9.e A.S.D.F.c.G.H.J.K.L.P.I.U.Y.T.R.) E.W.) E $.P ",
"P $.E ) f g n _ Q.!.~.^./.(.)._.`.'.].[.{.}.|. X.XXXoXOX+XE v P ",
"P v E @X#X$X%X&X*X=X-X;X:X>X,X<X1X2X3X4X5X6X7X8Xm.9X0XqXwXeXv P ",
"P v E rXb.tXyX .uXiXpXaXsXo.dXW _ e &.fXgXhXjXkXlXzXxXcXvXbXv P ",
"P v nXmXMXNXBXVXCXZXAX%.g f f f f f v.dXSXDXFXGXHXJXKXLXPXb Z P ",
"P Z IXUXYXTXRXEXWXQXg f f f f f f f f g !X~X^X/X(X)X_X( n b Z P ",
"P v IX`X'X]X[XcX{XC f f f f f f f f f f f g C d 2.E.:.f n b v S ",
"}Xv |X o.oXoooOon f f f f f f f f f f f f f f f f f f f n b Z +o",
"@o#os y.$o%o&o*of f f f f f f f f f f f f f f f f f f f d s a =o",
"-o;o:o>o,o<orX) n ) ) ) ) ) ) ) ) ) ) ) ) ) ) n n n n d 1o2oq 3o",
"4o5o6o:os b b b E E E E E E E E E E E E E E E E b b b s 2o7o8o9o",
"  0oqowoeororororororototototototototototorororororoyouoiopoao  ",
"    sodofogohohohohohohohohojojojojohohohohohohohokolozoxoco    "
};

/* XPM */
static const char *help_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 15 50 1",
"j c #4E7FD0",
"8 c #EDF2FB",
", c #7198D9",
"e c #DCE6F6",
"y c #FFFFFF",
"5 c #95B2E3",
"$ c #9AB6E4",
"g c #EAF0FA",
"1 c #2D59A3",
"@ c #B1C7EB",
"> c #6992D7",
"a c #D9E4F5",
"r c #356AC1",
"6 c #9BB7E5",
"= c #F7F9FD",
"+ c #BED0EE",
"z c #F0F5FC",
"f c #ADC4E9",
"# c #A8C0E8",
"7 c #CBD9F1",
"u c #366BC2",
"  c None",
"c c #FDFEFF",
"w c #274D8D",
"t c #C4D5F0",
"% c #7CA0DC",
"h c #E2EAF8",
"p c #487BCE",
"o c #4377CD",
"4 c #2A549A",
"< c #254A87",
"O c #CCDAF2",
"& c #89A9DF",
"9 c #2B559B",
"* c #D2DFF4",
". c #3366BB",
": c #2E5CA8",
"x c #FAFCFE",
"l c #F5F8FD",
"2 c #799EDB",
"d c #DFE8F7",
"; c #A6BFE8",
"3 c #638ED5",
"- c #5282D0",
"X c #2A5398",
"0 c #B8CCEC",
"s c #376EC9",
"q c #2D5AA5",
"i c #285092",
"k c #8CACE0",
/* pixels */
"   .......Xo    ",
"   .O+@#$%.&o   ",
"   .*O+@#$.=&-  ",
"  ;:::>#@#.==&: ",
" ,<1234<>@....: ",
"5<X67869<&0#$&. ",
"q99wwe;9Xrt0#$. ",
"<99X&yu99iOt0#. ",
"<99wyp999<aOt0. ",
"<99X<u99XsdaOt. ",
",w9<y;99<fghaO. ",
" j<X<XX<klzgha. ",
"  5:::j7x=lzgh. ",
"   .yyyccx=lzg. ",
"   ............ "
};

/* XPM */
static const char *lock_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 507 2",
"N  c #DDDDDC",
"7+ c #F6BB26",
"/o c #F7DB87",
"TO c #F0951F",
"^o c #F7DB8B",
"]O c #ED9420",
";+ c #ED9421",
"NO c #EA9927",
":  c #888686",
"s+ c #F0A432",
"Ko c #F3B738",
"JO c #F9CE40",
"RO c #F09B31",
"!X c #F7E4A2",
"@X c #F7DEA2",
"zO c #F9D14A",
"UX c #F7E4A6",
"d  c #9D9C9B",
"fo c #F3C34A",
"dX c #F7E1AC",
"Po c #F3B74A",
"go c #F3C04D",
"{X c #B18641",
"k+ c #F6C14F",
":o c #F3C651",
"fO c #F9D157",
"DO c #F9D158",
">o c #F3C354",
"j+ c #F6C155",
"7X c #F7EAB8",
">  c #AFAEAD",
"T  c #AFAEAE",
"(X c #F3C95A",
"g  c #B2B2B1",
")X c #F3C65D",
"_X c #F3C35E",
"BX c #F3CC62",
"Do c #F9D768",
"so c #F6D369",
"M  c #C1C0BF",
"6  c #C1C0C0",
",X c #FAEECA",
"pO c #F6C469",
"CX c #F3C96B",
"2. c #F1E2C9",
"4  c #C4C4C4",
"So c #F9DD72",
"(. c #FAEECF",
"2+ c #F6C76E",
"). c #FAEED0",
"B  c #CAC9C9",
"6O c #F5B617",
"PO c #F5B618",
"(o c #F6D979",
"0+ c #F5B319",
":O c #F9DA7C",
"[X c #545863",
".+ c #F8C321",
"&+ c #F2AC1B",
"gO c #F8C624",
"FO c #F8C625",
"-o c #F6D983",
"K  c #DFDFDF",
"H. c #FDF5E3",
"_O c #EF9F1E",
"Mo c #F6D685",
"IO c #EF9F1F",
"Bo c #F6DC88",
"A. c #FDF8E8",
"wo c #F6DC8B",
"^X c #F6DC8C",
"S. c #FDF8EA",
"qo c #F6DC8E",
"$o c #F5B62E",
"w. c #FDFBEF",
"yo c #F5B630",
"KO c #F8C937",
"`o c #F5B631",
"#o c #F2B231",
"t. c #FDFEF6",
"JX c #F6D997",
"xO c #F8CC42",
"5X c #F2B53C",
"K. c #F2B53D",
"0  c #969594",
"<  c #969595",
"#O c #E0A03D",
"aX c #F6DCA4",
">. c #F9DAA6",
",. c #F9DAA7",
"}O c #FBD955",
"f. c #F2B84B",
"!O c #FBD956",
"d. c #F9DDAE",
"%X c #F9E3B0",
"GO c #FBD958",
"kO c #FBD95A",
"jo c #F2BB52",
"`X c #FBC458",
"r  c #B1B0B0",
"Co c #F5D160",
"lX c #F9ECBE",
"u  c #B4B4B3",
"Q. c #F9E9BF",
";X c #F9E9C0",
"iO c #F5BC5F",
"~. c #F9E9C3",
"l  c #BDBDBC",
"Eo c #F2BE65",
"co c #F2BE66",
"b  c #C3C2C2",
"vo c #F2C16A",
"ao c #F8D873",
"^  c #C9CACC",
"3o c #807462",
"%+ c #F4B117",
"5+ c #FBDC7B",
"o+ c #F4B118",
"@+ c #F7C41F",
"0. c #FCF6DB",
"u+ c #EE9717",
"eO c #77746E",
"mo c #F5D17F",
"eo c #F5DA82",
"   c None",
"9O c #F1A41D",
"F  c #DEDDDD",
"i+ c #EE971A",
"vO c #F1A41E",
"7. c #F9F8E4",
"++ c #FACB2C",
"`O c #EE9A1E",
"=+ c #EE9A1F",
"0o c #F5D788",
"mO c #DF921F",
"Yo c #958A7A",
"LO c #F7C12D",
"I  c #E7E6E6",
"5. c #FBD088",
"yX c #F8D58A",
"Oo c #F5DD8D",
"oO c #F4A526",
"L. c #F5DD91",
"HX c #F5D48F",
"|O c #FAD139",
"cO c #F7C435",
";  c #838181",
"%  c #8F8E8E",
"Io c #EBA839",
"1o c #E2A53C",
"Go c #F7CA4B",
"e  c #A1A09F",
"J  c #A1A0A0",
")  c #989A9F",
"xo c #F7B549",
"lO c #FAD755",
"g. c #F8E7B1",
"}  c #B0ABA6",
"X. c #B6B0A8",
"=X c #F8E4B7",
"'o c #FADA5F",
"uO c #F4B455",
"W. c #F8E7BB",
"q  c #B3B2B1",
"<+ c #F4B458",
"Y. c #F8EDC6",
"iX c #EEBE66",
"&O c #F4C068",
"5  c #C5C4C4",
"<O c #F3AC0F",
"k. c #F8F0D0",
"c. c #FBF1D3",
" O c #F3AF15",
"_. c #FBF1D5",
"b. c #FBF1D6",
"q+ c #F3AC16",
"7o c #F4C976",
"n. c #FBF1D8",
"lo c #97866A",
"'. c #FBF4DC",
"Z  c #D7D6D6",
"-O c #F7D37E",
"YO c #F09619",
"'O c #ED951B",
"bO c #F09F1E",
".o c #F4D284",
"-+ c #ED951C",
" + c #F9C92C",
"^O c #F9CC36",
"a+ c #F09F2B",
"y. c #FEFEF8",
"YX c #F7E29D",
"OX c #F7DF9E",
"cX c #F3B53B",
"tO c #F6A438",
"LX c #F7DFA1",
"EO c #F0A23B",
"PX c #F7E2A6",
"f+ c #F3AF41",
">+ c #F0A23D",
"#X c #F7DFA6",
"IX c #F7E5A9",
" . c #BBB09F",
"$X c #F7DFA9",
"g+ c #F3B547",
"+. c #F1DAA9",
"'X c #B1873F",
"bX c #F7E8B0",
"s  c #A6A6A5",
"8X c #F7E8B3",
"9X c #F7E8B4",
"XX c #F7E8B7",
"S  c #ACABAB",
"[o c #F9D55E",
" X c #F7EBBF",
"2  c #B5B4B4",
"Ro c #F3BB60",
"E  c #BEBDBC",
"P. c #F7EEC8",
">X c #FAECC7",
"/. c #FAECCA",
"AO c #F6C569",
"2X c #FAF2D1",
"W  c #CACAC9",
"Ho c #F5B719",
".O c #F2AA18",
"Vo c #F6DA7D",
"|X c #F3CA78",
"t+ c #EF9D19",
"y+ c #EF9A19",
"8+ c #F5B423",
"nO c #E9981C",
",O c #F8C72A",
"&o c #F9E188",
"vX c #F6DD87",
"A  c #E2E1E1",
"_o c #F2B025",
"6X c #F6DD8B",
"D. c #FDF9EB",
"GX c #F9D58C",
"o  c #81807F",
"e. c #FDFCF2",
"jO c #F5B732",
"|  c #A29786",
"QO c #F5B733",
"{O c #F5B734",
"qO c #CE8F2C",
"pX c #F9D896",
"=  c #908E8E",
"d+ c #F2AA3A",
"&  c #939292",
"&. c #F9D8A0",
"<. c #FCD9A1",
"*. c #F9D8A1",
"Lo c #F2B642",
"=. c #F9D8A2",
"sX c #F6DDA5",
"WO c #F2AA46",
"VO c #F2AA47",
"ho c #F2BC4E",
"h+ c #F5BA4E",
"DX c #6B593E",
"SX c #6B5940",
"&X c #F9E4B4",
"R. c #F9E7B6",
"1O c #FBDA5D",
",o c #F2BF56",
"jX c #F9EABD",
"1. c #F6E0BB",
"%O c #F2B95D",
":X c #F9EAC2",
"_  c #B1B4B9",
"ro c #F5D267",
"VX c #F2CB67",
"1+ c #F5BD63",
"}X c #FBC86A",
"5o c #F5C36A",
"P  c #C9C8C7",
"l. c #F9F0D0",
"p  c #C9C8C8",
"*O c #F5C66F",
"6o c #F2C56F",
"ZX c #F8CD72",
"aO c #F8CD73",
"w+ c #F1A815",
"=O c #F5CC77",
"3+ c #F8D079",
"x  c #D5D5D4",
"no c #F5CC78",
"e+ c #F1A517",
"wX c #F5D57C",
"N. c #FCF4DE",
"!  c #DBDADA",
"(O c #F7BF24",
"+o c #F5DB85",
"{. c #FCF7E3",
"*+ c #F1A51F",
"XO c #F1A520",
"C. c #FCF7E5",
"Z. c #FCF7E6",
"*o c #F8DF8C",
"rX c #F5D589",
"q. c #FFFBEB",
"O  c #807E7D",
"+O c #838282",
"l+ c #F4B533",
"-  c #898787",
"}o c #F7C845",
"do c #F4C444",
"BO c #F1A23A",
"HO c #FAD54C",
"4o c #E2A642",
"zX c #F8E5AC",
"gX c #F8E5B4",
"6+ c #FAD55C",
"hX c #F8E8B8",
"G  c #B0AFAE",
"Ao c #FADB61",
"4X c #F4BB57",
"E. c #F8E8BB",
"i  c #B3B3B2",
"c  c #B3B3B3",
"uo c #FADB63",
"%o c #FADB64",
"NX c #F4CD60",
"D  c #B9B8B7",
"w  c #B9B8B8",
"]o c #FADB68",
"(  c #B9B8B9",
"v  c #BFBDBD",
"@o c #F4D36B",
"I. c #F8EEC9",
"U. c #F8EECC",
"x. c #FBEFCF",
"qX c #F4D372",
"z  c #CBCAC9",
"dO c #FAD874",
"Wo c #F4C46E",
"C  c #CBCACA",
"SO c #FAD875",
"bo c #F4C772",
"h. c #F8F1D5",
"7O c #F3B017",
"X+ c #F6BD1D",
"`. c #FBF2D9",
"m. c #FBF2DA",
"]X c #555964",
"R  c #D7D7D7",
"sO c #FAD87F",
"UO c #F09A1E",
"H  c #E3E4E3",
"Jo c #F3B628",
"Xo c #F4D688",
"Uo c #948C80",
"+  c #82807F",
"=o c #F7E095",
"G. c #FEFCF3",
"OO c #DE9A2F",
"6. c #F7D797",
"$  c #8B8989",
"oX c #F7E09B",
"u. c #FEFFFA",
"i. c #FEFFFB",
"p. c #FEFFFE",
"~X c #F7E3A1",
"*  c #949292",
"+X c #F7E0A2",
"QX c #F7E6A4",
"o. c #A9A298",
"To c #EDAB42",
"nX c #F7E6AA",
"mX c #F7E6AB",
"@. c #EBD6A8",
"3O c #F9D354",
"fX c #F7E3AF",
"$. c #F1D8AC",
"xX c #F3B94D",
")o c #F6CF55",
",  c #A9A8A8",
".X c #F7E9BA",
"h  c #B2B1B1",
"a  c #B2B1B2",
">O c #F9D361",
"n  c #B5B5B4",
"3X c #FAEAC1",
"~  c #BBBAB9",
"<X c #FAF0CD",
"WX c #F9DC72",
"j  c #C7C7C6",
"po c #F9D972",
"1X c #FAF0CF",
"'  c #C7C7C8",
"|o c #F5B515",
"9. c #FAF3D6",
"2o c #7E7464",
"$+ c #F5B819",
"io c #F9DF7B",
"8. c #FAF3D7",
"8O c #F2AB1A",
"9+ c #F5B821",
"0O c #F5A019",
" o c #F3CE7F",
"wO c #787570",
"~o c #F6D583",
"/O c #F8C52B",
"No c #F6DB8C",
"p+ c #EF9B24",
"Zo c #F2B12B",
"|. c #F6DE8F",
"F. c #FDFAED",
"to c #F2B12E",
"O+ c #FBD53B",
"MX c #F6DE96",
"r. c #FDFDF5",
"X  c #878685",
"[O c #EF9B30",
"KX c #F6DB9A",
"%. c #FCD798",
":+ c #EF9B31",
"RX c #F2B439",
"rO c #CE9032",
"ko c #F5B23E",
"-. c #F9D9A3",
";. c #F9D9A4",
":. c #F9D9A5",
"4O c #F8CE4E",
"@  c #A2A1A0",
"1  c #A2A1A1",
"yO c #F2AB48",
",+ c #F2AB4A",
"T. c #F9E8B5",
"FX c #DAA64D",
"{o c #F8D159",
"$O c #F5B554",
"kX c #F9EBBE",
"k  c #B4B3B3",
"Fo c #F8D463",
"{  c #C3BEB7",
"!. c #F9E8C0",
"}. c #F5BE5F",
"ZO c #F5BB5F",
".. c #C6C2BC",
"V  c #BDBCBC",
"^. c #F9EBC7",
"3. c #EDDEC4",
"3  c #C0C0C0",
"U  c #C6C5C5",
"f  c #CFCECE",
"Qo c #F5CA75",
"/  c #D2D2D3",
"!o c #F5D07C",
"Q  c #D8D7D7",
"TX c #F5D980",
"]. c #FCF5DE",
"[. c #FCF5DF",
"B. c #FCF5E1",
"9o c #F5D382",
"V. c #FCF5E2",
"4+ c #FBDB85",
";O c #F8DA86",
"eX c #F5D687",
"MO c #DF9122",
"oo c #F5D98D",
"@O c #83807D",
"tX c #F5D690",
"EX c #F4B631",
"5O c #F7C63D",
"~O c #FAD342",
"#  c #8F8D8D",
"9  c #929190",
"s. c #FFFFFE",
"0X c #F5E2A0",
".  c #929191",
"a. c #FFFFFF",
"Y  c #9B9A99",
"AX c #D9A446",
"#. c #E9D5AC",
"8  c #A4A3A3",
";o c #F4C851",
"*X c #F8E3B3",
"/X c #F4CB59",
"2O c #FAD95F",
"CO c #F4B354",
"<o c #F4BF58",
"-X c #F8E6BA",
"]  c #B0B0B2",
"7  c #B6B5B5",
"L  c #B9B9B8",
"4. c #EFDDBF",
"[  c #CBC5BC",
"uX c #EEBD63",
"J. c #F4BF64",
"y  c #C2C2C1",
"hO c #F3AB0D",
"z. c #FBF0CE",
"t  c #C8C7C7",
"j. c #F8EFD0",
"zo c #9D8765",
"v. c #FBF0D3",
"#+ c #F6BE1B",
")O c #F3B118",
"O. c #F7C975",
"`  c #D1D0D1",
"m  c #D4D4D3",
"M. c #FBF3DC",
"8o c #F4CE7C",
"r+ c #F0A119",
/* pixels */
"                          . X o O + X                           ",
"                      @ # $ % & * = - ; :                       ",
"                  > , < 1 2 3 4 5 6 7 8 < 9 0                   ",
"                q w e r t y u     i y p a 8 s d                 ",
"                f g h 5                 j k 2 l                 ",
"              z x c v                     b n m M               ",
"              N B 7                         V C Z               ",
"            7 A 5 S                         D t F G             ",
"            y H 5 J                         r f K L             ",
"            P I U Y                         T R A E             ",
"            W A b Y                         h Q ! ~             ",
"            ^ / ( )                         _ ` ' ]             ",
"            [ { } |                          ...X.o.            ",
"          O.+.@.#.$.%.&.&.*.=.-.;.:.>.>.,.<.1.2.3.4.5.          ",
"        6.7.8.9.0.q.w.e.e.r.t.y.u.i.p.p.a.a.s.s.a.a.a.d.        ",
"      f.g.h.j.k.l.z.x.c.v.b.n.m.M.N.B.V.C.Z.A.S.D.F.G.H.J.      ",
"      K.L.P.I.U.Y.T.R.E.W.Q.!.~.^./.(.)._.b.`.'.].[.{.v.}.      ",
"      K.|. X.X XXXoXOX+X@X#X$X%X&X*X=X-X;X:X>X,X<X1X2X3X4X      ",
"      5X6X7X8X9X0XqXwXeXrXtXyXuXiXpXaXsXdXfXgXhXjXkXlXzXxX      ",
"      cXvXbXnXmXMXNXBXVXCXZXAXSXDXFXGXHXJXKXLXPXIXUXYXTXRX      ",
"      EXWXQX!X~X^X/X(X)X_X`X'X]X[X{X}X|X o.oXoooL.Oo+o@o#o      ",
"      $o%o&o*o=o-o;o:o>o,o<o1o2o3o4o5o6o7o8o9o0oqowoeoroto      ",
"      youoiopoaosodofogohojokolozoxocovobonomoMoNoBoVoCoZo      ",
"      yoAoSoDoFoGoHoJoKoLoPoIoUoYoToRoEoWoQo!o~o^o/o(o)o_o      ",
"      `o'o]o[o{o}o|o O.OXOoOOO+O@O#O$O%O&O*O=O-O;O:O>O,O<O      ",
"      `o1O2O3O4O5O6O7O8O9O0OqOwOeOrOtOyOuOiOpOaOsOdOfOgOhO      ",
"      jOkOlOzOxOcO6O7O8OvObOnOmOMONOBOVOCOZOAOaOsOSODOFOhO      ",
"      jOGOHOJOKOLOPO7O8OvOIOUOYOTOROEOWOCOZOAOaOsOSODOFOhO      ",
"      QO!O~O^O/O(OPO)O8OvO_O`O'O]O[OEOWOCOZOAOaOsOSODOFOhO      ",
"      {O}O|O +.+X+POo+8OvO_O`O'O]O[OEOWOCOZOAOaOsOSODOFOhO      ",
"        5OO+++@+#+$+%+&+*+IO=+-+;+:+>+,+<+1+2+3+4+5+6+7+        ",
"          8+9+0+7Oq+w+e+r+t+y+u+i+p+a+s+d+f+g+h+j+k+l+          "
};

/* XPM */
static const char *page_next_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 384 2",
"so c #D4DADF",
"@  c #DADFE1",
"g  c #DDE0E3",
"F. c #E9EDE9",
";. c #ECE8E9",
"wX c #82C995",
"E  c #E6E9EC",
"2X c #91CE97",
"R  c #E9EDEE",
"/. c #129C40",
">. c #ECEEEF",
"M  c #ECEEF0",
"-O c #F2F3F2",
"]X c #78C144",
"J. c #EFF2F3",
"'  c #F2F0F3",
"0  c #F2F3F4",
"TX c #ECE8F2",
"5. c #0F9243",
"K. c #F2F3F5",
"vX c #F2EDF4",
",  c #F5F7F8",
"!  c #FBFCFC",
"eX c #5AB752",
"oX c #69BC56",
"-X c #4EB65A",
".X c #54B55C",
"~. c #5AB75D",
"To c #99CF68",
"_o c #9FD16A",
"%o c #7ABD14",
"[o c #7DBE16",
"mo c #7ABD1C",
"_X c #7EC67C",
")o c #E2F2D6",
"{X c #BEC5CB",
"yX c #CDE5D5",
"S  c #CDD3D7",
"l  c #D3D8DB",
"2o c #D0D7DB",
"3X c #6EBC33",
"gX c #80C234",
"|  c #E5E7E5",
"v  c #DCE1E4",
"FX c #77BF39",
"Xo c #DCE1E5",
" X c #2CA63A",
"7  c #E2E6E8",
"c  c #E2E6E9",
"b. c #08943B",
"ko c #7AC03D",
"r  c #E5E7EA",
"t. c #05903B",
"s  c #E8EBED",
".O c #F4F8F2",
"7X c #A5D69A",
"<o c #EEF0F0",
"^  c #EBECEF",
"d  c #EEF0F1",
" . c #F1EBF0",
"[. c #EEF0F2",
"XO c #F4F5F4",
"2. c #F4ECF1",
"U  c #F4F5F6",
"#X c #FAFDF9",
"E. c #FDFEFB",
"eo c #FDFEFC",
"a  c #FAFAFB",
")X c #F4FBFD",
";  c #FDFEFE",
"qX c #FAFDFE",
"y. c #26A156",
"A. c #3EAF61",
"p. c #3EAF63",
"+O c #A7D571",
"qo c #7FC01D",
"Ko c #82C121",
"yo c #85C223",
"9o c #85C225",
"BX c #8FCD7F",
"bX c #C0C7CD",
"Wo c #C3CBD0",
"Qo c #C3CBD1",
"w. c #D8E7DC",
"   c None",
"go c #D2D9DE",
"V. c #71C38E",
"X  c #DBDFE2",
"4o c #D8DEE2",
"OO c #B9DE91",
"LX c #DEE3E6",
"j  c #E1E4E6",
"k  c #E7E9EB",
"W  c #E4E8EB",
"l. c #E7E9EC",
",X c #EAEDEE",
"K  c #EAEDEF",
"{. c #EDF1F2",
"}. c #F0ECF1",
"`  c #F0F2F3",
"Y  c #F0F2F4",
"c. c #F3EDF3",
"1. c #F3F3F5",
"6  c #F6F7F8",
"HX c #F3EDF5",
"q  c #FCFCFC",
"r. c #8FCAA3",
"S. c #1C9F4D",
"&X c #58B64F",
"T. c #5EB856",
"8. c #28A056",
"n. c #2EA85A",
",o c #88C963",
"~X c #A6D363",
";o c #72BA18",
"CX c #B5DB77",
"hX c #B2DA78",
"po c #BCC4C9",
"Eo c #BFC8CD",
":X c #C2C9CE",
"X. c #C8CED3",
"V  c #CED3D7",
".  c #D4D8DC",
"VX c #81C234",
"do c #D4DBDF",
"+o c #72BD35",
"O  c #DDE1E3",
" o c #DAE0E3",
"xX c #72BD39",
"|X c #DAE0E4",
"@X c #A3D593",
"Go c #81C23D",
"Ro c #EFF6EE",
"8X c #A3D596",
"(  c #E3E6EA",
"L. c #ECECEC",
"aX c #E6EAEC",
"A  c #E6EAED",
"lX c #85CB9A",
"'. c #ECEFF0",
"p  c #EFF0F2",
"=  c #F2F4F5",
"~  c #F8F9F9",
"*O c #F5F2F8",
"$O c #C4E3A3",
"w  c #F8F9FA",
"$X c #F8FCFC",
"=o c #F2FAFC",
"*o c #FEFEFD",
"t  c #FEFEFE",
"!. c #4EB14F",
"rX c #8AC857",
"%. c #2D9D58",
"M. c #45B15C",
"Z. c #48B25E",
"m. c #3FAF60",
"^. c #4BB361",
"D. c #36A960",
"Fo c #D3EBC5",
"GX c #C7E4C5",
"$. c #48A96D",
"]o c #7DBF1B",
"Mo c #99D073",
"{o c #80C01C",
"Ho c #7DBF1C",
"lo c #80C01D",
"'o c #7DBF1E",
"&o c #B1D878",
"zo c #83C123",
"R. c #8ACB7F",
"=. c #5DB37D",
"!o c #C4CBD1",
"9  c #CAD0D4",
"SX c #84C988",
"ao c #CDD4D9",
"@. c #DCE5DE",
"}X c #D3D9DD",
"{  c #DFE6E1",
"%X c #7BC68E",
" O c #B7DD92",
"IX c #DFE3E7",
"tX c #65BA3E",
"pX c #E5E8EB",
"N  c #E8ECED",
"/  c #E8E9ED",
"B  c #E8ECEE",
"G  c #EBEDEF",
"F  c #EEF1F2",
"6o c #EEEEF2",
"=O c #F4F3F5",
"]  c #F4EDF3",
"y  c #FAFBFB",
":  c #FAFBFC",
"9. c #1D994E",
"h. c #239E51",
"XX c #5FB854",
"+X c #71BE58",
"!X c #A4D25C",
"f. c #35AA61",
"*. c #35A15F",
"4X c #80C668",
"0X c #C9E6C2",
"-o c #77C374",
"#O c #ADD87A",
"@o c #85C323",
"/o c #C0C8CD",
"xo c #85C327",
"%O c #E7F3DE",
"Q  c #CCD2D7",
"&  c #D2D7DA",
"<  c #D8DCDF",
"%  c #D8DCE0",
"fo c #D5DBE0",
".. c #DBD7DF",
"uX c #DBE0E3",
"`. c #E1E5E7",
";X c #E7EAE9",
"mX c #E1E5E8",
"'X c #9ACD3F",
"i  c #E7EAEB",
"L  c #E7EAEC",
"sX c #E7EAED",
"x  c #EDEFF1",
"C  c #F3F4F5",
"n  c #F3F4F6",
";O c #F6F5F9",
">  c #F9F9FA",
"4  c #FCFDFD",
"6. c #229C52",
"&. c #289B54",
"U. c #4FB460",
"-. c #ADD5BA",
"cX c #C2E2C5",
"oO c #DAEDC8",
"ro c #72BB18",
"(X c #DDEECA",
"#o c #7EBF1A",
"$o c #7EBF1B",
"8o c #78BD1B",
"Io c #75BC1E",
"Oo c #D1E7CD",
"}o c #7EBF20",
"uo c #72BB21",
"jo c #DDEED8",
"G. c #C5CBCF",
"^o c #C2CAD0",
"a. c #5EBC81",
"j. c #D4E8D9",
"Vo c #CBD3D8",
"9X c #97CF88",
"Co c #D1D8DD",
"#  c #DADEE1",
"3o c #D7DDE1",
"YX c #DDE2E5",
"MX c #E3E7EA",
"ho c #E0E0E9",
"[  c #EFEBED",
"e. c #E9ECEE",
"RX c #A9D89B",
"8  c #EFF1F2",
"m  c #EFF1F3",
"Do c #ECEAF2",
"+. c #F5EDF2",
"dX c #EFEBF3",
"J  c #F5F6F7",
"1X c #F2ECF4",
"#. c #88C49E",
"<. c #F8FAFA",
"o. c #FBFBFB",
",. c #FBFBFC",
"g. c #189A4B",
"jX c #FEFFFE",
"kX c #FEFFFF",
"4. c #219A4F",
"=X c #4BB153",
"wo c #9FD055",
"s. c #2AA655",
"7. c #2AA059",
"Y. c #5AB65C",
"3. c #A3D0B2",
"P. c #39AB60",
"u. c #36AA62",
"v. c #B5D9C0",
"|. c #B8DDC2",
"q. c #4BAE6F",
"bo c #78C375",
"Jo c #83C222",
"Bo c #BBC3C9",
"b  c #C1C8CD",
"~o c #C4CCD1",
"/X c #B1D982",
":. c #C7CDD1",
"`o c #80C12B",
"_  c #CAD1D5",
"I  c #CDD2D7",
"Zo c #D3DADE",
"Ao c #D3DADF",
"$  c #D9DCDF",
"KX c #D6DBDF",
"5o c #D6DBE0",
"o  c #DCE0E2",
"+  c #DCE0E3",
"h  c #E2E5E7",
"UX c #DFE4E7",
"iX c #E5E9EB",
"P  c #E5E9EC",
"oo c #E2E2EA",
"<X c #EBEEEF",
"Z  c #EBEEF0",
"f  c #EEEFF1",
"WX c #98CC45",
"0. c #0E9242",
"2  c #F1F3F4",
"O. c #F4F4F6",
"DX c #8FC948",
"z  c #F7F8F8",
":O c #F4F1F7",
"(. c #8ACFA2",
"e  c #F7F8F9",
"zX c #8CC84A",
"&O c #F7F5FA",
"3  c #FDFDFD",
"Po c #8CC850",
"OX c #71BF5A",
"i. c #32AA5C",
"*X c #65BB5E",
"d. c #38AC61",
"N. c #53B561",
"@O c #A1D267",
"B. c #C0E4C5",
"7o c #8FCC6D",
"`X c #70BA19",
"Yo c #7CBE1E",
"1o c #BDC5CA",
"^X c #B0D77C",
"fX c #86C97E",
"_. c #C3CACF",
"k. c #C6CBD0",
"1  c #C9CFD3",
"*  c #C9CFD4",
"}  c #DBE4DD",
"EX c #7CC135",
"Q. c #77C48D",
"nX c #D8DDE1",
".o c #DBE1E4",
">X c #DEE2E5",
"|o c #88C53B",
"). c #E7DFE4",
"5X c #9BD393",
"PX c #DEE2E6",
"H. c #E4E7E9",
")  c #E1E6E9",
"6X c #A1D596",
"H  c #E7EBED",
"(o c #E7E5ED",
"]. c #EDF0F0",
"T  c #EDF0F1",
"No c #F3F5F5",
"NX c #EDEAF2",
"z. c #F3F5F6",
"x. c #F6F6F7",
"D  c #F6F6F8",
"u  c #F9FAFA",
"5  c #F9FAFB",
"W. c #F9FDFC",
"co c #CEE7A5",
"I. c #25A44D",
"ZX c #FFFFFE",
"AX c #F9FDFF",
"vo c #F6FCFF",
"-  c #FFFFFF",
"C. c #3AAB50",
"io c #BFE2B2",
"QX c #A3D15B",
"Uo c #81C11F",
">o c #81C122",
":o c #87C323",
"no c #84C223",
"to c #87C324",
"JX c #BFC7CB",
"0o c #84C225",
"Lo c #87C327",
"[X c #DAEBD8",
"So c #CED5DA",
/* pixels */
"    . X o O O O O O O O + @ # $ % &                             ",
"  * = - - - - - - - - - - - ; : > , <                           ",
"  1 2 - 3 3 3 3 3 3 3 3 3 3 4 5 6 6 7 8                         ",
"  9 0 - 3 3 3 3 3 3 3 3 3 3 3 q w e r 2 e                       ",
"  9 0 - 3 3 3 t t t t 3 3 3 3 3 y u i p - 0                     ",
"  9 0 - 3 t t t t t t t 3 3 3 3 q a s d - - 8                   ",
"  9 0 - 3 t t t t t t t t 3 3 3 3 y f g h j k l                 ",
"  9 0 - t t t t t t t t t 3 3 3 3 a z = x s c v b               ",
"  9 0 - t t t t t t t t t 3 3 3 3 y 6 n m M N B V               ",
"  9 0 - t t t t t t t t t 3 3 3 3 a 6 C m Z s A S               ",
"  9 0 - 3 t t t t t t t 3 3 3 3 3 u D = F G H A S               ",
"  9 0 - 3 3 t t t t t t 3 3 3 3 q w J 2 d K L P I               ",
"  9 0 - 3 3 3 t t t 3 3 3 3 3 3 a 6 U Y T R E W Q               ",
"  9 0 - 3 3 3 3 3 3 3 3 3 3 3 ! ~ 6 n f ^ / ( ) _               ",
"  1 ` - 3 3 3 3 3 3 3 3 3 3 ! u 6 ' ] [ { } |  ...              ",
"  X.d 4 3 3 3 3 3 3 3 3 3 o.5 O.+.@.#.$.%.&.*.=.-.;.            ",
"  :.>.u u o.q 4 3 3 q ,.u <.1.2.3.4.5.6.7.8.7.9.0.q.w.          ",
"  :.e.6 6 e w u u u w e e C +.r.t.y.u.i.p.a.s.d.f.g.h.j.        ",
"  k.l.z.C U J x.D x.J J U c.v.b.n.m.M.N.B.- V.C.Z.A.S.D.F.      ",
"  G.H.J.m Y 2 = = = K.2 f L.P.I.U.Y.T.R.E.- W.Q.!.~.^./.(.).    ",
"  _.`.'.'.].T [.F [.[.{.}.|. X.XXXoXOX+X@X#X- $X%X&X*X=X-X;X    ",
"  :X>Xe.B e.,XK <XK K R 1X2X3X4X5X6X7X8X9X0X- - qXwXeXrXtXyX    ",
"  b uXiXpXE aXL sXL L iXdXfXgXhX- - - - - jX- - - kXlXzXxXcXvX  ",
"  bXnXmXmXc MXMXMXMXMXmXNXBXVXCX- - - - - ZX- - - AXSXDXFXGXHX  ",
"  JXKXLXPXLXIXIXUXIXIXYXTXRXEXWXQX!X~X^X/X(X- - )X_X`X'X]X[X    ",
"  {X}X|X o.ov v Xov v .oooOo+o@o#o$o#o%o&o*o- =o-o;o:o>o,o<o    ",
"  1o2o3o3o3o4o4o4o4o4o3o5o6o7o8o9o0oqowoeo- =o-orotoyouoio      ",
"  poaososodofofofofofodogohojokolo0ozoxocovoborotonomoMoNo      ",
"  BoVoCoCogoZoZoAoAoZogogoSoDoFoGoHoJoKoLoPoIo:oUoYoToRo        ",
"    EoWoWoQo!o!o~o~o!oQoWo^o/o(o)o_o`o'o]o[o{o}o|o O.O          ",
"                                  XOoOOO+O@O#O$O%O&O            ",
"                                      *O=O-O;O:O                "
};

/* XPM */
static const char *printer_search_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 347 2",
"#X c #D1D9DD",
"f  c #DADFE1",
":  c #E0E4E7",
"So c #797B7C",
"ho c #6D777D",
"`  c #ECEEEF",
"K  c #EFEFF0",
"O  c #ECEEF0",
"QX c #737C83",
"s. c #EFF2F4",
"'  c #F2F3F4",
"-. c #F5F7F8",
"co c #F8F8F9",
"mo c #21201F",
"t  c #FEFDFD",
"5o c #889299",
"eo c #9D9C9C",
"<. c #97A0A5",
"bo c #454748",
"Ko c #484B4C",
"c. c #454A4E",
"D. c #C4CACF",
"8. c #C1C9CF",
"d  c #CDD3D7",
"z  c #D0D7DA",
"Vo c #D6D9DB",
"gX c #D3D8DB",
"fo c #DCDEE1",
"f. c #DCE1E5",
"L. c #6F7679",
"sX c #DFE5E8",
"5X c #E2E6E9",
"j. c #E8EBEC",
"Ao c #E5EAED",
"+  c #E8EBED",
"g  c #EEF0F1",
"@o c #757E85",
"xX c #F4F5F5",
"9o c #788288",
"!. c #F4F5F6",
"WX c #788289",
"i  c #F4F5F7",
"zX c #FAFAF9",
"4. c #F7F9F9",
"{. c #FAFAFA",
"]X c #201E1F",
"&  c #FAFAFB",
"uo c #8A8E90",
"yo c #232222",
":o c #9FA1A3",
"J. c #3E4345",
"yX c #41474A",
"E  c #B1B6BA",
"^X c #4A5054",
"Q  c #B4BDC2",
"k  c #BAC2C7",
":. c #50585E",
"l  c #C3CBCF",
"0. c #C6CCD0",
"6X c #C3CBD0",
"9. c #C9D0D3",
"rX c #5F666B",
"H  c #D2D6DA",
"   c None",
"3X c #D2D9DD",
"CX c #626A6F",
"$. c #D2D9DE",
"E. c #DBDFE1",
"7. c #D8DEE2",
"V. c #DEE3E8",
"w  c #E7E9EA",
"|  c #6E777D",
"d. c #E4E8EB",
"BX c #717B81",
"-  c #EAEDEF",
"cX c #F0F2F3",
"=  c #F0F2F4",
".  c #F6F7F7",
"X  c #F6F7F8",
"EX c #7A848B",
"+. c #F6F7F9",
"8  c #FCFCFB",
"a. c #F9FBFB",
"e  c #FCFCFC",
"W  c #7D888F",
"U. c #FCFCFD",
"9X c #808990",
"*o c #8C9092",
"HX c #92989B",
"UX c #A7A8A8",
";. c #A1A6A9",
"]  c #9EA5A9",
"K. c #373C3E",
"LX c #3D3E3F",
"ro c #404243",
"-o c #404244",
"x. c #404547",
"F. c #B9BABB",
"_. c #B9C0C4",
"&o c #4F565A",
"[  c #C2C9CE",
"Bo c #CBCFD1",
"Co c #C8CED2",
"2o c #CBCFD2",
"g. c #C5CDD3",
" . c #556168",
"1. c #CED6DA",
"n. c #D4DBDF",
"C  c #E0E5E7",
"M  c #E3E6E8",
"I  c #E0E5E8",
"#o c #70797F",
"B. c #E6EAED",
"Po c #181514",
"P. c #ECEFF1",
"qX c #737D84",
"h  c #EFF0F2",
"'. c #ECEFF2",
"M. c #F2F4F5",
"~  c #F8F9FA",
">o c #F5F8FA",
"+o c #7C868C",
"r  c #FEFEFD",
"0  c #FEFEFE",
"{X c #FBFDFE",
"nX c #828B92",
"b. c #91999E",
"l. c #A3A5A6",
"<X c #42474A",
"Do c #484C4E",
"v. c #454B4F",
"^. c #4E5152",
"#  c #BBC2C6",
"tX c #51585B",
"L  c #BEC6CC",
"t. c #545C61",
"X. c #C7CFD3",
">. c #576066",
",. c #576067",
"zo c #D0D5D9",
"so c #DCDFE1",
"7X c #D9DEE1",
")X c #727577",
"$o c #697278",
"@  c #E5E8EA",
"P  c #E2E7EA",
"S. c #E8E9EB",
";  c #E5E8EB",
"@X c #757C7F",
"5  c #EBEDEE",
"4X c #E8ECEE",
"N  c #EBEDEF",
"p. c #E8ECEF",
"$X c #EEF1F2",
"3. c #EEF1F3",
"o  c #F1F2F4",
"0X c #757F86",
"@. c #F4F6F7",
"}. c #F7F7F7",
"F  c #F7F7F8",
";o c #201F1F",
"2  c #FAFBFB",
"^  c #FAFBFC",
"ko c #7E888F",
";X c #878E93",
"YX c #818C93",
"vo c #9C9B9B",
"4o c #909AA1",
"z. c #323739",
"io c #9FA5A9",
"G. c #353B3D",
"3o c #9FA8AE",
"'X c #414547",
"4  c #AEB6BA",
",X c #4D5255",
"w. c #C9CBCC",
"c  c #C0C8CD",
"G  c #CFD3D6",
":X c #5F676B",
"|. c #D2D7DA",
"p  c #D8DCDE",
"Zo c #D8DCDF",
"Q. c #DEE1E4",
">  c #DBE0E4",
"eX c #687076",
"VX c #6B7479",
"aX c #E4E9EB",
"k. c #E7EAEC",
"*. c #EAEBED",
"6. c #E7EAED",
"JX c #EDEFF0",
"8X c #F3F4F4",
"NX c #778187",
"-X c #F3F4F5",
"jo c #778188",
"I. c #F3F4F6",
"W. c #F9F9F9",
"N. c #F6F8F9",
"5. c #F9F9FA",
"mX c #7D868D",
"IX c #222121",
"Z  c #FCFDFD",
"7o c #808A90",
"T  c #FCFDFE",
"GX c #899094",
".. c #838E94",
"6o c #868F96",
"Jo c #2B2A2A",
"RX c #89939A",
"FX c #959DA1",
"(X c #2E3436",
"Lo c #3D3F40",
"(. c #3D4243",
"OX c #3D4244",
"oX c #404648",
"/X c #404649",
"). c #464B4E",
"r. c #4C5357",
"U  c #B6C0C6",
"]. c #C2CACE",
"~X c #555C61",
"Z. c #CBD0D2",
"7  c #CBD0D3",
",o c #C8CFD4",
"qo c #5E686D",
"=X c #D4D9DC",
"%o c #61696E",
"!X c #646D72",
",  c #D7DDE1",
"1o c #DDDFE2",
"ao c #E0E3E5",
"A  c #DDE2E5",
"`. c #E3E7EA",
"xo c #E6E8EA",
"2. c #E3E7EB",
"[. c #EFF1F2",
"#. c #EFF1F3",
"B  c #F5F6F6",
"*  c #F5F6F7",
"(  c #F5F6F8",
"%X c #F8FAFA",
"u  c #FBFBFB",
"q  c #FBFBFC",
" X c #FEFFFF",
".o c #828C92",
".X c #9D9B9A",
"PX c #2D2F30",
"+X c #363B3D",
"No c #A3A9AD",
"H. c #3F4446",
"=. c #ACB2B5",
"to c #424547",
"no c #45494B",
"Go c #484A4C",
" o c #B8BFC3",
"y. c #51595F",
"*X c #C4CCD0",
"s  c #C7CDD1",
")  c #C4CCD2",
"hX c #C7D0D5",
"!  c #CDD5D9",
"<  c #D3D7DA",
"b  c #D6DBDD",
"po c #D9DCDE",
"i. c #D3DADE",
"|X c #D9DCE0",
"}X c #DFE1E4",
"pX c #DFE4E7",
"Xo c #69737A",
"3  c #E5E9EB",
"SX c #757D80",
"0o c #727C82",
"~. c #F1F0F0",
"&X c #EBEEF0",
"uX c #787E82",
"m. c #EBEEF1",
"Y. c #F1F3F4",
"n  c #F7F8F8",
"{  c #F7F8F9",
"8o c #7B858B",
"q. c #F7F8FA",
"9  c #FDFDFC",
"$  c #FDFDFD",
"1  c #FDFDFE",
"lo c #818A91",
"DX c #8D9498",
"[X c #A2A4A5",
"lX c #A5A5A6",
"u. c #9CA5A9",
"1X c #383D3F",
"`X c #414344",
"wo c #ABB0B3",
"go c #AEB4B8",
"e. c #41464A",
"iX c #BAC1C5",
"ZX c #50575B",
"v  c #BDC5CA",
"fX c #C3CACE",
">X c #596064",
"V  c #C9CFD3",
"a  c #CFD4D7",
"C. c #CFD4D8",
"x  c #D8DDE1",
"o. c #DBE1E5",
"<o c #E4E7E8",
"do c #E4E7E9",
"R. c #E1E6E9",
"6  c #E4E7EA",
"T. c #E7EBED",
"Mo c #717A80",
"R  c #EDEDEF",
"KX c #F0F1F1",
"J  c #F0F1F2",
"wX c #747E84",
"vX c #747E85",
"O. c #F3F5F6",
"MX c #7A838A",
"&. c #F9FAFA",
"/  c #F9FAFB",
"y  c #FCFBFB",
"oo c #7D878E",
"m  c #FFFFFE",
"%  c #FFFFFF",
"2X c #899194",
"Oo c #838C93",
"Fo c #282727",
"Io c #2B2B2A",
"bX c #869097",
"kX c #2E2C2B",
"=o c #A19F9F",
"TX c #8F99A0",
"_X c #A4A3A2",
"XX c #313335",
"j  c #8F9CA4",
"jX c #A1A5A9",
"/. c #3A3F41",
"Ho c #494D4E",
"}  c #BCC0C3",
"Y  c #B9BFC3",
"S  c #B9C2C7",
"%. c #B9C2C8",
"_  c #BFC7CB",
"AX c #555D60",
"A. c #C8CDD0",
"h. c #CBD1D5",
"D  c #CED5D7",
"dX c #CED5D9",
/* pixels */
"                                                                ",
"                                                                ",
"                                                                ",
"                                . X o O + @                     ",
"                              # $ % % $ & * = - ; : > ,         ",
"                              < % % % % % % % % % 1 2 3         ",
"                            4 5 % % % % % % % % % % % 6         ",
"                            7 $ 8 9 0 % % % % % % % q :         ",
"                            w % $ e r t y u e e 9 $ i           ",
"                            p a s d f g r 0 e e e e h           ",
"                        j k l z x z c v b n % $ $ m M           ",
"                  N B V c C Z % % % 1 A S D F 0 $ 0 G           ",
"            H J % K 7 L P % % 0 0 0 % % I U b e e 2 Y           ",
"          6 T R E W Q ! ~ ^ 2 ^ ^ 2 / / ( ) _ ` % ' ]           ",
"      [ N { } |  ...X.o.O.{ ~ / / ~ +.@.#.$.%.b &.*.=.          ",
"      6 -.;.:.>.,.<.1.2.3.4./ ^ 2 5.X O.6.7.8.9.n % % %         ",
"    0.q.w.e.r.t.y.u.i.p.3.4.a.^ ^ / { s.d.f.g.h.% j.k.% % % $   ",
"    h.% l.z.x.c.v.b.n.m.M.{ / e e / N.s.B.V.) C.% Z.A.% % 1 u S.",
"    D.% F.G.H.J.K.L.1.P.I.{ & $ U./ N.Y.T.R.c E.% % % % 0 W.W.Q.",
"      !.~.^./.H.(.)._.`.5.X ~ u 2 ~ -.3.'.> ].[.% % % 0 {.}.X |.",
"         X.XXXoXOX+X@X#X$X1 a./ %X~ X N.&X*X=X% % % % u }.-X'   ",
"          ;X:X>X,X<X1X2X3X4X/ 0 0 Z q.5X6Xc 7X8X% % e }.-XY.k.  ",
"      9X0XqXwX| eXrXtXyXuXiX3XpXaXsXdXfXgXhXjXkXlX% zXxXcXo =X  ",
"      vXbXnXmXMXNXBXVXCXZXAXSXDXFXGXHXJX% KXLXPXIXUX% ' g N     ",
"        YXTXRXEXqXWXNXQX| !X~X^X/X(X)X% e 0 _X`X'X]X[X{X$X7X    ",
"    }X|X o.oXo    ooOo+oWX@o#o$o%o&o*o% {.u % =o-o'X;o:o>o,o    ",
"  <o1o2o        3o4o5oRX6o7o8o9o0oqowo1 r $ e % eorotoyouo      ",
"  iopoaosoaodofogo    hojokoloNX        zoxo8Xco% vobonomo      ",
"    MoNoBoVogo                                CoZoAoSoroDoFo    ",
"                                                        GoHoJo  ",
"                                                          KoLoPo",
"                                                            Io  "
};

/* XPM */
static const char *page_accept_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 394 2",
"-o c #D4DADE",
".  c #D7DBDF",
"Mo c #D4DADF",
".o c #ECF7E7",
",X c #E0E4E7",
"s  c #E6E9EB",
"!  c #E6E9EC",
"Go c #90C941",
"2X c #E9EDEE",
"Q  c #E9EDEF",
"C  c #ECEEF0",
"5O c #F2F3F2",
"s. c #F2EAEF",
"f  c #F2F3F4",
"U. c #F2F3F5",
"r  c #F8F8F9",
"(  c #FBFCFC",
"K. c #1EA04F",
"vX c #78BE51",
"c. c #249F51",
"RX c #6CBA53",
"TX c #9DD5B3",
"S. c #54B55D",
"0X c #7BC25F",
"j. c #36AB60",
".X c #AFD8B9",
"J. c #A9DCBB",
",O c #A2D267",
"do c #8ACA67",
"ro c #8DCB6A",
"fo c #7ABD1C",
"Zo c #D9ECD2",
"wX c #96CE7F",
"_. c #DCF0D9",
"0  c #CDD3D6",
"*  c #D3D8DB",
"|X c #8FC733",
"5X c #71BD35",
"XX c #29A538",
",o c #DCE1E4",
"<o c #DCE1E5",
"B. c #059339",
"f. c #028F39",
"&O c #89C53D",
"{  c #E2E6E9",
":X c #EBECEC",
"F  c #E8EBED",
"<X c #E8EBEE",
"Y  c #EEF0F1",
"d. c #84C59A",
"@. c #F1EBF0",
"|. c #EEF0F2",
"HX c #EBE9F0",
"to c #F1F1F3",
".. c #F4ECF2",
"-  c #F4F5F6",
"oo c #FAFDF9",
"c  c #FAFAFB",
"eX c #FDFEFD",
"mX c #F7FCFD",
"zX c #92CB4F",
"7. c #99CCAA",
"@X c #77C263",
"L. c #3BAB63",
"#X c #7DC467",
">O c #A7D570",
"PX c #E1F0CB",
"/o c #82C11D",
"z. c #4DB474",
"'o c #7CBF1E",
"`o c #82C11F",
"xo c #88C322",
"co c #85C222",
"Po c #9ED27B",
"0o c #85C223",
"Do c #85C225",
"N  c #C0C7CC",
"=o c #C0C7CD",
"|o c #C3CBD0",
"v. c #D8EADD",
"   c None",
"Vo c #D2D9DE",
"4X c #89CB8E",
"%  c #DBDFE2",
"po c #D8DEE2",
")X c #DEE3E6",
"KX c #85C53B",
"2. c #EDE8E9",
"Co c #E1E1E9",
"1X c #EAEDEE",
"t. c #0A903E",
"T  c #EAEDEF",
"iX c #67BB43",
"so c #EDEEF0",
"9  c #F0F2F3",
"6o c #91C945",
"a. c #F3F3F4",
"W  c #F0F2F4",
"7  c #F6F7F8",
"^X c #F3EDF5",
"xX c #B3DCA3",
"&. c #F9FBFB",
"e  c #FCFCFC",
"F. c #4CB256",
"l. c #31A95B",
"bX c #85C55D",
"D. c #55B55D",
"A. c #4FB35D",
"9X c #79C160",
"OO c #9DD067",
"]o c #9DD06E",
"2o c #CBE4C5",
"SX c #C2C9CE",
"b. c #C8CED3",
"S  c #CBD2D6",
"2O c #E9F4E0",
"*o c #E0EEDF",
"o. c #DDE4DE",
"No c #D4DBDF",
"#  c #DDE1E3",
":o c #DAE0E3",
"Ao c #78BF39",
"X  c #DDE1E4",
";o c #DAE0E4",
"sX c #DDE1E5",
"{X c #A3D593",
"P. c #ECEFEC",
"[  c #E3E6EA",
"X. c #ECE9EB",
"gX c #E6EAEC",
"n. c #E9EBED",
"G  c #E6EAED",
"{. c #ECEFF0",
"jX c #ECE9F0",
"M. c #F2EBF1",
"y  c #F2F4F5",
"_  c #F2F4F6",
"Io c #F5F5F7",
"&o c #7BC349",
")  c #F8F9F9",
"4O c #F5F2F8",
",  c #F8F9FA",
"+o c #63B84B",
"3O c #F8F6FB",
"i  c #FEFEFE",
"8. c #1B974B",
"=X c #48AF53",
"uX c #87C755",
":. c #2A9C56",
"(. c #6CBE5B",
"^. c #69BD5C",
"C. c #3FAF60",
"Jo c #6FBF6B",
";. c #42A769",
"5o c #7DBF18",
"~o c #7DBF1C",
"@O c #7DBF1E",
"So c #80C01E",
"lo c #BEE4D0",
"eo c #80C021",
"$o c #83C122",
"go c #86C225",
" O c #C4CBD1",
"3. c #CAD0D4",
"^  c #CAD0D5",
"7o c #EBF6E4",
"WX c #8FC838",
"O  c #DFE3E5",
"`X c #DFE3E7",
"fX c #E5E8EB",
"]  c #E8E9EC",
"Z  c #E8ECED",
"A  c #E8ECEE",
"L  c #EBEDEF",
"jo c #8FC842",
"T. c #EEEEF1",
"K  c #EEF1F2",
" . c #F1EFF2",
"/  c #F4F6F6",
"<  c #F4F6F7",
"7O c #F4F0F7",
"p  c #FAFBFB",
">  c #FAFBFC",
"UX c #BADEA5",
"r. c #1D994E",
"-X c #4DB252",
"&X c #C0E3B5",
"q. c #2CA15B",
"oX c #56B55F",
"tX c #56B560",
"kX c #7DC573",
"Uo c #BDC4CA",
"{o c #C0C8CD",
".O c #C0C8CE",
"8o c #7AC47F",
"[. c #C6CDD1",
"|  c #CCD2D5",
"@o c #8BC52E",
"2  c #CCD2D6",
"u. c #DBE9DE",
"(o c #7CC031",
"3o c #70BC31",
"x  c #D2D7DA",
"1  c #D8DCDF",
"Bo c #D5DBE0",
"FX c #E1E5E8",
"~  c #E4E9EB",
"g  c #E7EAEC",
"EX c #82C23F",
"hX c #E7EAED",
"H. c #F0F9F3",
"k  c #EDEFF0",
"-. c #80C098",
"j  c #EDEFF1",
"*. c #F3EBF0",
"b  c #F3F4F5",
"B  c #F3F4F6",
"5  c #FCFDFD",
"0. c #229C53",
"+X c #6ABD58",
"E. c #31A759",
"OX c #64BB5B",
"6X c #73C05C",
"~. c #64BB5C",
"/. c #6DBE5D",
"). c #6DBE5E",
"w. c #2EA35C",
"k. c #37AC5F",
";X c #55B960",
"Q. c #4FB460",
"*X c #A4D8B9",
"N. c #AAD4B8",
"9o c #75BC17",
"ho c #7EBF19",
"Ko c #7EBF1A",
"$O c #7EBF1C",
"%O c #7EBF20",
"MX c #67BC78",
"yo c #BFC6CB",
"zo c #60B528",
"}o c #C2CAD0",
">X c #C5CBD0",
"}  c #C8CFD4",
"$. c #CBD0D5",
"Ho c #E0F2E1",
"ko c #E9F5E1",
"uo c #D1D8DC",
"To c #D1D8DD",
"(X c #D7DDE0",
":O c #B8DD8F",
"&  c #DADEE1",
"io c #D7DDE1",
"}X c #7EC237",
"+. c #E3E7E4",
"DX c #DADEE2",
"+  c #E0E3E5",
"lX c #87C53A",
"z  c #E0E3E6",
"]X c #DDE2E6",
"'. c #0C993A",
"R. c #E6E8E7",
"!X c #75BF3C",
"M  c #E3E7E9",
"GX c #E3E7EA",
"[o c #F2F8F1",
"m  c #E9ECEE",
"h  c #EFF1F2",
"=O c #F5F9F5",
"V  c #EFF1F3",
"Wo c #ECEAF2",
"5. c #F2F2F4",
"q  c #F5F6F6",
"AX c #F2ECF3",
"I  c #F5F6F7",
"%. c #FBFBFB",
"YX c #BEDFA3",
"x. c #189A4A",
"4. c #FBFBFC",
"IX c #FEFFFF",
"W. c #27A54E",
"g. c #27A257",
"e. c #2AA059",
"NX c #72BE5B",
"!. c #5AB65C",
",. c #33A05D",
"h. c #36AA62",
"y. c #4BAE70",
"oO c #DFF0D3",
"^o c #83C223",
"<O c #AED87C",
"/X c #C1C8CD",
")o c #74BD29",
"nX c #E2F1D9",
"+O c #80C12A",
"I. c #C7CDD1",
"=. c #D6E1D9",
"E  c #CAD1D5",
"=  c #CDD2D6",
"rX c #D0EBDE",
"O. c #D9E2DC",
"8X c #99D189",
"Ro c #D3DADE",
"Eo c #D3DADF",
"#. c #D9D6DD",
"ao c #D6DBE0",
"$  c #DCE0E3",
"l  c #E2E5E7",
"'X c #DFE4E7",
"8  c #E2E5E8",
"*O c #BADF97",
"dX c #E5E9EB",
"%o c #95CB3F",
"cX c #A8D697",
"R  c #E5E9EC",
"1o c #E2E2EA",
"'  c #EBEBEE",
"p. c #EBEEEF",
"6. c #F1EAEE",
"D  c #EBEEF0",
"`  c #EEEFF1",
"U  c #F1F3F4",
"[X c #EBE8F1",
"9. c #0E9243",
"Y. c #F1F3F5",
"u  c #F7F8F8",
"v  c #F7F8F9",
"6O c #F7F5F9",
" o c #95CB4B",
"4  c #FDFDFD",
":  c #FDFDFE",
"]. c #93D2A9",
"qX c #74BD55",
">. c #269A53",
"V. c #2FA95B",
"Z. c #47B15E",
"%X c #77C160",
"bo c #C6E6BB",
"1. c #AED5BB",
"Qo c #CFE9C0",
";O c #D8ECC5",
"ZX c #C6E3C8",
"Fo c #7FBF1A",
"G. c #65BE73",
"JX c #86C975",
"vo c #73BB24",
"no c #BDC5CB",
"<. c #5CB27C",
"aX c #C3CACF",
"i. c #C9CFD3",
"Yo c #CCD3D9",
"Oo c #D8EFE7",
"o  c #DEE2E4",
">o c #DBE1E4",
"!o c #7FC23A",
"@  c #DEE2E5",
"CX c #70BD3B",
"_X c #DEE2E6",
"t  c #E4E7E9",
"P  c #E7EBED",
"XO c #E7E5ED",
"}. c #EDF0F0",
"n  c #EDF0F1",
"-O c #F3F5F3",
" X c #F0EBF0",
"d  c #F0F1F3",
"3  c #F3F5F5",
"3X c #F0EBF3",
"m. c #F6F6F7",
"J  c #F6F6F8",
"a  c #F9FAFA",
"LX c #9DCF4B",
"6  c #F9FAFB",
"Xo c #FCFEFC",
"1O c #C5E4A5",
"w  c #FFFFFE",
";  c #FFFFFF",
"VX c #A3D151",
"QX c #A3D152",
"yX c #70C05E",
"BX c #8BC963",
"7X c #79C368",
"$X c #7FC569",
"~X c #CBE6C9",
"#O c #81C11C",
"#o c #84C222",
"Lo c #84C223",
"_o c #87C324",
"wo c #84C224",
"qo c #84C225",
"4o c #87C326",
"`. c #67C084",
"pX c #D4E9DB",
"H  c #CBD1D6",
"mo c #CED5DA",
/* pixels */
"    . X o O + + + + + + @ # $ % & *                             ",
"  = - ; ; ; ; ; ; ; ; ; ; ; : > , < 1                           ",
"  2 3 ; 4 4 4 4 4 4 4 4 4 4 5 6 7 7 8 9                         ",
"  0 q w 4 4 4 4 4 4 4 4 4 4 4 e , r t y u                       ",
"  0 q w 4 4 4 i i i i 4 4 4 4 4 p a s d ; f                     ",
"  0 q w 4 i i i i i i i 4 4 4 4 e p g h ; ; j                   ",
"  0 q w 4 i i i i i i i i 4 4 4 4 p k X l z s x                 ",
"  0 q w i i i i i i i i i 4 4 4 4 c v b n m M X N               ",
"  0 q w i i i i i i i i i 4 4 4 4 p 7 B V C Z A S               ",
"  0 q w i i i i i i i i i 4 4 4 4 c 7 b V D F G H               ",
"  0 q w 4 i i i i i i i 4 4 4 4 4 a J y K L P G H               ",
"  0 q w 4 4 i i i i i i 4 4 4 4 e , I U Y T g R E               ",
"  0 q w 4 4 4 i i i 4 4 4 4 4 4 c 7 - W n Q ! ~ ^               ",
"  0 / w 4 4 4 4 4 4 4 4 4 4 4 ( ) 7 _ ` ' ] [ { }               ",
"  | b w 4 4 4 4 4 4 4 4 4 4 ( 6 7  ...X.o.O.+.@.#.              ",
"  $.9 e 4 4 4 4 4 4 4 4 4 %.&.b *.=.-.;.:.>.,.<.1.2.            ",
"  3.Y a a %.e 5 4 4 e 4.a a 5.6.7.8.9.0.q.w.e.r.t.y.u.          ",
"  i.p.7 7 v , a a a , v v a.s.d.f.g.h.j.k.k.k.l.z.x.c.v.        ",
"  b.n.b b - I m.J m.I I - M.N.B.V.C.Z.A.S.D.F.G.H.J.K.L.P.      ",
"  I.s V V W U y y y U.Y.T.R.E.W.Q.!.~.^./.(.)._.; ; `.'.].      ",
"  [.M {.{.}.n |.K |.|.K  X.XXXoXOX+X@X#X$X%X&X; ; *X=X-X;X:X    ",
"  >X,X<XA m 1XT p.T T 2X3X4X5X6X7X8X9X0XqXwXeX; rXtXyXuXiXpX    ",
"  aXsXdXfX! gXg hXg g fXjXkXlXzXxX; cXvXbXnX; mXMXNXBXVXCXZXAX  ",
"  SXDXFXFX{ GXGXGXGXGXFXHXJXKXLXPX; IXUXYX; ; TXRXEXWXQX!X~X^X  ",
"  /X(X)X_X)X`X`X'X`X`X]X[X{X}X|X o.o; Xooo; Oo+o@o#o$o%o&o*o    ",
"  =o-o;o:o>o,o,o<o,o,o>o1o2o3o4o5o6o7o; ; ; 8o9o0oqowoeoroto    ",
"  youoioioiopopopopopoioaosodofogohojoko; lozoxoqoqocovobo      ",
"  nomoMoMoNoBoBoBoBoBoNoVoCoZoAoSoDoFoGoHoJoKoDoqoLofoPoIo      ",
"  UoYoToToVoRoRoEoEoRoVoVomoWoQo!o~o^o/o(o)o_owo`o'o]o[o        ",
"    {o}o|o|o O O O O O|o|o}o.OXOoOOO+O@Oho#O$O%O&O*O=O          ",
"                                  -O;O:O>O,O<O1O2O3O            ",
"                                      4Oa.5O6O7O                "
};

////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merlin type definition
//

IMPLEMENT_CLASS( Merlin, wxFrame )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merlin event table definition
//

BEGIN_EVENT_TABLE( Merlin, wxFrame )

////@begin Merlin event table entries
////@end Merlin event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merlin constructors
//

Merlin::Merlin()
{
  Init();
}

Merlin::Merlin( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create( parent, id, caption, pos, size, style );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merlin creator
//

bool Merlin::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin Merlin creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  Centre();
////@end Merlin creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Merlin destructor
//

Merlin::~Merlin()
{
////@begin Merlin destruction
////@end Merlin destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void Merlin::Init()
{
////@begin Merlin member initialisation
////@end Merlin member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for Merlin
//

void Merlin::CreateControls()
{    
////@begin Merlin content construction
  Merlin* itemFrame1 = this;

  wxMenuBar* menuBar = new wxMenuBar;
  wxMenu* itemMenu3 = new wxMenu;
  itemMenu3->Append(ID_MENUITEM6, _("Open"), wxEmptyString, wxITEM_NORMAL);
  itemMenu3->Append(ID_MENUITEM, _("Save"), wxEmptyString, wxITEM_NORMAL);
  itemMenu3->Append(ID_MENUITEM1, _("Save as..."), wxEmptyString, wxITEM_NORMAL);
  menuBar->Append(itemMenu3, _("File"));
  itemFrame1->SetMenuBar(menuBar);

  wxToolBar* itemToolBar7 = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR1 );
  itemToolBar7->Realize();
  itemFrame1->SetToolBar(itemToolBar7);

  wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemFrame1, wxID_ANY, _("Tools"));
  wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxHORIZONTAL);
  itemFrame1->SetSizer(itemStaticBoxSizer8);

  wxScrolledWindow* itemScrolledWindow9 = new wxScrolledWindow;
  itemScrolledWindow9->Create( itemFrame1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(150, 0), wxRAISED_BORDER|wxHSCROLL );
  itemStaticBoxSizer8->Add(itemScrolledWindow9, 1, wxGROW|wxALL, 0);
  itemScrolledWindow9->SetScrollbars(1, 1, 0, 0);
  wxFlexGridSizer* itemFlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
  itemScrolledWindow9->SetSizer(itemFlexGridSizer10);

  wxBitmapButton* itemBitmapButton11 = new wxBitmapButton;
  itemBitmapButton11->Create( itemScrolledWindow9, ID_BITMAPBUTTON1, itemFrame1->GetBitmapResource(wxT("icons/png/32x32/pdf.png")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton12 = new wxBitmapButton;
  itemBitmapButton12->Create( itemScrolledWindow9, ID_BITMAPBUTTON, itemFrame1->GetBitmapResource(wxT("Help.xpm")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton13 = new wxBitmapButton;
  itemBitmapButton13->Create( itemScrolledWindow9, ID_BITMAPBUTTON2, itemFrame1->GetBitmapResource(wxT("icons/png/32x32/lock.png")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton14 = new wxBitmapButton;
  itemBitmapButton14->Create( itemScrolledWindow9, ID_BITMAPBUTTON3, itemFrame1->GetBitmapResource(wxT("icons/png/32x32/page_next.png")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton15 = new wxBitmapButton;
  itemBitmapButton15->Create( itemScrolledWindow9, ID_BITMAPBUTTON4, itemFrame1->GetBitmapResource(wxT("icons/png/32x32/printer_search.png")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton16 = new wxBitmapButton;
  itemBitmapButton16->Create( itemScrolledWindow9, ID_BITMAPBUTTON6, itemFrame1->GetBitmapResource(wxT("icons/png/32x32/page_accept.png")), wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW );
  itemFlexGridSizer10->Add(itemBitmapButton16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton17 = new wxBitmapButton;
  itemBitmapButton17->Create( itemScrolledWindow9, ID_BITMAPBUTTON7, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton17->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton17, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton18 = new wxBitmapButton;
  itemBitmapButton18->Create( itemScrolledWindow9, ID_BITMAPBUTTON8, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton18->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton18, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton19 = new wxBitmapButton;
  itemBitmapButton19->Create( itemScrolledWindow9, ID_BITMAPBUTTON9, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton19->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton20 = new wxBitmapButton;
  itemBitmapButton20->Create( itemScrolledWindow9, ID_BITMAPBUTTON10, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton20->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton21 = new wxBitmapButton;
  itemBitmapButton21->Create( itemScrolledWindow9, ID_BITMAPBUTTON11, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton21->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton22 = new wxBitmapButton;
  itemBitmapButton22->Create( itemScrolledWindow9, ID_BITMAPBUTTON12, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton22->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton22, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton23 = new wxBitmapButton;
  itemBitmapButton23->Create( itemScrolledWindow9, ID_BITMAPBUTTON13, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton23->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton23, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton24 = new wxBitmapButton;
  itemBitmapButton24->Create( itemScrolledWindow9, ID_BITMAPBUTTON14, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton24->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton24, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton25 = new wxBitmapButton;
  itemBitmapButton25->Create( itemScrolledWindow9, ID_BITMAPBUTTON15, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton25->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton25, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton26 = new wxBitmapButton;
  itemBitmapButton26->Create( itemScrolledWindow9, ID_BITMAPBUTTON16, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton26->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton26, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton27 = new wxBitmapButton;
  itemBitmapButton27->Create( itemScrolledWindow9, ID_BITMAPBUTTON17, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton27->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton27, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton28 = new wxBitmapButton;
  itemBitmapButton28->Create( itemScrolledWindow9, ID_BITMAPBUTTON18, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton28->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton28, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton29 = new wxBitmapButton;
  itemBitmapButton29->Create( itemScrolledWindow9, ID_BITMAPBUTTON19, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton29->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton29, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton30 = new wxBitmapButton;
  itemBitmapButton30->Create( itemScrolledWindow9, ID_BITMAPBUTTON20, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton30->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton30, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton31 = new wxBitmapButton;
  itemBitmapButton31->Create( itemScrolledWindow9, ID_BITMAPBUTTON21, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton31->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton31, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton32 = new wxBitmapButton;
  itemBitmapButton32->Create( itemScrolledWindow9, ID_BITMAPBUTTON22, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton32->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton32, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton33 = new wxBitmapButton;
  itemBitmapButton33->Create( itemScrolledWindow9, ID_BITMAPBUTTON23, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton33->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton33, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton34 = new wxBitmapButton;
  itemBitmapButton34->Create( itemScrolledWindow9, ID_BITMAPBUTTON24, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton34->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton34, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton35 = new wxBitmapButton;
  itemBitmapButton35->Create( itemScrolledWindow9, ID_BITMAPBUTTON25, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton35->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton35, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton36 = new wxBitmapButton;
  itemBitmapButton36->Create( itemScrolledWindow9, ID_BITMAPBUTTON26, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton36->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton36, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton37 = new wxBitmapButton;
  itemBitmapButton37->Create( itemScrolledWindow9, ID_BITMAPBUTTON27, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton37->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton37, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton38 = new wxBitmapButton;
  itemBitmapButton38->Create( itemScrolledWindow9, ID_BITMAPBUTTON28, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton38->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton38, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton39 = new wxBitmapButton;
  itemBitmapButton39->Create( itemScrolledWindow9, ID_BITMAPBUTTON29, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton39->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton39, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton40 = new wxBitmapButton;
  itemBitmapButton40->Create( itemScrolledWindow9, ID_BITMAPBUTTON30, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton40->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton40, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton41 = new wxBitmapButton;
  itemBitmapButton41->Create( itemScrolledWindow9, ID_BITMAPBUTTON31, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton41->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton41, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton42 = new wxBitmapButton;
  itemBitmapButton42->Create( itemScrolledWindow9, ID_BITMAPBUTTON32, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton42->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton42, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton43 = new wxBitmapButton;
  itemBitmapButton43->Create( itemScrolledWindow9, ID_BITMAPBUTTON33, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton43->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton43, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton44 = new wxBitmapButton;
  itemBitmapButton44->Create( itemScrolledWindow9, ID_BITMAPBUTTON34, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton44->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton44, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton45 = new wxBitmapButton;
  itemBitmapButton45->Create( itemScrolledWindow9, ID_BITMAPBUTTON35, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton45->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton45, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton46 = new wxBitmapButton;
  itemBitmapButton46->Create( itemScrolledWindow9, ID_BITMAPBUTTON36, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton46->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton46, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton47 = new wxBitmapButton;
  itemBitmapButton47->Create( itemScrolledWindow9, ID_BITMAPBUTTON37, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton47->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton47, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton48 = new wxBitmapButton;
  itemBitmapButton48->Create( itemScrolledWindow9, ID_BITMAPBUTTON38, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton48->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton48, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton49 = new wxBitmapButton;
  itemBitmapButton49->Create( itemScrolledWindow9, ID_BITMAPBUTTON39, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton49->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton49, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton50 = new wxBitmapButton;
  itemBitmapButton50->Create( itemScrolledWindow9, ID_BITMAPBUTTON40, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton50->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton50, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton51 = new wxBitmapButton;
  itemBitmapButton51->Create( itemScrolledWindow9, ID_BITMAPBUTTON41, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton51->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton51, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton52 = new wxBitmapButton;
  itemBitmapButton52->Create( itemScrolledWindow9, ID_BITMAPBUTTON42, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton52->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton52, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton53 = new wxBitmapButton;
  itemBitmapButton53->Create( itemScrolledWindow9, ID_BITMAPBUTTON43, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton53->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton53, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton54 = new wxBitmapButton;
  itemBitmapButton54->Create( itemScrolledWindow9, ID_BITMAPBUTTON44, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton54->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton54, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton55 = new wxBitmapButton;
  itemBitmapButton55->Create( itemScrolledWindow9, ID_BITMAPBUTTON45, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton55->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton55, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton56 = new wxBitmapButton;
  itemBitmapButton56->Create( itemScrolledWindow9, ID_BITMAPBUTTON46, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton56->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton56, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton57 = new wxBitmapButton;
  itemBitmapButton57->Create( itemScrolledWindow9, ID_BITMAPBUTTON47, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton57->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton57, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton58 = new wxBitmapButton;
  itemBitmapButton58->Create( itemScrolledWindow9, ID_BITMAPBUTTON48, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton58->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton58, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton59 = new wxBitmapButton;
  itemBitmapButton59->Create( itemScrolledWindow9, ID_BITMAPBUTTON49, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton59->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton59, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton60 = new wxBitmapButton;
  itemBitmapButton60->Create( itemScrolledWindow9, ID_BITMAPBUTTON50, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton60->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton60, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton61 = new wxBitmapButton;
  itemBitmapButton61->Create( itemScrolledWindow9, ID_BITMAPBUTTON51, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton61->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton61, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton62 = new wxBitmapButton;
  itemBitmapButton62->Create( itemScrolledWindow9, ID_BITMAPBUTTON52, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton62->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton62, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton63 = new wxBitmapButton;
  itemBitmapButton63->Create( itemScrolledWindow9, ID_BITMAPBUTTON53, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton63->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton63, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton64 = new wxBitmapButton;
  itemBitmapButton64->Create( itemScrolledWindow9, ID_BITMAPBUTTON54, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton64->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton64, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton65 = new wxBitmapButton;
  itemBitmapButton65->Create( itemScrolledWindow9, ID_BITMAPBUTTON55, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton65->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton65, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton66 = new wxBitmapButton;
  itemBitmapButton66->Create( itemScrolledWindow9, ID_BITMAPBUTTON56, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton66->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton66, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton67 = new wxBitmapButton;
  itemBitmapButton67->Create( itemScrolledWindow9, ID_BITMAPBUTTON57, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton67->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton67, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton68 = new wxBitmapButton;
  itemBitmapButton68->Create( itemScrolledWindow9, ID_BITMAPBUTTON58, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton68->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton68, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton69 = new wxBitmapButton;
  itemBitmapButton69->Create( itemScrolledWindow9, ID_BITMAPBUTTON59, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton69->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton69, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton70 = new wxBitmapButton;
  itemBitmapButton70->Create( itemScrolledWindow9, ID_BITMAPBUTTON60, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton70->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton70, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton71 = new wxBitmapButton;
  itemBitmapButton71->Create( itemScrolledWindow9, ID_BITMAPBUTTON61, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton71->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton71, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton72 = new wxBitmapButton;
  itemBitmapButton72->Create( itemScrolledWindow9, ID_BITMAPBUTTON62, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton72->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton72, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton73 = new wxBitmapButton;
  itemBitmapButton73->Create( itemScrolledWindow9, ID_BITMAPBUTTON63, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton73->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton73, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton74 = new wxBitmapButton;
  itemBitmapButton74->Create( itemScrolledWindow9, ID_BITMAPBUTTON64, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton74->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton74, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton75 = new wxBitmapButton;
  itemBitmapButton75->Create( itemScrolledWindow9, ID_BITMAPBUTTON65, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton75->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton76 = new wxBitmapButton;
  itemBitmapButton76->Create( itemScrolledWindow9, ID_BITMAPBUTTON66, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton76->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton76, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton77 = new wxBitmapButton;
  itemBitmapButton77->Create( itemScrolledWindow9, ID_BITMAPBUTTON67, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton77->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton77, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton78 = new wxBitmapButton;
  itemBitmapButton78->Create( itemScrolledWindow9, ID_BITMAPBUTTON68, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton78->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton78, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton79 = new wxBitmapButton;
  itemBitmapButton79->Create( itemScrolledWindow9, ID_BITMAPBUTTON69, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton79->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton79, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton80 = new wxBitmapButton;
  itemBitmapButton80->Create( itemScrolledWindow9, ID_BITMAPBUTTON70, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton80->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton80, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton81 = new wxBitmapButton;
  itemBitmapButton81->Create( itemScrolledWindow9, ID_BITMAPBUTTON71, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton81->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton81, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton82 = new wxBitmapButton;
  itemBitmapButton82->Create( itemScrolledWindow9, ID_BITMAPBUTTON72, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton82->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton82, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton83 = new wxBitmapButton;
  itemBitmapButton83->Create( itemScrolledWindow9, ID_BITMAPBUTTON73, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton83->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton83, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton84 = new wxBitmapButton;
  itemBitmapButton84->Create( itemScrolledWindow9, ID_BITMAPBUTTON74, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton84->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton84, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton85 = new wxBitmapButton;
  itemBitmapButton85->Create( itemScrolledWindow9, ID_BITMAPBUTTON75, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton85->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton85, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton86 = new wxBitmapButton;
  itemBitmapButton86->Create( itemScrolledWindow9, ID_BITMAPBUTTON76, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton86->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton86, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton87 = new wxBitmapButton;
  itemBitmapButton87->Create( itemScrolledWindow9, ID_BITMAPBUTTON77, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton87->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton87, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton88 = new wxBitmapButton;
  itemBitmapButton88->Create( itemScrolledWindow9, ID_BITMAPBUTTON78, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton88->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton88, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton89 = new wxBitmapButton;
  itemBitmapButton89->Create( itemScrolledWindow9, ID_BITMAPBUTTON79, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton89->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton89, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton90 = new wxBitmapButton;
  itemBitmapButton90->Create( itemScrolledWindow9, ID_BITMAPBUTTON80, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton90->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton90, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton91 = new wxBitmapButton;
  itemBitmapButton91->Create( itemScrolledWindow9, ID_BITMAPBUTTON81, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton91->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton91, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton92 = new wxBitmapButton;
  itemBitmapButton92->Create( itemScrolledWindow9, ID_BITMAPBUTTON82, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton92->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton92, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton93 = new wxBitmapButton;
  itemBitmapButton93->Create( itemScrolledWindow9, ID_BITMAPBUTTON83, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton93->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton93, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton94 = new wxBitmapButton;
  itemBitmapButton94->Create( itemScrolledWindow9, ID_BITMAPBUTTON84, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton94->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton94, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton95 = new wxBitmapButton;
  itemBitmapButton95->Create( itemScrolledWindow9, ID_BITMAPBUTTON85, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton95->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton95, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton96 = new wxBitmapButton;
  itemBitmapButton96->Create( itemScrolledWindow9, ID_BITMAPBUTTON86, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton96->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton96, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton97 = new wxBitmapButton;
  itemBitmapButton97->Create( itemScrolledWindow9, ID_BITMAPBUTTON87, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton97->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton97, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton98 = new wxBitmapButton;
  itemBitmapButton98->Create( itemScrolledWindow9, ID_BITMAPBUTTON88, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton98->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton98, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton99 = new wxBitmapButton;
  itemBitmapButton99->Create( itemScrolledWindow9, ID_BITMAPBUTTON89, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton99->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton99, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton100 = new wxBitmapButton;
  itemBitmapButton100->Create( itemScrolledWindow9, ID_BITMAPBUTTON90, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton100->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton100, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton101 = new wxBitmapButton;
  itemBitmapButton101->Create( itemScrolledWindow9, ID_BITMAPBUTTON91, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton101->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton101, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton102 = new wxBitmapButton;
  itemBitmapButton102->Create( itemScrolledWindow9, ID_BITMAPBUTTON92, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton102->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton102, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton103 = new wxBitmapButton;
  itemBitmapButton103->Create( itemScrolledWindow9, ID_BITMAPBUTTON93, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton103->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton103, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton104 = new wxBitmapButton;
  itemBitmapButton104->Create( itemScrolledWindow9, ID_BITMAPBUTTON94, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton104->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton104, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton105 = new wxBitmapButton;
  itemBitmapButton105->Create( itemScrolledWindow9, ID_BITMAPBUTTON95, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton105->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton105, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton106 = new wxBitmapButton;
  itemBitmapButton106->Create( itemScrolledWindow9, ID_BITMAPBUTTON96, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton106->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton106, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton107 = new wxBitmapButton;
  itemBitmapButton107->Create( itemScrolledWindow9, ID_BITMAPBUTTON97, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton107->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton107, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton108 = new wxBitmapButton;
  itemBitmapButton108->Create( itemScrolledWindow9, ID_BITMAPBUTTON98, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton108->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton108, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton109 = new wxBitmapButton;
  itemBitmapButton109->Create( itemScrolledWindow9, ID_BITMAPBUTTON99, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton109->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton109, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton110 = new wxBitmapButton;
  itemBitmapButton110->Create( itemScrolledWindow9, ID_BITMAPBUTTON100, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton110->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton110, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton111 = new wxBitmapButton;
  itemBitmapButton111->Create( itemScrolledWindow9, ID_BITMAPBUTTON101, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton111->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton111, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton112 = new wxBitmapButton;
  itemBitmapButton112->Create( itemScrolledWindow9, ID_BITMAPBUTTON102, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton112->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton112, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton113 = new wxBitmapButton;
  itemBitmapButton113->Create( itemScrolledWindow9, ID_BITMAPBUTTON103, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton113->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton113, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton114 = new wxBitmapButton;
  itemBitmapButton114->Create( itemScrolledWindow9, ID_BITMAPBUTTON104, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton114->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton114, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton115 = new wxBitmapButton;
  itemBitmapButton115->Create( itemScrolledWindow9, ID_BITMAPBUTTON105, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton115->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton115, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton116 = new wxBitmapButton;
  itemBitmapButton116->Create( itemScrolledWindow9, ID_BITMAPBUTTON106, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton116->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton116, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton117 = new wxBitmapButton;
  itemBitmapButton117->Create( itemScrolledWindow9, ID_BITMAPBUTTON107, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton117->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton117, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton118 = new wxBitmapButton;
  itemBitmapButton118->Create( itemScrolledWindow9, ID_BITMAPBUTTON108, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton118->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton118, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton119 = new wxBitmapButton;
  itemBitmapButton119->Create( itemScrolledWindow9, ID_BITMAPBUTTON109, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton119->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton119, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton120 = new wxBitmapButton;
  itemBitmapButton120->Create( itemScrolledWindow9, ID_BITMAPBUTTON110, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton120->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton120, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton121 = new wxBitmapButton;
  itemBitmapButton121->Create( itemScrolledWindow9, ID_BITMAPBUTTON111, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton121->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton121, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton122 = new wxBitmapButton;
  itemBitmapButton122->Create( itemScrolledWindow9, ID_BITMAPBUTTON112, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton122->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton122, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton123 = new wxBitmapButton;
  itemBitmapButton123->Create( itemScrolledWindow9, ID_BITMAPBUTTON113, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton123->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton123, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton124 = new wxBitmapButton;
  itemBitmapButton124->Create( itemScrolledWindow9, ID_BITMAPBUTTON114, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton124->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton124, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton125 = new wxBitmapButton;
  itemBitmapButton125->Create( itemScrolledWindow9, ID_BITMAPBUTTON115, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton125->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton125, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton126 = new wxBitmapButton;
  itemBitmapButton126->Create( itemScrolledWindow9, ID_BITMAPBUTTON116, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton126->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton126, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton127 = new wxBitmapButton;
  itemBitmapButton127->Create( itemScrolledWindow9, ID_BITMAPBUTTON117, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton127->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton127, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton128 = new wxBitmapButton;
  itemBitmapButton128->Create( itemScrolledWindow9, ID_BITMAPBUTTON118, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton128->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton128, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton129 = new wxBitmapButton;
  itemBitmapButton129->Create( itemScrolledWindow9, ID_BITMAPBUTTON119, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton129->Show(false);
  itemBitmapButton129->Enable(false);
  itemFlexGridSizer10->Add(itemBitmapButton129, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton130 = new wxBitmapButton;
  itemBitmapButton130->Create( itemScrolledWindow9, ID_BITMAPBUTTON120, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton130->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton130, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton131 = new wxBitmapButton;
  itemBitmapButton131->Create( itemScrolledWindow9, ID_BITMAPBUTTON121, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton131->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton131, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton132 = new wxBitmapButton;
  itemBitmapButton132->Create( itemScrolledWindow9, ID_BITMAPBUTTON122, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton132->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton132, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton133 = new wxBitmapButton;
  itemBitmapButton133->Create( itemScrolledWindow9, ID_BITMAPBUTTON123, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton133->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton133, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton134 = new wxBitmapButton;
  itemBitmapButton134->Create( itemScrolledWindow9, ID_BITMAPBUTTON124, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton134->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton134, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton135 = new wxBitmapButton;
  itemBitmapButton135->Create( itemScrolledWindow9, ID_BITMAPBUTTON125, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton135->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton135, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton136 = new wxBitmapButton;
  itemBitmapButton136->Create( itemScrolledWindow9, ID_BITMAPBUTTON126, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton136->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton136, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton137 = new wxBitmapButton;
  itemBitmapButton137->Create( itemScrolledWindow9, ID_BITMAPBUTTON127, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton137->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton137, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton138 = new wxBitmapButton;
  itemBitmapButton138->Create( itemScrolledWindow9, ID_BITMAPBUTTON128, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton138->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton138, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton139 = new wxBitmapButton;
  itemBitmapButton139->Create( itemScrolledWindow9, ID_BITMAPBUTTON129, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton139->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton139, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton140 = new wxBitmapButton;
  itemBitmapButton140->Create( itemScrolledWindow9, ID_BITMAPBUTTON130, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton140->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton140, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton141 = new wxBitmapButton;
  itemBitmapButton141->Create( itemScrolledWindow9, ID_BITMAPBUTTON131, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton141->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton141, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton142 = new wxBitmapButton;
  itemBitmapButton142->Create( itemScrolledWindow9, ID_BITMAPBUTTON132, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton142->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton142, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton143 = new wxBitmapButton;
  itemBitmapButton143->Create( itemScrolledWindow9, ID_BITMAPBUTTON133, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton143->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton143, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton144 = new wxBitmapButton;
  itemBitmapButton144->Create( itemScrolledWindow9, ID_BITMAPBUTTON134, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton144->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton144, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  wxBitmapButton* itemBitmapButton145 = new wxBitmapButton;
  itemBitmapButton145->Create( itemScrolledWindow9, ID_BITMAPBUTTON5, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
  itemBitmapButton145->Show(false);
  itemFlexGridSizer10->Add(itemBitmapButton145, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

  itemScrolledWindow9->FitInside();

  wxScrolledWindow* itemScrolledWindow146 = new wxScrolledWindow;
  itemScrolledWindow146->Create( itemFrame1, ID_DRAWING_AREA, wxDefaultPosition, wxSize(0, 600), wxRAISED_BORDER|wxFULL_REPAINT_ON_RESIZE|wxHSCROLL|wxVSCROLL );
  itemStaticBoxSizer8->Add(itemScrolledWindow146, 5, wxGROW|wxALL|wxFIXED_MINSIZE, 0);
  itemScrolledWindow146->SetScrollbars(1, 1, 0, 0);

  // Connect events and objects
  itemScrolledWindow146->Connect(ID_DRAWING_AREA, wxEVT_PAINT, wxPaintEventHandler(Merlin::OnPaint), NULL, this);
////@end Merlin content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool Merlin::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap Merlin::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin Merlin bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("icons/png/32x32/pdf.png"))
  {
    wxBitmap bitmap(pdf_xpm);
    return bitmap;
  }
  else if (name == _T("Help.xpm"))
  {
    wxBitmap bitmap(help_xpm);
    return bitmap;
  }
  else if (name == _T("icons/png/32x32/lock.png"))
  {
    wxBitmap bitmap(lock_xpm);
    return bitmap;
  }
  else if (name == _T("icons/png/32x32/page_next.png"))
  {
    wxBitmap bitmap(page_next_xpm);
    return bitmap;
  }
  else if (name == _T("icons/png/32x32/printer_search.png"))
  {
    wxBitmap bitmap(printer_search_xpm);
    return bitmap;
  }
  else if (name == _T("icons/png/32x32/page_accept.png"))
  {
    wxBitmap bitmap(page_accept_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end Merlin bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon Merlin::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin Merlin icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end Merlin icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_PAINT event handler for ID_DRAWING_AREA
//

void Merlin::OnPaint( wxPaintEvent& event )
{

    wxPaintDC dc(wxDynamicCast(event.GetEventObject(), wxWindow));
    // draw some text
    dc.DrawText(wxT("Testing"), 40, 60); 
    
    // draw a circle
    dc.SetBrush(*wxGREEN_BRUSH); // green filling
    dc.SetPen( wxPen( wxColor(255,0,0), 5 ) ); // 5-pixels-thick red outline
    dc.DrawCircle( wxPoint(200,100), 25 /* radius */ );
    
    // draw a rectangle
    dc.SetBrush(*wxBLUE_BRUSH); // blue filling
    dc.SetPen( wxPen( wxColor(255,175,175), 10 ) ); // 10-pixels-thick pink outline
    dc.DrawRectangle( 300, 100, 400, 200 );
    
    // draw a line
    dc.SetPen( wxPen( wxColor(0,0,0), 3 ) ); // black line, 3 pixels thick
    dc.DrawLine( 300, 100, 700, 300 ); // draw line across the rectangle 
}

