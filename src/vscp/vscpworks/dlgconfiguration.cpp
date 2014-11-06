/////////////////////////////////////////////////////////////////////////////
// Name:        dlgconfiguration.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 18:30:18 CEST
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
#pragma implementation "dlgconfiguration.h"
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
#include "wx/bookctrl.h"
////@end includes

#include <wx/colordlg.h>
#include <wx/imaglist.h>

#include "vscpworks.h"
#include "dlgconfiguration.h"

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

////@end XPM images

extern appConfiguration g_Config;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgConfiguration, wxPropertySheetDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration event table definition
//

BEGIN_EVENT_TABLE( dlgConfiguration, wxPropertySheetDialog )

////@begin dlgConfiguration event table entries
  EVT_BUTTON( ID_BUTTON4, dlgConfiguration::OnButtonSetRxGridTextColourClick )
  EVT_BUTTON( ID_BUTTON5, dlgConfiguration::SetRxGridBackgroundColourClick )
  EVT_BUTTON( ID_BUTTON6, dlgConfiguration::OnButtonSetTxGridTextColourClick )
  EVT_BUTTON( ID_BUTTON7, dlgConfiguration::OnButtonSetTxGridBackgroundColourClick )
////@end dlgConfiguration event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration constructors
//

dlgConfiguration::dlgConfiguration()
{
    Init();
}

dlgConfiguration::dlgConfiguration( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration creator
//

bool dlgConfiguration::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    ////@begin dlgConfiguration creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  SetSheetStyle(wxPROPSHEET_DEFAULT);
  wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

  CreateButtons(wxOK|wxCANCEL|wxHELP);
  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  LayoutDialog();
  Centre();
    ////@end dlgConfiguration creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgConfiguration destructor
//

dlgConfiguration::~dlgConfiguration()
{
    ////@begin dlgConfiguration destruction
    ////@end dlgConfiguration destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgConfiguration::Init()
{
    ////@begin dlgConfiguration member initialisation
  m_labelLogFile = NULL;
  m_checkEnableLogging = NULL;
  m_comboLogLevel = NULL;
  m_checkConfirmDeletes = NULL;
  m_comboNumericalBase = NULL;
  m_maxRetries = NULL;
  m_readTimeout = NULL;
  m_ChkAutoScroll = NULL;
  m_chkPyjamasLook = NULL;
  m_chkUseSymbols = NULL;
  m_editRxForeGroundColour = NULL;
  m_editRxBackGroundColour = NULL;
  m_editTxForeGroundColour = NULL;
  m_editTxBackGroundColour = NULL;
    ////@end dlgConfiguration member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgConfiguration
//

void dlgConfiguration::CreateControls()
{    
    ////@begin dlgConfiguration content construction
  dlgConfiguration* itemPropertySheetDialog1 = this;

  wxImageList* itemPropertySheetDialog1ImageList = new wxImageList(16, 16, true, 5);
  {
    wxIcon itemPropertySheetDialog1Icon0(GetIconResource(wxT("fatbee_v2.ico")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon0);
    wxIcon itemPropertySheetDialog1Icon1(GetIconResource(wxT("fatbee_v2.ico")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon1);
    wxIcon itemPropertySheetDialog1Icon2(GetIconResource(wxT("fatbee_v2.ico")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon2);
    wxIcon itemPropertySheetDialog1Icon3(GetIconResource(wxT("fatbee_v2.ico")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon3);
    wxIcon itemPropertySheetDialog1Icon4(GetIconResource(wxT("copy.xpm")));
    itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon4);
  }
  GetBookCtrl()->AssignImageList(itemPropertySheetDialog1ImageList);

  wxPanel* itemPanel2 = new wxPanel;
  itemPanel2->Create( GetBookCtrl(), ID_PANEL_GENERAL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  itemPanel2->SetName(wxT("general"));
  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemPanel2->SetSizer(itemBoxSizer3);

  wxGridSizer* itemGridSizer4 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer3->Add(itemGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText5 = new wxStaticText;
  itemStaticText5->Create( itemPanel2, wxID_STATIC, _("Logfile :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_labelLogFile = new wxStaticText;
  m_labelLogFile->Create( itemPanel2, wxID_STATIC, _("---"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer6->Add(m_labelLogFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText8 = new wxStaticText;
  itemStaticText8->Create( itemPanel2, wxID_STATIC, _("Enable logging :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_checkEnableLogging = new wxCheckBox;
  m_checkEnableLogging->Create( itemPanel2, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_checkEnableLogging->SetValue(false);
  itemBoxSizer9->Add(m_checkEnableLogging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemPanel2, wxID_STATIC, _("Log Level :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxArrayString m_comboLogLevelStrings;
  m_comboLogLevelStrings.Add(_("DEBUG - Highest"));
  m_comboLogLevelStrings.Add(_("INFO"));
  m_comboLogLevelStrings.Add(_("NOTICE"));
  m_comboLogLevelStrings.Add(_("WARNING"));
  m_comboLogLevelStrings.Add(_("ERROR"));
  m_comboLogLevelStrings.Add(_("CRITICAL"));
  m_comboLogLevelStrings.Add(_("ALERT"));
  m_comboLogLevelStrings.Add(_("EMERGENCY . Lowest"));
  m_comboLogLevel = new wxChoice;
  m_comboLogLevel->Create( itemPanel2, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, m_comboLogLevelStrings, 0 );
  itemBoxSizer12->Add(m_comboLogLevel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemGridSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemPanel2, wxID_STATIC, _("Confirm deletes :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_checkConfirmDeletes = new wxCheckBox;
  m_checkConfirmDeletes->Create( itemPanel2, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  m_checkConfirmDeletes->SetValue(false);
  itemBoxSizer17->Add(m_checkConfirmDeletes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText19 = new wxStaticText;
  itemStaticText19->Create( itemPanel2, wxID_STATIC, _("Base to use for numbers :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer4->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer4->Add(itemBoxSizer20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  wxArrayString m_comboNumericalBaseStrings;
  m_comboNumericalBaseStrings.Add(_("Hexadecimal"));
  m_comboNumericalBaseStrings.Add(_("Decimal"));
  m_comboNumericalBase = new wxChoice;
  m_comboNumericalBase->Create( itemPanel2, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_comboNumericalBaseStrings, 0 );
  itemBoxSizer20->Add(m_comboNumericalBase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  GetBookCtrl()->AddPage(itemPanel2, _("General"), false, 0);

  wxPanel* itemPanel22 = new wxPanel;
  itemPanel22->Create( GetBookCtrl(), ID_PANEL_COMMUNICATION, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
  itemPanel22->SetSizer(itemBoxSizer23);

  wxGridSizer* itemGridSizer24 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer23->Add(itemGridSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText25 = new wxStaticText;
  itemStaticText25->Create( itemPanel22, wxID_STATIC, _("Max number of register read/write retries :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer24->Add(itemStaticText25, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer24->Add(itemBoxSizer26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_maxRetries = new wxSpinCtrl;
  m_maxRetries->Create( itemPanel22, ID_SPINCTRL, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 5, 1 );
  itemBoxSizer26->Add(m_maxRetries, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText28 = new wxStaticText;
  itemStaticText28->Create( itemPanel22, wxID_STATIC, _("Register read/write timeout in seconds :"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer24->Add(itemStaticText28, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer24->Add(itemBoxSizer29, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_readTimeout = new wxSpinCtrl;
  m_readTimeout->Create( itemPanel22, ID_SPINCTRL1, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 1 );
  itemBoxSizer29->Add(m_readTimeout, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel22, _("Communication"), false, 1);

  wxPanel* itemPanel31 = new wxPanel;
  itemPanel31->Create( GetBookCtrl(), ID_PANEL_COLORS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxHORIZONTAL);
  itemPanel31->SetSizer(itemBoxSizer32);

  GetBookCtrl()->AddPage(itemPanel31, _("Colours"), false, 2);

  wxPanel* itemPanel33 = new wxPanel;
  itemPanel33->Create( GetBookCtrl(), ID_PANEL_RECEIVE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
  itemPanel33->SetSizer(itemBoxSizer34);

  wxGridSizer* itemGridSizer35 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer34->Add(itemGridSizer35, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_ChkAutoScroll = new wxCheckBox;
  m_ChkAutoScroll->Create( itemPanel33, ID_ChkAutoScroll, _("Autoscroll"), wxDefaultPosition, wxDefaultSize, 0 );
  m_ChkAutoScroll->SetValue(true);
  itemGridSizer35->Add(m_ChkAutoScroll, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_chkPyjamasLook = new wxCheckBox;
  m_chkPyjamasLook->Create( itemPanel33, ID_ChkPyjamasLook, _("Pyjamas Look"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkPyjamasLook->SetValue(true);
  itemGridSizer35->Add(m_chkPyjamasLook, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  m_chkUseSymbols = new wxCheckBox;
  m_chkUseSymbols->Create( itemPanel33, ID_CHECKBOX, _("Use symbols for class and type"), wxDefaultPosition, wxDefaultSize, 0 );
  m_chkUseSymbols->SetValue(true);
  itemGridSizer35->Add(m_chkUseSymbols, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  itemGridSizer35->Add(0, 0, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText44 = new wxStaticText;
  itemStaticText44->Create( itemPanel33, wxID_STATIC, _("Text colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText44, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer45 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer45, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editRxForeGroundColour = new wxTextCtrl;
  m_editRxForeGroundColour->Create( itemPanel33, ID_EditRxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer45->Add(m_editRxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton47 = new wxButton;
  itemButton47->Create( itemPanel33, ID_BUTTON4, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer45->Add(itemButton47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText48 = new wxStaticText;
  itemStaticText48->Create( itemPanel33, wxID_STATIC, _("Background colour for receive event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText48, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer49, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editRxBackGroundColour = new wxTextCtrl;
  m_editRxBackGroundColour->Create( itemPanel33, ID_EditRxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer49->Add(m_editRxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton51 = new wxButton;
  itemButton51->Create( itemPanel33, ID_BUTTON5, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer49->Add(itemButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText52 = new wxStaticText;
  itemStaticText52->Create( itemPanel33, wxID_STATIC, _("Text colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer53 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer53, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editTxForeGroundColour = new wxTextCtrl;
  m_editTxForeGroundColour->Create( itemPanel33, ID_EditTxForeGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer53->Add(m_editTxForeGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton55 = new wxButton;
  itemButton55->Create( itemPanel33, ID_BUTTON6, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer53->Add(itemButton55, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText56 = new wxStaticText;
  itemStaticText56->Create( itemPanel33, wxID_STATIC, _("Background colour for transmission event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer35->Add(itemStaticText56, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer35->Add(itemBoxSizer57, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  m_editTxBackGroundColour = new wxTextCtrl;
  m_editTxBackGroundColour->Create( itemPanel33, ID_EditTxBackGroundColour, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY );
  itemBoxSizer57->Add(m_editTxBackGroundColour, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton59 = new wxButton;
  itemButton59->Create( itemPanel33, ID_BUTTON7, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer57->Add(itemButton59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer60 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer34->Add(itemBoxSizer60, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxBoxSizer* itemBoxSizer61 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton62 = new wxButton;
  itemButton62->Create( itemPanel33, ID_BUTTON, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer61->Add(itemButton62, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton63 = new wxButton;
  itemButton63->Create( itemPanel33, ID_BUTTON1, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer61->Add(itemButton63, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer64 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText65 = new wxStaticText;
  itemStaticText65->Create( itemPanel33, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer64->Add(itemStaticText65, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString itemCheckListBox66Strings;
  itemCheckListBox66Strings.Add(_("Dir"));
  itemCheckListBox66Strings.Add(_("Time"));
  itemCheckListBox66Strings.Add(_("GUID"));
  itemCheckListBox66Strings.Add(_("Class"));
  itemCheckListBox66Strings.Add(_("Type"));
  itemCheckListBox66Strings.Add(_("Head"));
  itemCheckListBox66Strings.Add(_("Data Count"));
  itemCheckListBox66Strings.Add(_("Data"));
  itemCheckListBox66Strings.Add(_("Timestamp"));
  itemCheckListBox66Strings.Add(_("Note"));
  wxCheckListBox* itemCheckListBox66 = new wxCheckListBox;
  itemCheckListBox66->Create( itemPanel33, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, itemCheckListBox66Strings, wxLB_SINGLE );
  itemBoxSizer64->Add(itemCheckListBox66, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton68 = new wxButton;
  itemButton68->Create( itemPanel33, ID_BUTTON3, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer67->Add(itemButton68, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton69 = new wxButton;
  itemButton69->Create( itemPanel33, ID_BUTTON8, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer67->Add(itemButton69, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer60->Add(itemBoxSizer70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText71 = new wxStaticText;
  itemStaticText71->Create( itemPanel33, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer70->Add(itemStaticText71, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString itemCheckListBox72Strings;
  itemCheckListBox72Strings.Add(_("Dir"));
  itemCheckListBox72Strings.Add(_("Time"));
  itemCheckListBox72Strings.Add(_("GUID"));
  itemCheckListBox72Strings.Add(_("Class"));
  itemCheckListBox72Strings.Add(_("Type"));
  itemCheckListBox72Strings.Add(_("Head"));
  itemCheckListBox72Strings.Add(_("Data Count"));
  itemCheckListBox72Strings.Add(_("Data"));
  itemCheckListBox72Strings.Add(_("Timestamp"));
  itemCheckListBox72Strings.Add(_("Note"));
  wxCheckListBox* itemCheckListBox72 = new wxCheckListBox;
  itemCheckListBox72->Create( itemPanel33, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, itemCheckListBox72Strings, wxLB_SINGLE );
  itemBoxSizer70->Add(itemCheckListBox72, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel33, _("VSCP Receive View"), false, 3);

  wxPanel* itemPanel73 = new wxPanel;
  itemPanel73->Create( GetBookCtrl(), ID_PANEL_TRANSMIT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
  wxBoxSizer* itemBoxSizer74 = new wxBoxSizer(wxVERTICAL);
  itemPanel73->SetSizer(itemBoxSizer74);

  wxGridSizer* itemGridSizer75 = new wxGridSizer(24, 2, 0, 0);
  itemBoxSizer74->Add(itemGridSizer75, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);
  wxStaticText* itemStaticText76 = new wxStaticText;
  itemStaticText76->Create( itemPanel73, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText76, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer77 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer77, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl78 = new wxTextCtrl;
  itemTextCtrl78->Create( itemPanel73, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer77->Add(itemTextCtrl78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton79 = new wxButton;
  itemButton79->Create( itemPanel73, ID_BUTTON2, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer77->Add(itemButton79, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText80 = new wxStaticText;
  itemStaticText80->Create( itemPanel73, wxID_STATIC, _("Background colour for event table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText80, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer81 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer81, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl82 = new wxTextCtrl;
  itemTextCtrl82->Create( itemPanel73, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer81->Add(itemTextCtrl82, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton83 = new wxButton;
  itemButton83->Create( itemPanel73, ID_BUTTON9, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer81->Add(itemButton83, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText84 = new wxStaticText;
  itemStaticText84->Create( itemPanel73, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText84, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer85 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer85, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl86 = new wxTextCtrl;
  itemTextCtrl86->Create( itemPanel73, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer85->Add(itemTextCtrl86, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton87 = new wxButton;
  itemButton87->Create( itemPanel73, ID_BUTTON10, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer85->Add(itemButton87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxStaticText* itemStaticText88 = new wxStaticText;
  itemStaticText88->Create( itemPanel73, wxID_STATIC, _("Background colour for transmission table:"), wxDefaultPosition, wxDefaultSize, 0 );
  itemGridSizer75->Add(itemStaticText88, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer89 = new wxBoxSizer(wxHORIZONTAL);
  itemGridSizer75->Add(itemBoxSizer89, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
  wxTextCtrl* itemTextCtrl90 = new wxTextCtrl;
  itemTextCtrl90->Create( itemPanel73, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
  itemBoxSizer89->Add(itemTextCtrl90, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxButton* itemButton91 = new wxButton;
  itemButton91->Create( itemPanel73, ID_BUTTON11, _("..."), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer89->Add(itemButton91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer92 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer74->Add(itemBoxSizer92, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  wxBoxSizer* itemBoxSizer93 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer93, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton94 = new wxButton;
  itemButton94->Create( itemPanel73, ID_BUTTON12, _("Up"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer93->Add(itemButton94, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton95 = new wxButton;
  itemButton95->Create( itemPanel73, ID_BUTTON13, _("Down"), wxDefaultPosition, wxSize(60, -1), 0 );
  itemBoxSizer93->Add(itemButton95, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer96 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer96, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText97 = new wxStaticText;
  itemStaticText97->Create( itemPanel73, wxID_STATIC, _("Visible fields"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer96->Add(itemStaticText97, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString itemCheckListBox98Strings;
  itemCheckListBox98Strings.Add(_("Dir"));
  itemCheckListBox98Strings.Add(_("Time"));
  itemCheckListBox98Strings.Add(_("GUID"));
  itemCheckListBox98Strings.Add(_("Class"));
  itemCheckListBox98Strings.Add(_("Type"));
  itemCheckListBox98Strings.Add(_("Head"));
  itemCheckListBox98Strings.Add(_("Data Count"));
  itemCheckListBox98Strings.Add(_("Data"));
  itemCheckListBox98Strings.Add(_("Timestamp"));
  itemCheckListBox98Strings.Add(_("Note"));
  wxCheckListBox* itemCheckListBox98 = new wxCheckListBox;
  itemCheckListBox98->Create( itemPanel73, ID_CHECKLISTBOX2, wxDefaultPosition, wxDefaultSize, itemCheckListBox98Strings, wxLB_SINGLE );
  itemBoxSizer96->Add(itemCheckListBox98, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer99 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer99, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxButton* itemButton100 = new wxButton;
  itemButton100->Create( itemPanel73, ID_BUTTON14, _("-->"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer99->Add(itemButton100, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton101 = new wxButton;
  itemButton101->Create( itemPanel73, ID_BUTTON15, _("<--"), wxDefaultPosition, wxSize(40, -1), 0 );
  itemBoxSizer99->Add(itemButton101, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer92->Add(itemBoxSizer102, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  wxStaticText* itemStaticText103 = new wxStaticText;
  itemStaticText103->Create( itemPanel73, wxID_STATIC, _("Hidden fields"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer102->Add(itemStaticText103, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString itemCheckListBox104Strings;
  itemCheckListBox104Strings.Add(_("Dir"));
  itemCheckListBox104Strings.Add(_("Time"));
  itemCheckListBox104Strings.Add(_("GUID"));
  itemCheckListBox104Strings.Add(_("Class"));
  itemCheckListBox104Strings.Add(_("Type"));
  itemCheckListBox104Strings.Add(_("Head"));
  itemCheckListBox104Strings.Add(_("Data Count"));
  itemCheckListBox104Strings.Add(_("Data"));
  itemCheckListBox104Strings.Add(_("Timestamp"));
  itemCheckListBox104Strings.Add(_("Note"));
  wxCheckListBox* itemCheckListBox104 = new wxCheckListBox;
  itemCheckListBox104->Create( itemPanel73, ID_CHECKLISTBOX3, wxDefaultPosition, wxDefaultSize, itemCheckListBox104Strings, wxLB_SINGLE );
  itemBoxSizer102->Add(itemCheckListBox104, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  GetBookCtrl()->AddPage(itemPanel73, _("VSCP Transmission View"), false, 4);

  // Connect events and objects
  itemPanel33->Connect(ID_PANEL_RECEIVE, wxEVT_INIT_DIALOG, wxInitDialogEventHandler(dlgConfiguration::OnInitDialog), NULL, this);
    ////@end dlgConfiguration content construction


    ////////////////////////////////////////////////////////////////////
    //                    Init dialog data
    ////////////////////////////////////////////////////////////////////
    if ( g_Config.m_bAutoscollRcv ) {
        m_ChkAutoScroll->SetValue( 1 );
    }
    else {
        m_ChkAutoScroll->SetValue( 0 );
    }

    // Pyjamas look
    if ( g_Config.m_VscpRcvFrameRxbPyamas ) {
        m_chkPyjamasLook->SetValue( 1 );
    }
    else {
        m_chkPyjamasLook->SetValue( 0 );
    }

    // Symbolic Names
    if ( g_Config.m_UseSymbolicNames ) {
        m_chkUseSymbols->SetValue( 1 );
    }
    else {
        m_chkUseSymbols->SetValue( 0 );
    }  

    m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameRxTextColour.Red(), 
        g_Config.m_VscpRcvFrameRxTextColour.Green(),
        g_Config.m_VscpRcvFrameRxTextColour.Blue()));

    m_editRxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameRxBgColour.Red(), 
        g_Config.m_VscpRcvFrameRxBgColour.Green(),
        g_Config.m_VscpRcvFrameRxBgColour.Blue()));

    m_editTxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameTxTextColour.Red(), 
        g_Config.m_VscpRcvFrameTxTextColour.Green(),
        g_Config.m_VscpRcvFrameTxTextColour.Blue()));

    m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
        g_Config.m_VscpRcvFrameTxBgColour.Red(), 
        g_Config.m_VscpRcvFrameTxBgColour.Green(),
        g_Config.m_VscpRcvFrameTxBgColour.Blue()));

	m_maxRetries->SetValue( g_Config.m_VscpRegisterReadMaxRetries );
	m_readTimeout->SetValue( g_Config.m_VscpRegisterReadErrorTimeout );

	m_labelLogFile->SetLabel( g_Config.m_strPathLogFile );
	m_checkEnableLogging->SetValue( g_Config.m_bEnableLog );
	m_comboLogLevel->Select( g_Config.m_logLevel );

	m_checkConfirmDeletes->SetValue( g_Config.m_bConfirmDelete );

	m_comboNumericalBase->Select( g_Config.m_Numberbase );

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getDialogData
//

bool dlgConfiguration::getDialogData( bool bWriteToConfigFile )
{
    g_Config.m_bAutoscollRcv = m_ChkAutoScroll->GetValue();
    g_Config.m_VscpRcvFrameRxbPyamas = m_chkPyjamasLook->GetValue();	
    g_Config.m_UseSymbolicNames = m_chkUseSymbols->GetValue();
	g_Config.m_VscpRegisterReadMaxRetries = m_maxRetries->GetValue();
	g_Config.m_VscpRegisterReadErrorTimeout = m_readTimeout->GetValue();
	g_Config.m_logLevel = m_comboLogLevel->GetSelection();
	g_Config.m_bEnableLog = m_checkEnableLogging->GetValue();
	g_Config.m_bConfirmDelete = m_checkConfirmDeletes->GetValue();
	g_Config.m_Numberbase = m_comboNumericalBase->GetSelection();

    if ( bWriteToConfigFile ) {
        wxGetApp().writeConfiguration();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgConfiguration::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgConfiguration::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin dlgConfiguration bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end dlgConfiguration bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgConfiguration::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin dlgConfiguration icon retrieval
  wxUnusedVar(name);
  if (name == wxT("fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  else if (name == wxT("copy.xpm"))
  {
    wxIcon icon(copy_xpm);
    return icon;
  }
  return wxNullIcon;
    ////@end dlgConfiguration icon retrieval
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
//

void dlgConfiguration::OnButtonSetRxGridTextColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameRxTextColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameRxTextColour = dlg.GetColourData().GetColour();
        m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
//

void dlgConfiguration::SetRxGridBackgroundColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameRxBgColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameRxBgColour = dlg.GetColourData().GetColour();
        m_editRxForeGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip();  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
//

void dlgConfiguration::OnButtonSetTxGridTextColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameTxTextColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameTxTextColour = dlg.GetColourData().GetColour();
        m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
//

void dlgConfiguration::OnButtonSetTxGridBackgroundColourClick( wxCommandEvent& event )
{
    wxColourData cdata;
    cdata.SetColour( g_Config.m_VscpRcvFrameTxBgColour );
    wxColourDialog dlg ( this, &cdata );
    if ( wxID_OK == dlg.ShowModal() ) {
        g_Config.m_VscpRcvFrameTxBgColour = dlg.GetColourData().GetColour();
        m_editTxBackGroundColour->SetValue( wxString::Format(_("0x%02X%02X%02X"), 
            dlg.GetColourData().GetColour().Red(), 
            dlg.GetColourData().GetColour().Green(),
            dlg.GetColourData().GetColour().Blue()));	
    }

    event.Skip(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_INIT_DIALOG event handler for ID_PANEL4
//

void dlgConfiguration::OnInitDialog( wxInitDialogEvent& event )
{
    if ( g_Config.m_bAutoscollRcv ) {
        m_ChkAutoScroll->SetValue( 1 );
    }
    else {
        m_ChkAutoScroll->SetValue( 0 );
    }
    event.Skip();
}

