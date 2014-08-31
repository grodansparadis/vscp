/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscptrmit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/11/2007 17:46:01
// RCS-ID:      
// Copyright:   (C) 2007-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
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
#include "../common/vscpeventhelper.h"
#include "vscptxobj.h"
#include "dlgvscptrmit.h"

////@begin XPM images
/* XPM */
static const char *fatbee_v2_xpm[] = {
/* columns rows colors chars-per-pixel */
"48 48 731 2",
"4. c Black",
"@X c #000001",
"EO c #A3A813",
"U@ c #000002",
"H@ c #000003",
"L# c #D5920D",
"*X c #D9E10E",
"1. c #81FCFD",
"hO c #F7FE0C",
"wo c #F7FE0D",
"-O c #06070B",
"XO c #F7FE0E",
"MX c #FDFF0C",
"Y@ c #2A2B0E",
"tO c #54560A",
"%# c #303214",
"=O c #8A8F09",
"}@ c #605E0A",
"N. c #5A5D14",
"*  c #4FA6A6",
"LX c #CE9410",
"A. c #8A8F12",
"9. c #4F9A9A",
"p@ c #916301",
"NX c #BAC10D",
"!. c #242422",
"_+ c #B4BA0F",
"W  c #6E6C6B",
"W# c #7A7465",
"u  c #80FFFF",
"'. c #EAED05",
"g+ c #242428",
"!o c #DEE50D",
"T@ c #E4EC0D",
"X# c #E4EC0E",
"9@ c #DEE50F",
"k@ c #E4EC0F",
"m  c #99A0A0",
"d  c #61C7C6",
".o c #675C44",
"9# c #0B0B0E",
"cO c #3B3D0A",
"zX c #111213",
"F+ c #6B6F0A",
"[+ c #353614",
"P  c #489C9C",
"x# c #5A4B2A",
"G@ c #9BA10B",
"(. c #9BA10D",
"ZO c #72733C",
"H. c #47633B",
"s  c #4E8B8B",
"9  c #4E7F7F",
"JO c #8F9313",
"g. c #B3B70C",
"XX c #5F611C",
"%@ c #CBD30E",
"2@ c #534D17",
"C# c #F5FE0C",
"<X c #EFF70C",
"X  c #737C7C",
"%X c #222404",
"{o c #EFF70E",
"`O c #E3E90E",
"v  c #60CAC9",
"D. c #222406",
"CO c #FBFF0A",
"c+ c #FBFF0B",
"M@ c #FBFF0C",
"ZX c #FBFF0D",
",+ c #F5F80E",
"i. c #797777",
"@. c #858577",
"{X c #464808",
"gX c #654A0C",
"dX c #707303",
"5# c #6B4B08",
"uo c #52560C",
" o c #8B8C82",
"^# c #70730A",
" @ c #594825",
"p. c #767A10",
"R# c #CAD006",
"=. c #53A1A1",
"V  c #7EFFFF",
"s# c #D8960C",
"$X c #D6DE0D",
"a+ c #CAD00E",
"~@ c #F4FB0B",
"Zo c #F4FB0C",
"@@ c #40351B",
"+# c #F4FB0D",
"nX c #272804",
"yo c #F4FB0E",
"o  c #346969",
"c# c #B37B04",
"i@ c #AD7A0C",
"(+ c #5D610B",
"O# c #212112",
"/# c #878C06",
"T  c #71E2E2",
"B@ c #C58A0A",
"~O c #878C0B",
"eo c #878C0C",
"B  c #77EFEF",
"g@ c #5E4C29",
"0# c #818514",
"5X c #B1B70D",
"5O c #B7BE0F",
"@+ c #080800",
"6# c #0E0F02",
"J+ c #080803",
"4@ c #0E0F03",
";@ c #F9FF0A",
"Z. c #020108",
"3. c #080808",
"]+ c #080809",
"k+ c #F9FF0C",
"0X c #F9FF0D",
"Po c #F9FF0E",
"uX c #383A06",
"`X c #686C0C",
"b+ c #886B39",
"v@ c #92970B",
"@# c #989E0D",
"=+ c #929714",
"FO c #C2C90E",
"uO c #62651D",
"rX c #ECF40C",
"@O c #E0E60C",
"V@ c #E29E0C",
"6O c #ECF40D",
"[X c #F2FB0D",
"&X c #191A04",
"to c #E0E60F",
")+ c #131306",
"]@ c #503809",
"e# c #434501",
"O  c #63C2C2",
"5. c #737705",
"~# c #5C5E33",
"rO c #6D7009",
"1O c #73770B",
"J# c #C98B05",
"!# c #6E5522",
"BO c #CDD40C",
"qo c #D3DB0E",
"=X c #F7FF0C",
"[. c #F7FF0D",
"0O c #F1F80D",
"[O c #242505",
"@  c #5C7070",
"#@ c #7A6338",
"u@ c #302100",
"f+ c #84890B",
"|+ c #67693B",
"vX c #AEB40D",
"c. c #DEE000",
":o c #AEB40F",
"dO c #050501",
"s. c #050502",
"iO c #0B0C02",
"k. c #55A2A2",
"bo c #D8DF0D",
"}. c #050504",
"b@ c #0B0C04",
"do c #D8DF0E",
"m@ c #F6FC0C",
"R@ c #F6FC0D",
"I@ c #2F3007",
"<O c #353707",
"$. c #111F1E",
"RO c #35370A",
"f# c #66480A",
"h# c #5F6208",
"8o c #111316",
"Qo c #353710",
"Io c #595B09",
"Xo c #78570A",
"CX c #8F940A",
"$  c #73E3E3",
"OX c #8F940C",
"{+ c #8F940D",
"+X c #BFC60D",
"*. c #54A5A4",
"q@ c #BFC611",
"zO c #EFF80C",
"!X c #101003",
"fo c #EFF80D",
"bO c #161703",
"1+ c #1C1E03",
"1o c #E9F10D",
"Eo c #101004",
"3+ c #DDE30E",
"8O c #101005",
"B+ c #A38546",
"0+ c #161707",
"}O c #16170A",
"B# c #161106",
"n# c #786947",
">  c #4D7170",
"b# c #7E7663",
"G# c #4D4D4D",
"2X c #F4FC0B",
",o c #F4FC0C",
",# c #E49F0C",
"c@ c #F4FC0D",
"D# c #575B04",
"xX c #4B4D08",
"E  c #6BDCDC",
"7o c #3A3B3C",
">. c #15262B",
" X c #7B7F0C",
"'@ c #7C5809",
"9O c #7B7F0F",
"&  c #4C9292",
"ao c #B1B80B",
"hX c #64542C",
" + c #B1B80E",
"h  c #4C6E6E",
"w  c #7DFDFD",
";O c #080900",
"c  c #7DFDFE",
"I# c #E7F10B",
"|. c #020201",
"n@ c #ABB113",
";o c #020202",
"jO c #D5DC0D",
"V. c #DBE30E",
"o# c #383B06",
"Ko c #383B07",
"8X c #5C5F0B",
"o@ c #8E733B",
"&o c #868A0B",
"=# c #5C5F19",
"GO c #0D0D03",
"b. c #ECF50D",
" # c #382F1E",
"w. c #5C7D4A",
"/. c #F8FD0C",
"O@ c #D7AC5D",
"7X c #3D3F07",
"O. c #63C3C3",
"[  c #69D0CF",
"M  c #6FE9E9",
"co c #73780A",
"]o c #6D710B",
"U+ c #7B6742",
":. c #253B2F",
"R  c #75EAEA",
">X c #979C0C",
"1# c #372606",
"ho c #7A5810",
"N+ c #BE9955",
"m+ c #D9E30A",
"r  c #508D8C",
"g# c #C1C70D",
"IO c #C7CE0E",
"no c #F1F90C",
"-@ c #EBF20D",
"yX c #F1F90D",
"A# c #1E1F04",
"'  c #5CBFC0",
"Ho c #1E1F05",
"$O c #242607",
"|X c #4E5100",
"Y+ c #505154",
"3  c #87FFFF",
")  c #62CCCC",
"f@ c #4E5108",
"eO c #4E5109",
"r# c #61633A",
"3# c #BC810D",
"+. c #74E7E7",
"o. c #4F9C9B",
"lO c #A2A70C",
"2. c #496B6B",
"pX c #A8AE10",
"L@ c #D8E00E",
"m. c #110E00",
"WX c #F6FD0C",
"a# c #E6A00D",
"%+ c #292A07",
"5@ c #110E08",
"aO c #FCFE0C",
"t# c #B57D03",
"X@ c #474D0F",
"~. c #5F630E",
"8  c #489898",
"Mo c #898E0C",
"M+ c #6B6B11",
"!O c #ADB20C",
"S# c #B3B90D",
"V# c #B9C00D",
"q  c #7FFEFE",
"V+ c #040302",
"4X c #0A0A02",
"lo c #0A0A03",
"^. c #DDE40D",
"<  c #85FFFF",
"F# c #0A0A0A",
"C@ c #EBA40C",
"OO c #0A0A0B",
"_. c #2E2E05",
"wX c #3A3C06",
"b  c #609696",
":@ c #6A6E0C",
":+ c #16181D",
"zo c #94990C",
"Fo c #706909",
"nO c #9AA010",
";+ c #6A6E17",
"6@ c #8A6D40",
"}+ c #E8EF06",
"-X c #EEF60C",
".. c #59B0B1",
"vo c #1B1C05",
"v+ c #AE8B43",
"f. c #F4F70D",
"5+ c #4B4E08",
"jo c #584114",
"|O c #A5AB03",
"X+ c #75790A",
"n  c #468C8C",
"R+ c #75790B",
"j. c #214140",
"T+ c #B1B909",
"e+ c #836F48",
"l# c #A5AB0E",
"B. c #B7BA07",
"7+ c #9FA410",
"C  c #7DFEFE",
"z@ c #020301",
"3o c #CFD60C",
"*# c #E7EC08",
"^X c #C9CF0F",
"a  c #83FFFF",
"UO c #F3FA0C",
"^o c #EDF30E",
">@ c #0E0B06",
".# c #202006",
"++ c #262706",
"J. c #203226",
"oX c #393939",
"t. c #807E00",
"d. c #868B0A",
".  c #646D6D",
"#X c #868B0E",
"h+ c #7A7D12",
"S  c #7CFBFB",
"T# c #D4DA09",
"z  c #517F7F",
"LO c #DAE10D",
"VO c #E0E80D",
"$# c #070707",
"KO c #F8FE0C",
"mo c #F8FE0D",
"#  c #387878",
",. c #2C5E6B",
"_o c #FEFF0C",
"q# c #FEFF0E",
"so c #3D400B",
"+@ c #1F1D0C",
"K+ c #31320C",
"Q  c #63A6A6",
"|@ c #1F1709",
"<. c #3E7F87",
"2  c #4A9F9E",
";# c #69655F",
".O c #979D10",
"-# c #564D38",
"^  c #7BF8F8",
"-  c #81FFFF",
">o c #EBF30C",
"*@ c #181903",
"9X c #E5EC0E",
",O c #EBF30F",
"[@ c #242706",
"G. c #363606",
"2+ c #424407",
"oO c #484B08",
";. c #18251D",
" O c #181910",
"7  c #5C7878",
"Jo c #6C6F0A",
"t+ c #B4BD08",
"J  c #74E8E8",
"`@ c #855E0A",
"G  c #6E8D8C",
"HX c #42441D",
"U  c #7AF5F5",
"-. c #7AF5F6",
"Y  c #7AEFEF",
"]O c #C6CC0D",
")O c #CCD30D",
"`+ c #CCD30F",
"Y# c #292B01",
"+  c #5BB7B7",
"aX c #F6FE0C",
"*O c #F0F70D",
"qO c #F6FE0D",
"Co c #232405",
" . c #5BB7BA",
"=@ c #232406",
"'# c #4F4F53",
"t@ c #FCFF0C",
"xo c #FCFF0D",
"E. c #FCFF0E",
"k  c #5B7B7A",
"N@ c #898F02",
"z. c #61D0DD",
"|  c #172223",
"WO c #D7DE05",
"i+ c #2F2615",
"m# c #D99607",
"x  c #54ADAD",
"]. c #CBD009",
",  c #7FFFFF",
"0o c #0A0B03",
"W@ c #040403",
")X c #0A0B04",
"Bo c #DDE50E",
"Q@ c #040404",
"E# c #777A1E",
"*+ c #040407",
"'X c #F5FB0C",
"v# c #B47B01",
"jX c #665C47",
".+ c #646809",
"y@ c #8E9304",
"$@ c #9AA109",
"e@ c #8E930B",
"&# c #BEC50C",
"Ro c #BEC50E",
"e. c #D0CE00",
"QX c #B8BE0E",
"go c #7C780F",
"GX c #EEF70B",
")o c #EEF70C",
"AO c #151603",
"+o c #DCE20D",
"AX c #E8F00E",
"i  c #5FCAC9",
"F. c #03070D",
"8@ c #FAFF0C",
"6X c #F4F80D",
"%o c #FAFF0D",
"kX c #403827",
"U. c #696C0E",
"g  c #77F8F8",
"K  c #77ECEB",
"H+ c #999E0F",
"Z+ c #C9D00A",
"6  c #7DFFFF",
"p# c #8E6308",
"`. c #CFD710",
"1X c #F3FB0C",
",X c #EDF40D",
"eX c #EDF40E",
"Vo c #202105",
"p+ c #202106",
"C+ c #505307",
"TX c #565A08",
"ro c #50530B",
"0  c #3F6B6B",
"wO c #AAB00B",
"W+ c #C59F54",
"5  c #7CFCFC",
"VX c #010100",
"5o c #808519",
"bX c #010101",
"fO c #010102",
"NO c #D4DB0C",
"9+ c #070803",
"D@ c #0D0F07",
"-+ c #F8FF0B",
"UX c #070809",
";X c #F8FF0C",
"&O c #F8FF0D",
"u# c #E8A20D",
"a. c #313304",
"4+ c #F8FF0E",
"PO c #31330B",
"P# c #554B05",
"%. c #75E6E7",
"pO c #B5BB0E",
"F  c #B3AEAE",
"YO c #B5BB0F",
"<@ c #50473E",
"~o c #E5ED0C",
"4O c #0C0C02",
"So c #E5ED0D",
"<+ c #D9DF0D",
"C. c #121303",
"<o c #DFE60D",
"xO c #E5ED0E",
"r@ c #121305",
"W. c #3C3E06",
"-o c #3C3E07",
"DX c #424508",
"Go c #747759",
"`  c #7B7B80",
"No c #6C7009",
"/o c #6C700B",
"0. c #437E7E",
"`# c #3C3E13",
"d@ c #B4BE06",
"3@ c #A8B00A",
"Z# c #C0C60C",
"}X c #CCD40E",
"$+ c #C0C610",
"]X c #F0F80C",
",@ c #6E7074",
"G+ c #F6FF0D",
"Q+ c #0B0906",
"h. c #1D1E07",
":# c #3C3220",
"p  c #67D8D8",
"6+ c #171713",
"X. c #6DD9D9",
"k# c #7D820B",
"A  c #73F2F1",
"7. c #716E0E",
"># c #D9970B",
"M# c #D9970C",
"JX c #665733",
"3O c #D1D80D",
"po c #F5FC0C",
"4o c #E9EE0C",
"K# c #E59F0C",
"To c #F5FC0E",
"{  c #60BCBC",
"L. c #FBFD0D",
"x@ c #585B08",
"J@ c #7E6738",
"u+ c #585B0A",
"U# c #82860B",
"l. c #16262E",
"2o c #888D0C",
"(o c #B2B80C",
"q. c #72EFF9",
"*o c #B8BF0D",
"E+ c #BB9255",
"_@ c #402D06",
"Z  c #4D7473",
"^+ c #59462D",
"{. c #B2B80F",
"}  c #7EFDFC",
"K. c #D0D508",
"~X c #090902",
"&@ c #0F1003",
"8# c #E2EA0E",
"(@ c #DE9B0D",
"O+ c #DCE30E",
"PX c #DEA115",
"L+ c #D6DC0F",
"## c #090906",
"Q. c #09090A",
"]  c #5FB9B9",
"{O c #09090B",
"SX c #333407",
"1@ c #DFB05E",
"'o c #696D09",
"H# c #644B17",
"7@ c #57580E",
"r+ c #4C3B27",
"(# c #93980A",
"!@ c #636611",
"y. c #515D1F",
"1  c #52AEAE",
"#O c #B7BC0C",
"F@ c #C9D10C",
"/O c #939815",
"D  c #58B5B5",
"tX c #EDF50C",
"=o c #EDF50D",
"K@ c #080604",
"@o c #1A1B04",
"}o c #EDF50E",
"<# c #E39F0E",
"y+ c #F9FD0B",
"DO c #F9FD0C",
"Y. c #2D2D2F",
">+ c #505406",
"i# c #694905",
"s@ c #886F39",
"_X c #141417",
"yO c #6E7109",
"HO c #141418",
"P. c #74780C",
"6o c #51452B",
"A@ c #1A1B1B",
"w+ c #CED504",
"t  c #455555",
"l+ c #A4AA0C",
"4  c #7CFDFD",
":X c #CED50E",
"'O c #F2F90C",
"l@ c #ECF20E",
"z# c #2B2D08",
"RX c #555809",
"n+ c #3D3C0D",
"S+ c #252011",
"^O c #1F1F21",
".X c #D9E007",
"l  c #56AFAF",
"#o c #060601",
"d+ c #D3D90B",
"7# c #D9E00D",
"). c #060603",
"&+ c #060606",
"(O c #F7FD0B",
"|o c #363806",
"Z@ c #5B410A",
"a@ c #B7924A",
"w# c #666A0B",
"%  c #6EDDDD",
"u. c #243B30",
")# c #8A8E09",
")@ c #C88C0A",
"I  c #498C8C",
"N# c #C88C0D",
"r. c #AEB208",
"%O c #BAC00C",
"mX c #B4B90D",
"8+ c #BAC00E",
"(X c #BAC00F",
"s+ c #EAF20B",
"(  c #80F8F8",
"o+ c #F0F90C",
"y# c #E09C0C",
"mO c #EAF20D",
"TO c #111105",
";  c #86FFFF",
"9o c #474A08",
"_O c #474A09",
"M. c #111111",
"&. c #67CDCD",
"n. c #5F6006",
"/  c #488988",
"+O c #6B6E12",
"!  c #79F4F4",
":O c #A1A70E",
"#+ c #CBD20E",
"/X c #F5FD0C",
"EX c #F5FD0D",
"Lo c #EFF60D",
"d# c #E5A00D",
"7O c #282A04",
"FX c #E9EF0E",
"'+ c #222304",
"y  c #60C3C3",
"_# c #4C4E07",
"lX c #413D34",
"Q# c #71510C",
"YX c #82870B",
"/+ c #76790D",
"2# c #2E1F07",
"oo c #342607",
"{@ c #A6AB0D",
"_  c #6C7171",
"j# c #030300",
"~  c #53A0A0",
"j  c #7EFEFE",
"j@ c #1B1F00",
"vO c #030301",
"iX c #030302",
"Uo c #030303",
":  c #59BFBF",
"S. c #DCE40E",
"P@ c #D6DD0E",
"Wo c #030305",
"0@ c #090A07",
"=  c #84FFFF",
"w@ c #F4FA0D",
"A+ c #58501C",
"z+ c #333508",
"qX c #63670B",
"h@ c #4C3C24",
"~+ c #645824",
"Ao c #696E0D",
"P+ c #BDC401",
"E@ c #8D920C",
"$o c #93990C",
"io c #BDC40C",
"q+ c #B1B60E",
"N  c #7DFBFB",
"Do c #8D8C0F",
"   c None",
"T. c #141500",
"D+ c #0E0E02",
"ko c #272728",
"kO c #0E0E03",
"IX c #946C11",
"I. c #0E0E04",
"KX c #A0740D",
"j+ c #F9FE0B",
"H  c #5EB7B6",
">O c #F9FE0C",
"QO c #838476",
"[o c #3E4006",
"!+ c #505506",
"3X c #6E7200",
"sX c #FFFF0B",
"S@ c #766138",
"v. c #FFFF0C",
"6. c #FFFF0D",
"cX c #FFFF0E",
"fX c #2C1F02",
"Yo c #6E720A",
"L  c #70DEDE",
"8. c #1A343E",
".@ c #7C6E53",
"f  c #7CFEFE",
"SO c #C2C80D",
"`o c #989D14",
"x+ c #C2C80E",
"e  c #82FFFF",
"2O c #F2FA0C",
"MO c #F2FA0D",
"R. c #ECF30E",
"BX c #252704",
"#. c #504F23",
"Oo c #252705",
"I+ c #AC8842",
"x. c #50795D",
"/@ c #744F02",
"^@ c #797D09",
"gO c #797D10",
"sO c #A9AF0C",
"4# c #CF910D",
/* pixels */
"                                                                                                ",
"                                            . X                     o O + @                     ",
"                                          # $ % &                 * = - ; :                     ",
"                                        > - , , < 1             2 3 4 5 4 6 7                   ",
"                                        8 = 5 5 4 - 9         0 q w 5 5 5 e r                   ",
"                                      t y u 5 5 5 - i         p u 5 5 5 5 a s                   ",
"                                      t d u 5 5 5 f g       h j 4 5 5 5 4 6 k                   ",
"                                        l e 5 5 5 5 u z     x a c 5 5 5 - v                     ",
"                                        b u 5 5 5 5 a n   m M N B V C 4 V Z                     ",
"                                          A j S 4 S e D F G e H J K L ; P                       ",
"                                          I = U Y T R E W Q ! ~ ^ / ( ) _                       ",
"                                          ` ' = ] [ { } |  ...X.o.O.+.t                         ",
"                                        @.#.$.%.&.*.=.-.;.:.>.,.<.1.2.                          ",
"                              t t 3.4.4.5.6.7.8.B 9.0.q.w.e.r.t.y.u.i.                          ",
"                          p.a.4.4.4.s.d.f.g.h.4.j.k.l.z.x.c.v.b.n.m.4.M.t                       ",
"                      N.B.V.C.4.4.Z.A.v.S.D.4.4.4.F.G.H.J.K.L.P.4.4.4.4.I.U.t                   ",
"                  Y.T.R.E.W.4.Q.!.~.^./.(.).4.4.4._.`.'.].[.{.}.4.4.|.4. X6..XXX                ",
"                oX4.OX6.+X@X4.4.4.#X6.$X%X|.4.4.&X*X=X-X;X:XD.4.4.4.h.>X,X<X1X2X3X              ",
"                4.4X5X6X7X4.4.@X8X9X0XqX4.4.4.4.wXeX-XrXtXyXuX4.4.4.iXpXaXrXrX<XsXdX            ",
"  fXgXhXjXkXlXzX4.xXcXvXbX4.4.nXmXMXNXBX4.4.4.VXCXZXrXrX<XAXSX4.4.4.DXFX-XrXrXtX$XGXHX          ",
"    JXKXLXPXIXUX4.YXcXTX4.4.4.RXEXWXQX!X4.4.4.~X^X1XrXrX/X(X)X4.bX_X`X'XtXrX]X[X{X}X|X o        ",
"        .oXooo4.Oo+oAX@o4.4.#o$oEX%o&o4.4.4.4.@X*oaXrX-X=o-o|.4.4.;o:o=X>o,o<o1o2o3o4o5ot 6o7o  ",
"            8o4.9ocX+X|.4.4.0oqo1Xwoeo|.4.4.@Xroto]XrXtXyouo4.4.4.s.io/Xpoaosodo0Xfogohojot     ",
"            kolozoxoco4.4.4.vobonomoMo|.4.4.4.No0XrXrXnoBoVo4.4.4.Co^.Zo^.AoroSo<X1XDoFoGo      ",
"            4.HoAXxoJo4.4.4.KoLo-XPoIo4.4.4.UoYoTotXrX1XRoEo4.4.WoQo!ono~o^o/o(oMXrX)o_o`o      ",
"            4.'o%o0X]o4.4.4.[o{o]X}o|o4.4.4. O.O'XrXtXXOoO4.4.4.OO+O@O]XtX;X#O$O%O&O[.*O=O-O    ",
"            ;O:O>O,O<O4.4.;o1O%o2O3O4O4.4.4.#o5O/XrX-X6O7O4.4.4.8O9O0OtXrXrXqOwOeONorOtOyOuO    ",
"            iOpOaOsOdO4.4.fOgOhO2OjOkO4.4.4.@XlO;XrXzOxOcOvO4.4.bOnOmO-XrXrXrXaXMONOBOVOCOZO    ",
"            AOSODOFOGO4.bXHOJOKOnoLOHo4.4.4.POIOUOrX,oYOTOVX4.4.ROEOnorXrXrXrXrXtX<X]X1XWOQO    ",
"            4.!O6.~O4.4.bX^O/O(O1X)OkO4.4.4._O`O]XrX'O]O[O4.4.{O}O(XZorXrXrXrXrXrXrXrX;X|O      ",
"            4. +6..+4.4.4.4.X+6.o+O+++4.4.4.@+#+1XrX]X$+%+4.4.&+*+=+KOrXrXrXrXrXrXrXrX-+;+      ",
"            :+>+,+<+1+4.4.4.2+3+Zo4+5+4.4.4.6+7+WXrX,o8+9+4.4.4.0+q+/XrXrXrXrXrXrXrXUOw+t       ",
"            e+r+t+y+u+i+iX4.p+a+s+d+f+dO4.bXg+h+j+rXk+l+@X4.4.4.z+x+UOrXrXrXrXrXrXrXc+t         ",
"          v+b+n+m+M+N+B+V+4.C+Z+A+S+sOD+4.4.4.F+G+tXyXH+J+4.4.bXK+L+norXrXrXrXrXrXhOP+          ",
"        I+U+Y+T+R+E+W+Q+4.VX!+~+^+/+(+4.4.4.)+_+=XrXrX`+'+4.4.]+[+{+EXrXrXrXrXrX]X}+|+          ",
"       @.@    X@o@O@+@4.4.4.@@#@$@6.%@&@4.4.VX*@$X2Oo+xO=@4.4.4.4.7X-@-XrXrXrXtX;@:@            ",
"      >@,@    <@1@2@3@4@4.5@6@7@8@no9@vo4.4.4.0@q@WXtXw@e@VX4.3.;or@do2OrXrXtXt@y@              ",
"    u@i@p@    a@s@d@cXf@4.g@h@j@k@2Ol@{X4.4.4.z@x@c@tX8@v@4.4.4.4.b@n@m@rX<XM@N@                ",
"    B@V@C@Z@A@S@D@F@6.G@H@J@K@D+L@no1XP@I@4.4.U@Y@T@]XR@E@W@Q@4.4.U@!@_o2O~@^@                  ",
"    /@(@)@_@`@'@]@[@{@}@|@ #4..#X#<X-X}oo#4.4.U@O#{.m@+#@###$#4.4.;o%#&#*#=#                    ",
"      -#;#:#>#,#<#1#2#3#4#5#@X6#7#no]X8#++4.4.4.9#0#aXrXq#w#4.4.4.4.4.e#r#                      ",
"            t#y#u#i#p#a#s#d#f#U@g#/XrX%oh#4.4.4.j#k#=o-Xpol#z#4.4.4.4.                          ",
"            x#c#v#b#n#m#M#M#N#B#V#C#rX'XZ#A#4.4.4.4.S#M@aX'OD#F#G#                              ",
"                      H#J#K#L#P#I#t@8@;X6.U#4.4.4.4.Y#T#R#E#                                    ",
"                        W#Q#!#  ~#^#/#(#)#_#4.4.4.`#'#                                          ",
"                                                                                                ",
"                                                                                                ",
"                                                                                                "
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
  SetIcon(GetIconResource(wxT("fatbee_v2.ico")));
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
  m_wxChkActive->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Sans")));
  itemFlexGridSizer7->Add(m_wxChkActive, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( itemDialog1, wxID_STATIC, _("Name :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText10->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  m_wxStrName = new wxTextCtrl;
  m_wxStrName->Create( itemDialog1, ID_StrName, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrName->SetToolTip(_("All transmission event should have a \n\n descriptive name. \n\nSet this name here."));
  itemFlexGridSizer7->Add(m_wxStrName, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText14 = new wxStaticText;
  itemStaticText14->Create( itemDialog1, wxID_STATIC, _("Class :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText14->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  wxArrayString m_wxComboClassStrings;
  m_wxComboClass = new wxComboBox;
  m_wxComboClass->Create( itemDialog1, ID_StrVscpClass, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboClassStrings, wxCB_READONLY );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxComboClass->SetToolTip(_("Select the VSCP class for the \ntransmission event here."));
  itemFlexGridSizer7->Add(m_wxComboClass, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemDialog1, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText16->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  wxArrayString m_wxComboTypeStrings;
  m_wxComboType = new wxComboBox;
  m_wxComboType->Create( itemDialog1, ID_StrVscpType, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboTypeStrings, wxCB_READONLY );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxComboType->SetToolTip(_("Select the VSCP type for the \ntransmission event here."));
  itemFlexGridSizer7->Add(m_wxComboType, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText18 = new wxStaticText;
  itemStaticText18->Create( itemDialog1, wxID_STATIC, _("Priority :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText18->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

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
  itemStaticText23->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText23, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 0);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  m_wxChkDefaultGUID = new wxCheckBox;
  m_wxChkDefaultGUID->Create( itemDialog1, ID_CHKBOX_USE_DEFAULT, _("Use default GUID"), wxDefaultPosition, wxDefaultSize, 0 );
  m_wxChkDefaultGUID->SetValue(true);
  m_wxChkDefaultGUID->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));
  itemFlexGridSizer7->Add(m_wxChkDefaultGUID, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText26 = new wxStaticText;
  itemStaticText26->Create( itemDialog1, wxID_STATIC, _("15 - 8 :"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  itemStaticText26->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

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
  itemStaticText36->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

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
  itemStaticText46->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  m_wxStrData = new wxTextCtrl;
  m_wxStrData->Create( itemDialog1, ID_StrVscpData, wxEmptyString, wxDefaultPosition, wxSize(-1, 50), wxTE_MULTILINE );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrData->SetToolTip(_("You enter data for the transmission event\n as a comma separated list. \nYou can arrange the data over several lines. \nHexadecimal values should be preceded with '0x'."));
  itemFlexGridSizer7->Add(m_wxStrData, 5, wxGROW|wxALIGN_TOP|wxALL, 1);

  itemFlexGridSizer7->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  itemFlexGridSizer7->Add(5, 20, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

  wxStaticText* itemStaticText50 = new wxStaticText;
  itemStaticText50->Create( itemDialog1, wxID_STATIC, _("Count :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText50->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText50, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  m_wxStrCount = new wxTextCtrl;
  m_wxStrCount->Create( itemDialog1, ID_StrCount, _("1"), wxDefaultPosition, wxSize(60, 25), 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrCount->SetToolTip(_("Set event transmission event here. \nThis value sets how many times \nthis transmission event will be sent \nwhen triggered or doubleclicked."));
  itemFlexGridSizer7->Add(m_wxStrCount, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

  wxStaticText* itemStaticText52 = new wxStaticText;
  itemStaticText52->Create( itemDialog1, wxID_STATIC, _("Period :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText52->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  m_wxStrPeriod = new wxTextCtrl;
  m_wxStrPeriod->Create( itemDialog1, ID_StrPeriod, _("0"), wxDefaultPosition, wxSize(60, 25), 0 );
  if (dlgVSCPTrmitElement::ShowToolTips())
    m_wxStrPeriod->SetToolTip(_("Set transmission event period here expressed as milliseconds. \nThe transmission event period is  interval between automatically sent events."));
  itemFlexGridSizer7->Add(m_wxStrPeriod, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

  wxStaticText* itemStaticText54 = new wxStaticText;
  itemStaticText54->Create( itemDialog1, wxID_STATIC, _("Trigger :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText54->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemFlexGridSizer7->Add(itemStaticText54, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

  wxArrayString m_wxComboTriggerStrings;
  m_wxComboTrigger = new wxComboBox;
  m_wxComboTrigger->Create( itemDialog1, ID_ComboTrigger, wxEmptyString, wxDefaultPosition, wxSize(200, 25), m_wxComboTriggerStrings, wxCB_DROPDOWN );
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
  if (name == wxT("fatbee_v2.ico"))
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
    pObj->m_count = vscp_readStringValue( str );  
    str = m_wxStrPeriod->GetValue();
    pObj->m_period = vscp_readStringValue( str ); 
    str = m_wxComboTrigger->GetValue();
    pObj->m_trigger = vscp_readStringValue( str );  
  
	pObj->m_Event.head = ( m_wxChoicePriority->GetCurrentSelection() << 5 );

    pObj->m_Event.timestamp = 0;
    if ( wxNOT_FOUND == m_wxComboClass->GetSelection() ) {
        str = m_wxComboClass->GetValue();
        pObj->m_Event.vscp_class = vscp_readStringValue( str );
    }
    else {
        pObj->m_Event.vscp_class = 
        (uintptr_t)m_wxComboClass->GetClientData( m_wxComboClass->GetSelection() );
    }
  
    if ( wxNOT_FOUND == m_wxComboType->GetSelection() ) {
        str = m_wxComboType->GetValue();
        pObj->m_Event.vscp_type = vscp_readStringValue( str );
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
        pObj->m_Event.GUID[ 0 ] = vscp_readStringValue( str );
        str = m_wxStrGUID1->GetValue();
        pObj->m_Event.GUID[ 1 ] = vscp_readStringValue( str );
        str = m_wxStrGUID2->GetValue();
        pObj->m_Event.GUID[ 2 ] = vscp_readStringValue( str );
        str = m_wxStrGUID3->GetValue();
        pObj->m_Event.GUID[ 3 ] = vscp_readStringValue( str );
        str = m_wxStrGUID4->GetValue();
        pObj->m_Event.GUID[ 4 ] = vscp_readStringValue( str );
        str = m_wxStrGUID5->GetValue();
        pObj->m_Event.GUID[ 5 ] = vscp_readStringValue( str );
        str = m_wxStrGUID6->GetValue();
        pObj->m_Event.GUID[ 6 ] = vscp_readStringValue( str );
        str = m_wxStrGUID7->GetValue();
        pObj->m_Event.GUID[ 7 ] = vscp_readStringValue( str );
        str = m_wxStrGUID8->GetValue();
        pObj->m_Event.GUID[ 8 ] = vscp_readStringValue( str );
        str = m_wxStrGUID9->GetValue();
        pObj->m_Event.GUID[ 9 ] = vscp_readStringValue( str );
        str = m_wxStrGUID10->GetValue();
        pObj->m_Event.GUID[ 10 ] = vscp_readStringValue( str );
        str = m_wxStrGUID11->GetValue();
        pObj->m_Event.GUID[ 11 ] = vscp_readStringValue( str );
        str = m_wxStrGUID12->GetValue();
        pObj->m_Event.GUID[ 12 ] = vscp_readStringValue( str );
        str = m_wxStrGUID13->GetValue();
        pObj->m_Event.GUID[ 13 ] = vscp_readStringValue( str );
        str = m_wxStrGUID14->GetValue();
        pObj->m_Event.GUID[ 14 ] = vscp_readStringValue( str );
        str = m_wxStrGUID15->GetValue();
        pObj->m_Event.GUID[ 15 ] = vscp_readStringValue( str );
    }
  
    // Data is allowed to scan multiple lines and to be in hex
    // or decimal form
    str = m_wxStrData->GetValue();
    vscp_getVscpDataFromString( &pObj->m_Event, str );
  
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
	vscp_writeVscpDataToString( &pObj->m_Event, str );
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
