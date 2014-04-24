/////////////////////////////////////////////////////////////////////////////
// Name:        frmNode.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Wed 25 Apr 2007 11:47:53 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmNode.h"
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

#include "frmNode.h"
#include "threadvscpwork.h"
#include "pictures.h"

DEFINE_EVENT_TYPE(wxUPDATE_EVENT)
DEFINE_EVENT_TYPE(wxS1_EVENT)
DEFINE_EVENT_TYPE(wxS2_EVENT)
DEFINE_EVENT_TYPE(wxSB_EVENT)
DEFINE_EVENT_TYPE(wxBLOCK_EVENT)
DEFINE_EVENT_TYPE(wxDRIFT_EVENT)
DEFINE_EVENT_TYPE(wxPROGRESS_EVENT)
DEFINE_EVENT_TYPE(wxTEMPERATURE_EVENT)
DEFINE_EVENT_TYPE(wxVOLTAGE_EVENT)
DEFINE_EVENT_TYPE(wxMESSAGE_EVENT)
DEFINE_EVENT_TYPE(wxLOG_EVENT)
DEFINE_EVENT_TYPE(wxTERMINATE_EVENT)

////@begin XPM images
/* XPM */
static char *wxwin32x32_xpm[] = {
"32 32 6 1",
"   c None",
".  c #000000",
"X  c #000084",
"o  c #FFFFFF",
"O  c #FFFF00",
"+  c #FF0000",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"        ..............          ",
"        .XXXXXXXXXXXX.          ",
"        .XXXXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXXXXX.          ",
"        .XooXXXXXX..............",
"        .XooXXXXXX.OOOOOOOOOOOO.",
".........XooXXXXXX.OOOOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+++++++.XooXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++.XXXXXXXXX.OooOOOOOOOOO.",
".+oo++++...........OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OooOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".+oo+++++++++.    .OOOOOOOOOOOO.",
".++++++++++++.    ..............",
".++++++++++++.                  ",
"..............                  ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                "};

/* XPM */
static char *button_black_benji_park_01_xpm[] = {
/* columns rows colors chars-per-pixel */
"80 80 153 2",
"4  c #CACACA",
"M  c #D9D9D9",
"{  c #E8E8E8",
"5  c #F7F7F7",
"f  c #4B4B4B",
"F  c #5A5A5A",
".. c #696969",
"u. c #787878",
"*. c #878787",
">  c #969696",
"/  c #A5A5A5",
"<. c #B4B4B4",
":. c #C3C3C3",
"=. c #D2D2D2",
"&. c #E1E1E1",
".  c #FFFFFF",
"x  c #535353",
"<  c #626262",
"C  c #717171",
"v. c #8F8F8F",
"$  c #9E9E9E",
"#  c #ADADAD",
"R  c #E9E9E9",
"[  c #F8F8F8",
"c  c #4C4C4C",
"P  c #5B5B5B",
"|  c #6A6A6A",
"z  c #797979",
"+. c #888888",
"q. c #979797",
"a. c #A6A6A6",
"d. c #B5B5B5",
"k  c #C4C4C4",
"N. c #D3D3D3",
"x. c #F1F1F1",
"v  c #545454",
"1  c #636363",
"S  c #727272",
"@. c #909090",
"%  c #AEAEAE",
"@  c #BDBDBD",
"+  c #CCCCCC",
"-. c #DBDBDB",
"o. c #EAEAEA",
"d  c #4D4D4D",
"q  c #5C5C5C",
"Q  c #6B6B6B",
";. c #7A7A7A",
"G  c #898989",
"V. c #A7A7A7",
"2. c #B6B6B6",
"f. c #C5C5C5",
"C. c #E3E3E3",
"p  c #555555",
"9  c #646464",
"h. c #737373",
"3. c #828282",
"K  c #919191",
"n. c #AFAFAF",
"&  c #BEBEBE",
">. c #CDCDCD",
"O  c #DCDCDC",
"o  c #EBEBEB",
",. c #FAFAFA",
"V  c #4E4E4E",
"b  c #5D5D5D",
"'  c #6C6C6C",
"O. c #7B7B7B",
"l. c #8A8A8A",
"3  c #999999",
"9. c #A8A8A8",
":  c #C6C6C6",
"7. c #D5D5D5",
"   c None",
"J  c #E4E4E4",
"4. c #F3F3F3",
" . c #565656",
"e  c #656565",
"m. c #747474",
"j  c #838383",
"p. c #A1A1A1",
"t. c #B0B0B0",
"t  c #BFBFBF",
"*  c #DDDDDD",
"6  c #ECECEC",
"X  c #FBFBFB",
"s  c #4F4F4F",
"i  c #5E5E5E",
"Y  c #6D6D6D",
"8  c #8B8B8B",
"y. c #C7C7C7",
"}  c #D6D6D6",
"i. c #E5E5E5",
";  c #F4F4F4",
"T  c #575757",
"D  c #666666",
"U  c #757575",
"N  c #848484",
"]  c #939393",
"#. c #A2A2A2",
"s. c #B1B1B1",
"E  c #C0C0C0",
"H  c #DEDEDE",
"=  c #EDEDED",
"-  c #FCFCFC",
"L  c #505050",
"0  c #5F5F5F",
",  c #6E6E6E",
"_  c #8C8C8C",
"k. c #AAAAAA",
"8. c #B9B9B9",
"~  c #C8C8C8",
"M. c #D7D7D7",
")  c #F5F5F5",
"h  c #494949",
"W  c #585858",
"!  c #676767",
"A  c #767676",
"w. c #858585",
"r. c #949494",
"0. c #B2B2B2",
"5. c #C1C1C1",
"g. c #D0D0D0",
"j. c #DFDFDF",
"y  c #EEEEEE",
"^  c #FDFDFD",
"a  c #515151",
"w  c #606060",
"2  c #6F6F6F",
"n  c #7E7E7E",
"r  c #8D8D8D",
"A. c #9C9C9C",
"%. c #C9C9C9",
"c. c #D8D8D8",
"6. c #E7E7E7",
"g  c #4A4A4A",
"`  c #595959",
"X. c #686868",
"Z  c #777777",
"z. c #959595",
"b. c #A4A4A4",
"Z. c #C2C2C2",
"m  c #D1D1D1",
"1. c #E0E0E0",
"l  c #FEFEFE",
"B  c #525252",
"(  c #616161",
"I  c #707070",
"u  c #7F7F7F",
"e. c #8E8E8E",
"B. c #9D9D9D",
"$. c #ACACAC",
"7  c #BBBBBB",
/* pixels */
"                                                  . . . . . . . . . . . . . . . . . .   . .   . . . . .   . .                                                   ",
"                                                  .       . . . . X o O + @ # $ $ % &   * = -           .   .                                                   ",
"                                                  . . . . ; : > , < < < < < < < < < < < < < 1 2 3 4 5 . . .                                                     ",
"                                                . . 6 7 8 9 < < < < < < < < 0 q q w < < < < < < < < e r t y . . .                                               ",
"                                              - t u < < < < < i p a s d f g h h h h g f d s a p 0 < < < < 1 j k l . . .                                         ",
"                                        . . + z < < < < q x c h h h h h h g f c c f g h h h h h h d v b < < < < n m . . .                                       ",
"                                    . . M N < < < q B c g g g V p q e C Z Z Z Z Z Z Z A S D F x d g g g c B b < < < G H . . .                                   ",
"                                  . J K < < < i L g g g V P I U U U U U U U U U U U U U U U U U U Y T f g g g a 0 < < < > R . .                                 ",
"                              . . E e < < 0 B g g d W Q S S S S S S S S S S S S S S S S S S S S S S S S < B g g f x w < < ! ~ .                                 ",
"                            . ^ / ( ( w v f g V ! I I I I I I I I I I I I I I I I I I I I I I I I I I I I I Y F c g f p ( ( ( # l                               ",
"                          . ) _ ( ( b d g f ` , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ' x g g V b ( ( ] [                             ",
"                        . { z w w ` c f s D ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' 0 c f c P w w n 6 .                         ",
"                      . } | 0 0  .f f F ..............................................................................! B f f T 0 0 Y * .                       ",
"                  . . J ! 0 0 B f f b X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.X.q c f x 0 0 Q o..                     ",
"                  . ; U i i L f f P ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! b c f a i i O.5                     ",
"                  - +.i i B f f ` e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e e P c f x i i @.                    ",
"                . #.b b p c c T 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 F c c  .b b $.                  ",
"                t b b T c c L ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ` c c W b b %.                ",
"              &.w q F d c d b w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w v c V P q 1 {               ",
"            . *.q P s c c x 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 i V c s P q K .             ",
"            =.P P x d d V b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b ` d d v P P -..           ",
"          l ;.P ` V d d x b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b a d V F P j .           ",
"        . :.F F B d d d W q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q F d d x F F >.          ",
"        ,., F T V V V V P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P L V V W F U ^         ",
"      . <.` ` a V V V L F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F x V V B ` ` t         ",
"    . . , ` T V V V V a F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F T V V V T ` Z         ",
"    . 1.W W B V V V V a W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W s V V B W W o.      ",
"  . . $.W T s s s s s s v T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T x x x x x p T T T T T T T T s s s s W W 2..     ",
"  . . Z T p s s s s s s s L a B x p T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T  .x a L s s s s s s s L B  .T T T T p s s s s  .T 3..     ",
"  . o T T B s s s s s s s s s s s s s L B v  . . . . . . . . . . . . . . . . . . . . . . . . .p x a L s s s s s s s s s s s s s s B p  .p L s s s s x T ` 4.    ",
"  . 2.T T a L L L L L L L L L L L L L L L L L L a B v  . . . . . . . . . . . . . . . .v x a L L L L L L L L L L L L L L L L L L L L L L L L L L L L a T T 5..   ",
"  . 3. .p L L L L L L L L L L L L L L L L L L L L L L L a a B x x v v v x B B a a L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L p  ._ .   ",
". - W  .x L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L x  .w .   ",
". 6 p p B a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a B p p 5 . ",
". O p p B a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a B p p 6.. ",
". 4 v v B a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a B v v 7.. ",
". 8.v v B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B v v k . ",
". 9.v v B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B v v 0.. ",
". q.x x B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B x x #.. ",
". w.x x B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B B x x @.. ",
". j B x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x B B e.. ",
". r.B B x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x B B $ . ",
". / B B x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x B B t.. ",
". 2.a a v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v a a 5.. ",
". y.a a v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v x a a =.. ",
". M a a v p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p x a a J . ",
". o.L L x p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p x L L ) . ",
". X a L B p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p p B L W . . ",
". . u.L a  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .a L j .   ",
". . % s s p  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .p s s 8..   ",
"  . i.s s x T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T x s L y .   ",
"    . Q s a T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T L s A . .   ",
"    . p.V V T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T  .V V $.. .   ",
"      M V V v T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T T x V V J . .   ",
"      l 0 V L W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W s V ... .     ",
"      . a.d d p W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W W v d d s.. .     ",
"        5 b d s ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` W V d 9 X .       ",
"        . d.d d v ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` x d d E .         ",
"          - D c V ` F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F ` d c I l .         ",
"          . f.c c x F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F F B c c g..           ",
"          . . h.c d W P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P T c c n . .           ",
"            . } d c d F P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P ` d c s j..             ",
"              . k.f f s P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P V f f d..               ",
"              . . l.f f B q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q a f f z..                 ",
"                . [ 2 f f p q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q q v f f Z X                   ",
"                  . 6 q f f W b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b T f f < x.                    ",
"                    . c.s g f  .b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b p f g x j..                     ",
"                      . k L g g B i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i b a g g v >.. .                     ",
"                      . . M P g g s b i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i i b V g g ( 1.. .                       ",
"                        . . 6 , g g c P 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 P c g g U x..                           ",
"                            . [ *.g g g v 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 i x g g g v.X .                             ",
"                              . l b.g h h f  .w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w 0 p f h h f n.. .                               ",
"                              . . . =., h h h c W w w w w w w w w w w w w w w w w w w w w w w w w w w w w w w T c h h h m.M.. . .                               ",
"                                . . . l E w h h h d T 0 ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( 0  .d h h h e y.. . .                                   ",
"                                    . . . X n.T h h h g V p i ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( i p V h h h h F d.- . .                                       ",
"                                        . . . ; $ T h h h h h d x T ` b ( < < < < < < ( b ` T x c h h h h h ` b.5 . . .                                         ",
"                                              . . ^ N.3 ( h h h h h h h h g f d d f g h h h h h h h h e B.} l . . .                                             ",
"                                                    . . . 1.V., g h h h h h h h h h h h h h h g S k.C.. . . .                                                   ",
"                                                          . . . X { 7.Z.t.A.l.Z u._ $ s.k } o.X . . . . .                                                       ",
". .                                                                   . . . . . . . . . . . . . .                                                               "
};

/* XPM */
static char *button_blue_benji_park_01_xpm[] = {
/* columns rows colors chars-per-pixel */
"80 80 398 2",
"aX c #0B43C0",
"2O c #0E53D7",
"6  c #0E56DA",
"O  c #0E56DB",
"+  c #0E56DC",
"6O c #071076",
"7O c #071077",
"9  c #071378",
"oO c #071379",
"]  c #07167E",
"!  c #0D1B80",
"i. c #071982",
"'o c #071983",
"f  c #313C90",
"4. c #313F96",
"u. c #3A489B",
"`. c #3D4FA4",
" . c #5B65AA",
"=  c #0A2FA2",
"8o c #2240A6",
"so c #0A2FA3",
"vo c #0A32A5",
"a  c #0A32A6",
":o c #2E4AAB",
"I  c #0A32A7",
"{X c #3A54AE",
"xo c #0A32A8",
"A. c #0A35AA",
"+X c #0A35AB",
"po c #0A35AC",
"R. c #6D7AB9",
"*X c #0A38AF",
"o. c #0A38B0",
"R  c #888FC0",
"7X c #0D45C3",
"$  c #0D4BCA",
"p  c #0D4BCB",
"{o c #0D4ECE",
"[o c #0D4ECF",
"V  c #0D4ED0",
"P  c #0D51D4",
",. c #0D54D7",
"<. c #0D51D6",
"=O c #0D54D8",
")  c #0D54D9",
";O c #061176",
"t  c #121E7E",
"q  c #182381",
";  c #09218E",
"Qo c #092491",
",O c #092492",
"xX c #092796",
"Zo c #092797",
"XX c #092A9A",
"Go c #092A9B",
"uX c #243C9E",
"$o c #092A9C",
"v  c #6068A9",
"mo c #092D9F",
"#o c #092DA0",
"pX c #334AA5",
",o c #1B39A3",
"FX c #2D48A6",
"QX c #2D48A8",
"/X c #334DAA",
"K. c #5765AE",
"CX c #3F57AD",
"v. c #636FB1",
"Oo c #3651AC",
"^X c #3952AD",
"ko c #0933A9",
"$X c #5D6DB4",
"}  c #9096C5",
"Q. c #0C43BF",
"   c None",
"sX c #0C43C0",
"~. c #0C46C3",
"1X c #0C46C4",
"~o c #0C49C7",
"oX c #0C49C8",
"S. c #0C4CCE",
"X  c #0F56DB",
")o c #081981",
"#O c #081982",
"K  c #1D2986",
"(o c #081984",
"h. c #0E2187",
"M. c #081C86",
"Y. c #081F8A",
"/. c #081F8B",
"Io c #08228E",
"$. c #08228F",
"5X c #0B2691",
"<X c #082593",
"eX c #082594",
"T. c #293C99",
".o c #11319D",
":X c #3248A2",
"%o c #11319F",
"RX c #354FAB",
"7. c #6872B2",
"co c #0B32A7",
"zo c #0B32A8",
"7  c #0B35AB",
"{  c #7D85BB",
"Po c #0B38AF",
"ao c #0B38B0",
"c. c #7A84BD",
"4  c #0B3BB3",
"]o c #0B3BB4",
"z  c #9BA1C9",
"@o c #0B3EB8",
"MX c #0B41BC",
"mX c #0B41BD",
"I. c #0E4ECE",
"@  c #0E51D4",
"o  c #0E57DA",
"-O c #071175",
"%O c #071176",
">  c #07147A",
"<  c #07147B",
".O c #07147C",
"+O c #07177E",
"%. c #07177F",
"U  c #071780",
"&. c #102084",
"'  c #071A83",
"w. c #071A84",
".. c #1C2A89",
"3. c #071D88",
"S  c #2E3990",
"To c #07208D",
".X c #192F94",
"Co c #0A2797",
"1o c #0D2E9D",
"cX c #1F3A9F",
"T  c #5862A7",
"C  c #0A2D9F",
"H. c #4C5BA9",
"3  c #0A30A3",
"go c #0A30A4",
"BX c #344DA9",
"fo c #0A30A5",
"g. c #5E6AAF",
"2X c #495DAD",
"nX c #0A33A7",
"N  c #0A33A8",
"lo c #0A33A9",
"). c #5E6DB3",
"j. c #0A36AC",
"`  c #0A36AD",
"V. c #7681BC",
"t. c #828BC0",
"E  c #949AC6",
"u  c #0D4CCC",
"`o c #0D4CCD",
"&  c #0D4FD0",
"k. c #0D4FD1",
"_  c #0D52D5",
"1. c #0D52D6",
"O. c #0D52D7",
":O c #0D55D9",
"0  c #0F1B7C",
"m  c #09167B",
"Z. c #0C2088",
"(. c #0C238D",
"1  c #09228E",
"!o c #09228F",
"yX c #092593",
"Do c #092594",
"{. c #092897",
"-  c #092898",
"JX c #092B9B",
"IX c #12319D",
"4o c #092B9D",
"bo c #092EA0",
"2o c #092EA1",
"SX c #2A45A5",
"YX c #2440A4",
"ZX c #304AA7",
"B. c #5462AB",
"]X c #2D49A9",
"qo c #2A48AA",
";o c #334EAD",
"3X c #5769B3",
"5. c #7B84BC",
"f. c #7E88BF",
"uo c #0C3EB6",
"x  c #9398C5",
"3O c #0C3EB7",
"1O c #0C3EB8",
"l  c #9CA1CA",
"U. c #0C41BC",
"_o c #0C41BD",
"tX c #0C44C1",
"0. c #0C47C5",
"=X c #0C47C6",
"'. c #0C4AC8",
"]. c #0C4AC9",
"/o c #0C4ACA",
">. c #0C4DCE",
"<O c #081175",
"0O c #081176",
"XO c #081479",
" O c #08147B",
"d  c #0B187C",
"w  c #202B85",
"^o c #081A84",
"e  c #263088",
"s. c #0B1E86",
"s  c #081D87",
"G. c #081D88",
"Ro c #081D89",
"W. c #0B218A",
"8  c #08208C",
"Eo c #08208D",
"^. c #082390",
"Jo c #082391",
"Fo c #082392",
"0X c #112C96",
"NX c #0B2A98",
"}. c #384BA2",
"5o c #0B2D9E",
"!X c #203DA2",
"to c #1132A0",
"ro c #1737A2",
"DX c #2C47A6",
"|X c #2946A7",
"eo c #1D3CA5",
"GX c #3851AA",
"C. c #5C69AF",
"}X c #3550AC",
"=o c #2F4BAB",
"4X c #5366B1",
"-o c #3550AD",
"qX c #5366B3",
"_. c #6876B8",
"no c #0B39B1",
"Ko c #0B39B2",
"|  c #8990C1",
"(X c #0B3CB4",
"q. c #0B3CB5",
"G  c #989EC8",
"3o c #0B3CB7",
"p. c #0B3FB9",
")X c #0B3FBA",
"Wo c #0B3FBB",
"&O c #0B42BD",
"%  c #0E52D5",
"/  c #0E52D6",
"i  c #0E55DB",
"qO c #071275",
",  c #071277",
"|o c #07157B",
"Z  c #07157C",
"L  c #07157D",
"OO c #071880",
"~  c #071881",
"X. c #071B85",
"b. c #0D2087",
"#X c #132B92",
"b  c #434D9A",
"Q  c #3D4899",
"HX c #0D2C99",
"wX c #1F389C",
"Xo c #0A2B9A",
",X c #2B429F",
"gX c #223C9F",
"`X c #16359F",
"yo c #0D2F9E",
"l. c #0A2B9D",
"<o c #10309F",
"Ao c #0A2EA0",
"6o c #1334A1",
"'X c #223FA4",
"&o c #1C3AA3",
"D  c #6169AB",
"n. c #0A2EA1",
"r. c #5B67AC",
"E. c #5261AC",
"*o c #2543A7",
"zX c #0A31A5",
"m. c #0A31A6",
"wo c #2B48AA",
"jX c #4359AE",
"WX c #3A53AD",
"d. c #6470B2",
"[X c #3752AD",
"iX c #4F63B1",
"vX c #465DB0",
"6X c #0A34A9",
"W  c #737BB6",
"^  c #0A34AA",
" X c #6473B6",
"|. c #6473B7",
"ho c #0A37AD",
"do c #0A37AE",
"LX c #0A3AB2",
"=. c #8B92C3",
"#  c #0D4ACA",
"B  c #0D4DCD",
"D. c #0D4DCE",
"5  c #0D50D2",
"a. c #0D50D3",
"*O c #0D53D6",
"@. c #0D53D7",
".  c #0D56DB",
"@O c #061378",
"N. c #091D86",
"8. c #182889",
"L. c #0C218A",
"Y  c #27338D",
"5O c #09208C",
"9O c #092390",
"8O c #092391",
"}o c #092694",
"fX c #092695",
";. c #424E9D",
"kX c #092696",
"2  c #092998",
"PX c #092999",
"Vo c #09299A",
"_X c #0C2D9B",
"J  c #545DA4",
"Mo c #092C9D",
"No c #09299D",
"io c #092C9E",
"UX c #1B38A0",
" o c #1E3CA2",
"*. c #5A65AA",
"x. c #5D69AF",
"9o c #2745A9",
"VX c #425AAF",
"hX c #4B60B1",
"%X c #6070B5",
"jo c #0935AB",
"j  c #7B82B8",
"H  c #8A90C0",
"-. c #878FC1",
"F  c #9095C3",
"6. c #878FC2",
"lX c #0C3FB9",
"rX c #0C42BE",
"Lo c #0C42BF",
";X c #0C45C1",
"2. c #0C45C2",
"8X c #0C45C3",
"-X c #0C48C6",
"[. c #0C48C7",
"OX c #0C4BC9",
"!. c #0C4BCB",
"#. c #0C4BCC",
">O c #0C4ECF",
"+. c #0C54D7",
"(  c #0C54D9",
"$O c #081276",
"A  c #08157C",
"r  c #1D2884",
":  c #081B84",
"e. c #0E2085",
"z. c #081B85",
"n  c #26318A",
"M  c #081E89",
"9. c #081E8A",
"@X c #08218D",
"[  c #2C3991",
"Ho c #082491",
":. c #082492",
"Yo c #082493",
"bX c #0E2C98",
"g  c #4D569F",
"~X c #11309C",
"Bo c #082A9C",
"AX c #2944A4",
"oo c #203EA3",
"h  c #656DAC",
"7o c #1A39A3",
"TX c #2C48A7",
">o c #2643A8",
"&X c #4D5FAD",
"+o c #324DAB",
"0o c #2947AA",
"EX c #3E57AF",
"4O c #0B34AA",
">X c #5C6DB5",
"c  c #7D84B9",
"y  c #0B37AD",
"Uo c #0B37AE",
"J. c #717DBA",
"y. c #7D87BD",
"*  c #0B3AB1",
"F. c #0B3AB2",
"k  c #9297C4",
"P. c #0B3DB6",
"dX c #0B3DB7",
"So c #0B40BA",
"KX c #0B40BB",
"9X c #0B40BC",
/* pixels */
"                                                                                                                                                                ",
"                                                                            . X X o                                                                             ",
"                                                              . O O O O O O O O O O O O O O O O +                                                               ",
"                                                        . O O O O O O O O O @ # $ % O O O O O O O O O O                                                         ",
"                                                  O O O O O O & * = - ; : > , , , , < : 1 2 3 4 5 O O O O O 6                                                   ",
"                                              O O O O O # 7 8 9 9 9 9 9 9 0 q w e r t 9 9 9 9 9 9 1 y u O O O O X                                               ",
"                                          i O O O p a s > > d f g h j k l l l l l l l z x c v b n m > > M N B O O O X                                           ",
"                                      i O O O V C Z < A S D F G G G G G G G G G G G G G G G G G G H J K < < L = 5 O O O O                                       ",
"                                    O O O P I U L Y T R E E E E E E E E E E E E E E E E E E E E E E E E W Q ! L ~ ^ / O O O                                     ",
"                                ( ) ) _ ` ' ] [ { } } } } } } } } } } } } } } } } } } } } } } } } } } } } } |  ...] X.o.O.) ) )                                 ",
"                              +.@.@.#.$.%.&.*.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.-.;.%.%.:.>.@.@.,.                              ",
"                            <.1.1.2.3.~ 4.5.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.6.7.8.~ 9.0.1.1.O.                            ",
"                          P P P q.w.e.r.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.t.y.u.i.X.p.P P P                           ",
"                        a.a.5 N w.s.d.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.g.h.w.j.a.a.a.                        ",
"                      k.k.V l.z.s.x.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.v.b.z.n.k.k.k.                      ",
"                    V V V m.M.N.B.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.V.C.Z.M.A.V V &                     ",
"                  S.D.D.F.G.G.H.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.K.L.G.P.D.D.I.                  ",
"                  u u U.Y.M T.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.E.W.8 Q.u u                   ",
"                !.!.~.^./.(.)._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._._.`./.:.0.!.!.                ",
"              '.].[.{.8 8 }.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|. X.X8 XXoX].OX              ",
"              oXoX+X@X@X#X$X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X&X@X@X*XoXoX              ",
"            =X-X;X:.$.$.:X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X>X,X$.<X~.-X=X            ",
"            1X1Xa ^.^.^.2X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X4X5X^.6X1X1X            ",
"          7X8X9X<X:.:.0XqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXwX:.eXrX8X8X          ",
"          tXtX3 yXyXyXuXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXiXpXyXyXa tXtX          ",
"        aXsXdXfXfXfXfXgXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXjXfXfXkXlXsXsX        ",
"        rXrXzXxXxXxXxXcXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXvXbXxXxXnXrXrX        ",
"        mXMXXX{.{.{.{.NXBXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXVXCXZXAXSXDXFXGXVXVXVXVXVXVXVXVXHX{.{.JXmXmX        ",
"      KXKXLXPXPXPXPXPXPXPXIXUXYXTXRXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXEXWXQX!XIXPXPXPXPXPXPXPX~XYX^XEXEXEXEX/XPXPXPXPX(XKXKX      ",
"      )X)XN XXXXXXXXXXXXXXXXXXXXXXXX_X`X'X]X[X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X{X}X|X o.oXXXXXXXXXXXXXXXXXXXXXXXXXXXoooOo{X+o.oXXXXXXXX^ )X)X      ",
"      @o@o#o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o%o&o*o=o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o;o:o>o,o<o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o1o$o$o$o$o$o$o2o@o@o      ",
"    3oP.* 4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o5o6o7o8o9o9o0oqowo9o8oeorotoyo4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4o4 P.uo    ",
"    q.q.^ ioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioioiopoq.q.    ",
"    4 4 I #o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#o#oN 4 4     ",
"    F.F.m.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.I F.F.    ",
"    aoaozXsosososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososom.aoao    ",
"    dodofogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogozXdodo    ",
"  ho` ` zXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzXzX` `     ",
"  +X+XA.I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I +X+Xjo  ",
"  ^ ^ 6XN N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N ^ ^ ko  ",
"  6XN lo^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ N N zo  ",
"  a I xo+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+XI I co  ",
"  vozXzXj.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.j.zXzX    ",
"    3 3 hododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododo` 3 3     ",
"    = = ho*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*Xj.= =     ",
"    bobo` nononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononono+Xbobo    ",
"    momo+XF.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.^ momo    ",
"    MoMoN 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 nXMoMo    ",
"    No$o2oq.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.#o$oBo    ",
"      VoVo4 P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.* VoVo      ",
"      - - +X@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o6X- -       ",
"      CoZoAop.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.p.ioZoZo      ",
"      fXfXxX@oSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoSoP.fXfX        ",
"        DoDodo9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9X9XpoDoDo        ",
"        Fo:.MomXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXmXGo:.:.        ",
"        HoJoJoKoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoPoJoJo          ",
"          $.$.- sXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXLoxX$.Io          ",
"            @X@XUotXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtX+X@X@X            ",
"            8 8 YotX8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8X8XsXJo8 To            ",
"              Y.Y.^ 1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1X1Xa Y.Y.              ",
"              RoM EomX=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=XWo/.M M               ",
"                s s Qo~.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.2.!os s                 ",
"                  z.z.PX~ooXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoX[.xXz.z.                  ",
"                  ^o^o^ozX/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/o/on.^o^o(o                  ",
"                    )oi.i.LX!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.doi.i.i.                    ",
"                      ~ ~ w._o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o)X'o~ ~                       ",
"                        U %.i.]oD.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.no~ %.%.                        ",
"                          ] ] ] I [o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o[o{oso] ] ]                           ",
"                            Z Z Z - D.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.u }oZ Z |o                            ",
"                               O< < /.oX5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 0.Ro< < .O                              ",
"                                XOoOoOOOho5 P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P P k.A.+OoOoO@O                                ",
"                                    , , , : q._ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ P F.#O, , ,                                     ",
"                                      $O%O%O%O9.&O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*OSos %O%O%O%O                                      ",
"                                          %O%O%O%OQo@oP =O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=O=Oa.P.Io%O%O%O-O                                          ",
"                                              ;O%O%O%OoODo4 k.:O:O:O:O:O:O:O:O:O:O:O:O:O:O:O:O:O:O>O* ,O9 %O%O%O%O                                              ",
"                                                <O;O%O%O%O%O, 1 7 1O2.`o2OO O O O O O 2Ou 2.3O4O5O%O%O%O%O%O6O                                                  ",
"                                                      7O%O%O%O%O%O%O%O%O%OZ M.8O9OM.Z %O%O%O%O%O%O%O%O%O-O                                                      ",
"                                                              $O%O%O%O%O%O%O%O%O%O%O%O%O%O%O%O%O;O                                                              ",
"                                                                          %O0O%OqO;O0O                                                                          ",
"                                                                                                                                                                "
};

/* XPM */
static char *button_green_benji_park_01_xpm[] = {
/* columns rows colors chars-per-pixel */
"80 80 393 2",
"A  c #5A902E",
"+  c #68DB0D",
" O c #65D40D",
"u  c #68DB0F",
"B. c #62D00D",
"(o c #62CD0D",
"OX c #5FC90C",
"-X c #5FC60C",
"L. c #7EAE57",
"!. c #5CBF0C",
"_X c #59BB0B",
"`  c #53AD0A",
"S. c #81AF5C",
"t  c #53AD0B",
"_. c #84B35E",
"b. c #81AF5D",
"p  c #50A60A",
"Fo c #4A9B09",
"tX c #479408",
"Ao c #479409",
"Vo c #47970A",
"#X c #448D08",
"v  c #81A960",
"^o c #3E8207",
"V. c #418608",
"7X c #47910B",
"XO c #3E8208",
"1  c #3B7B07",
"Q. c #448A0B",
"[o c #3B7B08",
"oo c #509D11",
"P. c #448A0C",
"UX c #509D12",
"wX c #4D9611",
"M. c #44870D",
"d  c #3E7C0B",
"!  c #41800D",
"%. c #448410",
"-o c #59A31C",
"XX c #509419",
"[X c #5CA422",
"TX c #5CA424",
"<X c #5F9F2B",
"#o c #68AB32",
"G  c #A2C08A",
"@o c #6BAC36",
"1. c #5F9631",
"}  c #A8C590",
"-. c #6B9D42",
"+O c #67D60D",
"/  c #67D60E",
"4  c #64D20D",
"~. c #61CB0C",
"5X c #80B357",
"i  c #61CB0D",
"3X c #5EC40C",
"dX c #5BC00B",
"Do c #5BBD0B",
"/o c #5BBD0C",
"I. c #58B60B",
"fo c #55B20B",
"@X c #52AB0A",
"6  c #52AB0B",
"ko c #4FA40A",
"So c #4C9D09",
"&X c #86B560",
"IX c #499909",
";. c #469208",
"-O c #469209",
".X c #89B664",
"(. c #438B08",
"0. c #408407",
"U  c #3D8007",
"|o c #3A7907",
"#O c #3A7607",
"4O c #3A7608",
"g. c #9BBF7E",
"SX c #61A429",
"e. c #9EC082",
"!X c #64A82D",
"R. c #5B9929",
"   c None",
"=. c #A1C187",
",o c #6AAD33",
"D  c #A7C390",
"WX c #70AF3E",
"z  c #B0C99B",
".  c #69DB0D",
"@  c #69DB0E",
"8. c #66D70D",
"X  c #69DB0F",
"P  c #66D40D",
"$  c #66D40E",
"x. c #63D00D",
"B  c #63CD0D",
"[. c #60C90C",
",. c #5DC20C",
"LX c #5ABB0B",
"6o c #57B70B",
"T  c #7CA758",
")X c #57B40B",
"ho c #54B00B",
"co c #51A909",
"8X c #51A90A",
"bo c #51A60A",
"h. c #82AF5E",
"-  c #4EA20A",
"KX c #4B9B09",
"}. c #489709",
"/. c #459008",
"2O c #459009",
"BX c #4B980B",
"M  c #428908",
"y. c #3F8207",
"_o c #3C7E07",
"C. c #428609",
"po c #51A011",
"d. c #42860B",
"<  c #397707",
"j. c #45870E",
"5. c #4B8918",
"rX c #579C1F",
"1o c #60A826",
"r. c #9ABD7D",
"FX c #63A62C",
"AX c #66A730",
"[  c #5A912C",
"EX c #69AB35",
"x  c #A9C593",
"t. c #669B3A",
"*X c #78AD4D",
"J. c #75A94C",
"aX c #7BB14F",
"g  c #729F4D",
")  c #68D90D",
"#  c #68DC0E",
"L  c #65D20D",
"]o c #62CE0D",
"Z. c #7BAB54",
"{. c #5FC70C",
"H  c #78A454",
"fX c #5CC00C",
"i. c #59B90B",
"xX c #59B90C",
"PX c #56B20A",
"G. c #56B20B",
"xo c #53AB09",
"Mo c #53AE0A",
"%X c #84B45D",
"I  c #50A70A",
"&o c #4DA009",
"Co c #4DA00A",
"Eo c #4A9909",
"hX c #479509",
"%O c #448E08",
":  c #448E09",
"a  c #418708",
"f. c #87B264",
"C  c #3B7C07",
"D. c #44880C",
"]X c #539F16",
"YX c #56A01B",
"{  c #99BB7D",
".o c #62A729",
"n  c #538A26",
"|  c #A2C189",
">o c #6BAD35",
">X c #65A232",
"HX c #6BAA38",
"f  c #5C9031",
"|X c #6EAE3A",
"'. c #6BA43D",
"F  c #AEC898",
"+. c #67D70C",
"O. c #67D70D",
">O c #67D70E",
"o  c #6ADB0F",
"l. c #64D30D",
"*  c #64D00D",
"7. c #5EC50C",
"yX c #5BBE0C",
"gX c #58B70B",
",O c #58B70C",
"o. c #55B00A",
"k. c #52AC0A",
"cX c #4FA50A",
"do c #4C9E09",
"NX c #499709",
"qX c #469308",
"'X c #4C9B0C",
"7  c #438C08",
" X c #89B764",
"Yo c #438908",
"X. c #408507",
"v. c #408508",
"]  c #3D7E07",
"mX c #4C980E",
"). c #468D0C",
",  c #3A7A07",
"`. c #8CB868",
"^X c #4F9C11",
"m  c #3D7B09",
"E. c #8FB96D",
"$X c #4C9213",
"9  c #407C0F",
"K. c #92BA71",
"+o c #5BA320",
"bX c #589F1F",
"2. c #98BC7B",
"J  c #4C861D",
"c  c #98B97D",
"DX c #61A52A",
"{X c #64A92D",
"GX c #64A62D",
"w  c #528826",
"3. c #A1C287",
"*. c #A4C38B",
"ZX c #70AD3F",
"lX c #73AE43",
"(  c #69D90C",
"_  c #66D50D",
"&  c #66D50E",
":. c #63CE0C",
"F. c #63CE0D",
"Wo c #60C70C",
"U. c #63CE0E",
"^. c #5DC30C",
"9X c #5DC30D",
"0X c #5ABC0B",
"Y. c #5ABC0C",
"9. c #57B50B",
"jo c #54AE0A",
"Po c #54AE0B",
"$o c #51AA0A",
"Bo c #4B9C08",
"no c #51A70B",
"2  c #4EA30A",
"*o c #4B9C09",
"Ho c #459108",
"Zo c #489509",
"Lo c #458E08",
"4o c #4E9D0D",
"6. c #428A08",
")o c #3F8307",
"'o c #3C7C07",
"Z  c #3C7C08",
"&O c #397507",
";O c #397508",
"q. c #45850E",
"qo c #57A31A",
"e  c #4B841D",
"ro c #63AA29",
"to c #63AA2A",
"q  c #4E8520",
"RX c #63A72C",
"<o c #66AB2E",
"sX c #66A533",
"VX c #69A934",
"}X c #6CAD37",
"/X c #6CAD39",
"Q  c #66993D",
"nX c #75B046",
"kX c #78B14B",
"eX c #7EB353",
"O  c #68DA0E",
"W. c #7BAC52",
"p. c #65D30D",
"@. c #62CC0C",
"y  c #62CC0D",
"]. c #5FC80C",
"{o c #5FC50C",
"Io c #5CC10C",
"`X c #59BA0B",
",X c #84B55C",
"3  c #56B30B",
"so c #56B60C",
"&. c #7EAA5A",
"mo c #53AC0A",
"a. c #50A80A",
"lo c #50A50A",
"5o c #4DA109",
"N. c #4DA10A",
"oX c #4A9A09",
"9o c #4D9E0B",
"1X c #479308",
"zX c #479609",
"iX c #479309",
"#. c #448F08",
"<. c #418807",
"1O c #448C09",
"u. c #418507",
"H. c #418808",
"!o c #3E8408",
"~  c #3E8107",
"3o c #509F10",
"s  c #3B7A07",
"5O c #387607",
"6O c #387608",
"uo c #59A51D",
"Xo c #59A21E",
"0  c #478118",
"jX c #599F22",
".. c #4D891C",
"(X c #68AA33",
"k  c #A8C492",
"l  c #B1CA9C",
"$O c #67D80D",
"6X c #7DB153",
"z. c #64D10D",
"Qo c #61CA0C",
"%  c #61CA0D",
"2X c #5EC30C",
"Jo c #5BBF0C",
"%o c #58B80B",
":O c #58B80C",
"=  c #55B10B",
"zo c #52AD0A",
"w. c #80AC5B",
"^  c #52AA0A",
"<O c #52AA0B",
"go c #4FA60A",
"No c #4C9F09",
"uX c #4FA30A",
"V  c #4C9F0A",
";  c #499809",
"Uo c #438D07",
"Go c #469108",
"To c #469109",
"T. c #438A08",
"m. c #86B163",
"JX c #4C990D",
"'  c #408307",
"$. c #3D7F07",
".O c #3A7806",
"8  c #3A7807",
"3O c #3A7507",
"0o c #52A113",
"4. c #89B268",
"io c #55A217",
"2o c #58A31B",
"r  c #437E12",
"A. c #95BC76",
"n. c #98BD7A",
"eo c #61A927",
"pX c #5B9E24",
"yo c #64AA2B",
":o c #67AB2F",
"Y  c #558D27",
"R  c #A1C088",
" o c #6AAC35",
"QX c #6DAD3A",
"|. c #67A238",
"E  c #AAC694",
"CX c #73AF42",
"b  c #6A9A43",
"4X c #76AD49",
"5  c #69DA0E",
">. c #66D60D",
"=O c #63CF0C",
"`o c #63CF0D",
"+X c #60C80C",
"MX c #5ABD0B",
";X c #5DC10C",
"@O c #5ABA0B",
"7o c #57B60B",
"=X c #54AF0A",
"Ko c #54AF0B",
"N  c #51A80A",
" . c #7FAA5B",
"vo c #51A80B",
"8o c #4B9D09",
"c. c #4B9D0A",
"Oo c #4B9A0A",
"vX c #489609",
":X c #458F08",
"Ro c #458F09",
"ao c #4E9E0D",
"S  c #82AB61",
"s. c #3F8407",
">  c #3F8408",
"~o c #3F8108",
"K  c #3C7D07",
"*O c #397606",
"=o c #519F11",
"h  c #85AC65",
"OO c #397607",
"}o c #397908",
"oO c #397608",
"W  c #91B673",
"~X c #5AA220",
"wo c #5DA622",
";o c #60A725",
"j  c #97B87B",
/* pixels */
"                                                                                                                                                                ",
"                                                                            . X o O                                                                             ",
"                                                              + @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ #                                                               ",
"                                                        . @ @ @ @ @ @ @ @ @ $ % % & @ @ @ @ @ @ @ @ @ @                                                         ",
"                                                  @ @ @ @ @ @ * = - ; : > , < < < < 1 > : ; 2 3 4 @ @ @ @ @ 5                                                   ",
"                                              @ @ @ @ @ % 6 7 8 8 8 8 8 8 9 0 q w e r 8 8 8 8 8 8 : t y @ @ @ @ u                                               ",
"                                          @ @ @ @ i p a s s d f g h j k l l l l l l l z x c v b n m s s M N B @ @ @ X                                           ",
"                                      @ @ @ @ * V C 1 Z A S D F F F F F F F F F F F F F F F F F F G H J 1 1 K - L @ @ @ @                                       ",
"                                    @ @ @ P I U K Y T R E E E E E E E E E E E E E E E E E E E E E E E E W Q ! K ~ ^ / @ @ @                                     ",
"                                ( ) ) _ ` ' ] [ { } } } } } } } } } } } } } } } } } } } } } } } } } } } } } |  ...] X.o.O.) ) )                                 ",
"                              +.O.O.@.#.$.%.&.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.=.-.$.$.;.:.O.O.O.                              ",
"                            >.>.>.,.<.~ 1.2.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.4.5.~ 6.7.>.>.8.                            ",
"                          P P P 9.0.q.w.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.r.t.y.u.i.P P P                           ",
"                        p.p.L a.s.d.f.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.h.j.s.k.p.p.l.                        ",
"                      z.z.x.c.v.d.b.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.n.m.M.v.N.z.z.z.                      ",
"                    B.x.x.p V.C.Z.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.S.D.V.^ x.x.x.                    ",
"                  :.F.F.G.H.H.J.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.L.P.H.I.F.F.U.                  ",
"                  y y Y.T.M R.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.W.Q.7 !.y y                   ",
"                ~.~.^./.(.)._.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.`.'.(.;.7.~.~.                ",
"              ].[.{.}.7 7 |. X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X.XXX7 oX].[.OX              ",
"              +X+X@X#X#X$X%X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X*X#X#X=X+X+X              ",
"            -X-X;X;.:X:X>X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X<X:X1X2X-X-X            ",
"            3X3Xp /././.4X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X5X6X7X/.8X3X3X            ",
"          9X^.0XqX;.;.wXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXeXrX;.tXyX^.^.          ",
"          ;X;XuXiXiXiXpXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXaXsXiXiXp ;X;X          ",
"        dXfXgXhXhXhXhXjXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXkXlXhXhXzXxXfXfX        ",
"        yXyXcXvXvXvXvXbXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXnXmXvXvXI yXyX        ",
"        MX0XoXNXNXNXNXBXVXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXCXZXAXSXDXFXGXHXCXCXCXCXCXCXCXCXJXNXNXKXMXMX        ",
"      LXLXPXIXIXIXIXIXIXIXUXYXTXRXEXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXWXQX!X~XUXIXIXIXIXIXIXIX^XTX/XWXWXWXWX(XIXIXIXIX)XLX_X      ",
"      `X`XN oXoXoXoXoXoXoXoXoXoXoXoX'X]X[X{X}X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X o.oXooooXoXoXoXoXoXoXoXoXoXoXoXoXOo+o@o|X#ooooXoXoXoX$o`X`X      ",
"      %o%o&o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o=o-o;o:o>o>o>o>o>o>o>o>o>o>o>o>o>o>o>o,o<o1o2o3o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o*o4o*o*o*o*o*o*o5o%o%o      ",
"    6o7o= 8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o9o0oqowoeoeorotoyoeowouoiopoao8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o8o3 7oso    ",
"    9.9.^ dodododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododododok.9.9.    ",
"    3 3 I &o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&o&oN 3 3     ",
"    fofogoN.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.I fofo    ",
"    hohocX2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 gohoho    ",
"    jojocXkokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokolojojo    ",
"  zo` ` cXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcXcX` `     ",
"  @X@X^ I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I @X@Xxo  ",
"  $o$o8XN N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N $o$oco  ",
"  8XN 8X$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$oN N vo  ",
"  boI a.@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@XI I no  ",
"  cXcXcXmomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomomocXcX    ",
"    2 2 ` MoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMoMozo2 2     ",
"    - - ` =X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=Xmo- -     ",
"    &o&o` = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = @X&o&o    ",
"    NoNo@Xfofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofo$oNoNo    ",
"    8o8oa.3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 I 8o8o    ",
"    8o*o5o9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.9.&o*oBo    ",
"      oXoX3 7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o7o= oXoX      ",
"      ; ; @X%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o8X; ;       ",
"      Vo}.Coi.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.i.do}.}.      ",
"      ZoZovX%o`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X`X7oZoZo        ",
"        AoAoMo0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0X0Xk.AoAo        ",
"        ;.;.SoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoDoFo;.;.        ",
"        GoHoHofoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoJoKoHoHo          ",
"          :X:X; fXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfX!.vX:XLo          ",
"            #X#XPo;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X@X#X#X            ",
"            7 7 qXIo^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.fXHo7 Uo            ",
"              6.6.$o3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3X3Xp 6.6.              ",
"              YoM #XDo-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-XLX(.M Yo              ",
"                a a To^.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.{.,.Roa a                 ",
"                  v.v.EoWo+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X+X{.vXv.v.                  ",
"                  > > > cXQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoQoN.> > !o                  ",
"                    ~oy.y.PX~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.joy.y.^o                    ",
"                      ~ ~ s./o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o`X)o~ ~                       ",
"                        U $.y.)XF.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.= ~ $.$.                        ",
"                          _o_o_oI `o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`oF.2 _o_o_o                          ",
"                            'o'o'o; ]oz.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.z.y Ao'o'o1                             ",
"                              [o1 1 (.+XL L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L {oM 1 1 C                               ",
"                                }o|o|oU ` 4  O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O Oz.$o] |o|o.O                                ",
"                                    < < < > 9._ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ P foXO< < <                                     ",
"                                      oOOOOOOO6.Do+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O+O@Oa OOOOOO#O                                      ",
"                                          OOOOOOOOTo%o O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$O$Ol.7o%OOOOOOO&O                                          ",
"                                              *OOOOOOO|oAo3 z.) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) =O= -O8 OOOOOOOO                                              ",
"                                                ;O*OOOOOOOOO< : 6 :O,.(o>O@ @ @ @ @ @ >Oy ,.,O<O1OOOOOOOOOOOOO                                                  ",
"                                                      < OOOOOOOOOOOOOOOOOO'oV.To2OV.'oOOOOOOOOOOOOOOOOOO3O                                                      ",
"                                                              oOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO*O                                                              ",
"                                                                          OO4O5O&O*O6O                                                                          ",
"                                                                                                                                                                "
};

/* XPM */
static char *button_red_benji_park_01_xpm[] = {
/* columns rows colors chars-per-pixel */
"80 80 391 2",
"4O c #751207",
"|o c #781006",
"Q  c #811307",
"`  c #7E1207",
"B  c #7B1107",
"f  c #B8817B",
"Wo c #811308",
"Fo c #8D1407",
"Q. c #901508",
".X c #8D1408",
";X c #931608",
"2O c #901509",
"zo c #9F1709",
"v. c #87180D",
"jo c #A5190A",
"=  c #A2180A",
"M  c #9F170A",
"ko c #AE190A",
"WX c #B41B0B",
"*  c #B11A0B",
"#X c #C61E0C",
"W. c #C31D0C",
"uX c #C01C0C",
" O c #D5200D",
"3  c #D21F0D",
"%  c #D5200E",
"9o c #A22417",
"q  c #84261D",
"]X c #A22A1E",
"8X c #9C2B1F",
"aX c #9F2C22",
"c  c #8A2E26",
"UX c #A8382D",
"tX c #A53D33",
"]. c #A24238",
"dX c #AE4C43",
"<X c #B15C53",
"G. c #AE5E57",
"F. c #BA7771",
",  c #771107",
"^o c #831207",
"b  c #891408",
"m. c #861308",
">O c #8C1509",
"l. c #86160B",
"AX c #9B1709",
"-  c #981609",
"pX c #951509",
"ao c #AD1A0A",
"!  c #AA190A",
"to c #A7180A",
"w. c #C08882",
":O c #AA190B",
"e  c #AD1A0B",
"qo c #9E1B0D",
"4X c #BC1C0B",
"bo c #B91B0B",
"Mo c #B61A0B",
"P. c #BC1C0C",
"gX c #B91B0C",
"wo c #B61A0C",
"]o c #C81D0C",
";. c #CE1F0C",
"E. c #CB1E0C",
"n. c #D1200D",
"Z. c #CE1F0D",
"y  c #CB1E0D",
"K. c #CE1F0E",
"|  c #89261C",
"JX c #A42F24",
"MX c #A43529",
"7o c #AA372B",
")X c #AD4137",
"r. c #9B443A",
"IX c #AD443A",
">X c #AD5349",
"+X c #AD564D",
"U  c #A76058",
"%. c #AA615A",
"{. c #B66B64",
"   c None",
"R  c #B67A73",
"B. c #BC7C76",
"XO c #761208",
"#. c #7F1207",
"N  c #7C1107",
"y. c #851407",
"t. c #821307",
".O c #821308",
"Co c #911508",
"`o c #941609",
"Lo c #911509",
"yo c #A01709",
"1X c #91180B",
"1  c #A3180A",
"vo c #A0170A",
"0. c #851A0E",
"DX c #B21A0A",
"8. c #B51B0B",
"A. c #B21A0B",
"`. c #C71E0C",
":X c #C41D0C",
"%X c #C11C0C",
"GX c #9D1F12",
"o. c #D9210D",
":. c #D6200D",
"i. c #D31F0D",
"~  c #D6200E",
"2o c #A3271A",
"9  c #852920",
"k  c #C59793",
"3o c #A62E22",
"'  c #91362C",
"BX c #A6372C",
"(X c #A9382D",
"bX c #AF4C42",
"Y. c #AC5A52",
"q. c #AC635B",
"z. c #AF645D",
"a. c #B26B64",
"4. c #B26E68",
"/. c #B87068",
"6  c #781107",
"<  c #7B1207",
"'o c #7B1208",
"Qo c #841207",
"To c #841208",
"Ho c #8D1508",
"]  c #BB837D",
"6. c #8A1408",
"i  c #871308",
"xo c #9C1708",
"Oo c #9C1709",
"FX c #991609",
"fX c #961509",
"T. c #8A170B",
"~. c #8D180C",
"H. c #8A170C",
"io c #AE1A0A",
" X c #AB190A",
"!X c #A8180A",
"So c #AE1A0B",
"4  c #AB190B",
"go c #A8180B",
"$. c #841B10",
"xX c #BD1C0B",
"QX c #BA1B0B",
"w  c #7E1C12",
"!o c #BD1C0C",
"Do c #C01D0C",
";o c #9F1E10",
"6X c #961E11",
"%O c #CF1F0C",
"+. c #CC1E0C",
"(o c #CF1F0D",
"L. c #CC1E0D",
"{  c #C18E89",
"@  c #DB200D",
"X  c #DB200F",
"YX c #A22B20",
"I  c #8D3027",
"h  c #CAA09C",
"=o c #A83326",
"I. c #993429",
"-X c #9F362B",
"NX c #A5352A",
"C  c #90372E",
"a  c #903A31",
"*o c #AB3A2E",
"*X c #A23D32",
" o c #AB3D32",
"LX c #AB4035",
"E  c #99463D",
"_X c #AE443A",
"s  c #9F544D",
"sX c #B1544B",
"rX c #B1574F",
"Z  c #AB6761",
"c. c #B16963",
"_  c #831307",
"L  c #801207",
"H  c #7D1107",
"1O c #861408",
"Go c #891508",
"-. c #921508",
"@. c #8F1408",
"M. c #861409",
"&O c #921509",
"Yo c #8F1409",
"mo c #951609",
"p. c #86170B",
"W  c #80180D",
"do c #AA1A0A",
"K  c #A7190A",
"po c #A4180A",
"b. c #A1170A",
"<o c #9E190B",
"u. c #B91C0B",
"J. c #B61B0B",
"ro c #B31A0B",
"Uo c #C21C0C",
"). c #C81E0C",
"7. c #C51D0C",
"O. c #D7200C",
"}o c #D41F0D",
"X. c #D7200D",
"D  c #C08F8A",
"-O c #D7200E",
"o  c #DA210E",
"&. c #C3908B",
"G  c #86261D",
"/X c #A42D22",
"eX c #9E2E24",
"5o c #AA3529",
"KX c #A7372C",
"&o c #AD3F33",
"EX c #AA3E33",
"7X c #B35C53",
"}  c #AA625B",
",O c #761007",
"{o c #791107",
")o c #7C1207",
"[o c #791108",
",. c #881307",
"V  c #7C1208",
"Ao c #8E1508",
"Vo c #911608",
"!. c #8B1408",
"S. c #881308",
"l  c #B9827D",
"s. c #BF847E",
"p  c #7C150B",
";  c #8E1509",
"oo c #A01809",
",o c #9D1709",
"|. c #9A1609",
"j. c #9D170A",
"co c #97150A",
"@X c #AF1A0A",
"g. c #AC190A",
"Zo c #AF1A0B",
"Jo c #BB1B0B",
"qX c #C11D0C",
"0X c #BE1C0C",
"~o c #CD1E0D",
"&  c #D01F0D",
"^  c #D91F0C",
"+  c #DC200E",
"[  c #C59590",
"-o c #A3281B",
"HX c #A0271B",
"}X c #A32B20",
"0  c #882E26",
"VX c #A6382D",
"vX c #A93F34",
"`X c #AC4035",
"|X c #AC4036",
"=. c #9D4A42",
"D. c #A9544C",
"=X c #B5645C",
"z  c #A96660",
"OX c #B56760",
"#O c #751107",
"*O c #751108",
"9. c #841307",
"<. c #811207",
"+O c #871408",
":  c #841308",
"v  c #7B1309",
"5X c #931508",
"jX c #961609",
"wX c #931509",
"so c #AB1A09",
"n  c #A8190A",
"hX c #A5180A",
"f. c #871A0E",
"ZX c #991A0D",
"OO c #BA1C0B",
"iX c #B71B0B",
";O c #B71B0C",
"yX c #C01B0B",
"Po c #C31C0C",
"_. c #C91E0C",
"*. c #C18C87",
"$X c #C61D0C",
"+o c #9F1F11",
"TX c #9C1E11",
"r  c #CC1F0D",
"3X c #C31C0D",
"@O c #D8200D",
".  c #DB210D",
"(  c #D51F0D",
"8  c #812118",
"O  c #DB210E",
"^X c #9F2216",
"t  c #DB210F",
"g  c #C49692",
"8o c #A52A1D",
"kX c #9F2B1F",
"$o c #AB3B2F",
"1. c #963A31",
"N. c #AB5C54",
"oX c #B4655D",
"[. c #B76C64",
"<O c #771007",
"/o c #801307",
"P  c #7D1207",
"x. c #BD807A",
">  c #7A1107",
"Ko c #891308",
"e. c #BD837D",
"&X c #8F1508",
"5  c #8C1408",
"no c #951709",
"Bo c #9B1609",
">o c #A11809",
"eo c #9E1709",
"Eo c #A1180A",
"cX c #98180B",
"~X c #9B190C",
".. c #B01A0A",
")  c #AD190A",
".o c #AA180A",
"zX c #981B0E",
"2  c #B31B0B",
"uo c #B01A0B",
"ho c #A7170B",
">. c #C21D0C",
"R. c #BF1C0C",
"Y  c #C08D88",
"XX c #921F13",
"_o c #CE1E0D",
"1o c #A12113",
"J  c #D4200D",
"h. c #D11F0D",
"#  c #D4200E",
"5. c #892218",
"A  c #C39490",
"T  c #C69894",
"#o c #A73225",
"j  c #C99F9B",
"'X c #A73529",
"6o c #AA362A",
"mX c #A73B30",
"%o c #AD4035",
"CX c #AA4238",
"RX c #AD4339",
"(. c #A4463D",
"nX c #AD493F",
"lX c #B05046",
"F  c #A45B54",
",X c #B36057",
"^. c #B3665E",
"U. c #B9746D",
"$O c #761106",
"oO c #761107",
"3O c #761108",
" . c #851307",
"2. c #BC817B",
"k. c #851308",
"9X c #941508",
"No c #941509",
"'. c #971609",
"C. c #88170C",
"fo c #A91909",
"{X c #9A170A",
"lo c #AC1A0A",
"2X c #A9190A",
"u  c #A6180A",
"7  c #7C190F",
":o c #9D1B0D",
"SX c #BB1C0B",
"Xo c #B81B0B",
"=O c #B81B0C",
"Ro c #CA1E0C",
"Io c #C71D0C",
"3. c #C28C87",
"0o c #A01F11",
"[X c #9D1E11",
"m  c #CD1F0D",
"$  c #CA1E0D",
"/  c #D9200D",
"}. c #942419",
"@o c #A3291C",
"S  c #C89D98",
"4o c #A93427",
"PX c #AF483E",
"x  c #9A4A43",
"V. c #AF635C",
"d. c #AF665E",
"d  c #AC6B65",
/* pixels */
"                                                                                                                                                                ",
"                                                                            . X X o                                                                             ",
"                                                              . O O O O O O O O O O O O O O O O +                                                               ",
"                                                        @ O O O O O O O O O # $ $ % O O O O O O O O O O                                                         ",
"                                                  O O O O O O & * = - ; : > , , , , < : ; - 1 2 3 O O O O O o                                                   ",
"                                              O O O O O $ 4 5 6 6 6 6 6 6 7 8 9 0 q w 6 6 6 6 6 6 ; e r O O O O t                                               ",
"                                          O O O O y u i > > p a s d f g h h h h h h h j k l z x c v > > b n m O O O t                                           ",
"                                      O O O O & M N B V C Z A S S S S S S S S S S S S S S S S S S D F G B B H = 3 O O O O                                       ",
"                                    O O O J K L P I U Y T T T T T T T T T T T T T T T T T T T T T T T T R E W P Q ! ~ O O O                                     ",
"                                ^ / / ( ) _ ` ' ] [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ { } | `  ...X./ / o.                                ",
"                              O.X.X.+.@.#.$.%.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.*.=.#.#.-.;.X.X.X.                              ",
"                            :.:.:.>.,.<.1.2.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.3.4.5.<.6.7.:.:.X.                            ",
"                          J J J 8.9.0.q.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.w.e.r.t.y.u.J J J                           ",
"                        i.i.3 n 9.p.a.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.s.d.f.9.g.i.i.i.                        ",
"                      h.h.& j.k.l.z.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.c.v.k.b.h.h.n.                      ",
"                    & & & u m.M.N.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.V.C.m.! & & &                     ",
"                  ;.Z.Z.A.S.S.D.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.F.G.H.S.J.Z.Z.K.                  ",
"                  L.L.P.6.b I.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.U.Y.T.5 R.L.L.                  ",
"                E.E.W.Q.!.~.^./././././././././././././././././././././././././././././././././././././././././././././././././././.(.!.-.7.E.E.                ",
"              )._.`.'.5 5 ].[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.{.}.5 |.)._._.              ",
"              ).). X.X.XXXoXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOXOX+X.X.X@X).).              ",
"            #X$X%X-.&X&X*X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X=X-X&X;XW.$X$X            ",
"            :X:Xu Q.Q.Q.>X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X<X1XQ.2X:X:X            ",
"          3XW.4X5X-.-.6X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X7X8X-.9X0XW.W.          ",
"          qXqX1 wXwXwXeXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXrXtXwXwXu qXqX          ",
"        yXuXiXpXpXpXpXaXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXsXdXpXpXfXgXuXuX        ",
"        0X0XhXjXjXjXjXkXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXlXzXjXjXK 0X0X        ",
"        xX4X|.'.'.'.'.cXvXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXbXnXmXMXNXBXVXCXbXbXbXbXbXbXbXbXZX'.'.AXxXxX        ",
"      SXSXDXFXFXFXFXFXFXFXGXHXJXKXLXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXPXIXUXYXGXFXFXFXFXFXFXFXTXJXRXPXPXPXPXEXFXFXFXFXWXSXSX      ",
"      QXQX!X|.|.|.|.|.|.|.|.|.|.|.|.~X^X/X(X)X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X_X`X'X]X[X|.|.|.|.|.|.|.|.|.|.|.|.|.{X}X|X_X o[X|.|.|.|..oQXQX      ",
"      XoXoooOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOo+o@o#o$o%o%o%o%o%o%o%o%o%o%o%o%o%o%o%o&o*o=o-o;oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOo:oOoOoOoOoOoOo>oXoXo      ",
"    iXJ.* ,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o<o1o2o3o4o4o5o6o7o4o3o8o9o0oqo,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o2 J.wo    ",
"    8.8.! eoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeoeog.8.8.    ",
"    rorotoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyoyo!Xroro    ",
"    A.A.u b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.toA.A.    ",
"    uouohX1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 u uouo    ",
"    ioiohXpopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopohXioio    ",
"  ao) ) hXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhXhX) )     ",
"   X X! totototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototototo X Xso  ",
"  do! 2Xn n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n n ! ! fo  ",
"  2Xn 2X! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! n n go  ",
"  u to!X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X Xtotoho  ",
"  johXhXg.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.g.hXhX    ",
"    1 1 ) kokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokokoko) 1 1     ",
"    = = ao@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@X@Xlo= =     ",
"    yoyo) * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  Xyoyo    ",
"    zozo XA.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.A.! zozo    ",
"    ,o,o!Xrorororororororororororororororororororororororororororororororororororororororororororororororororororororororororororororororororororororoto,o,o    ",
"    ,oOo>o8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.8.ooOoxo    ",
"      |.|.roJ.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.J.* |.|.      ",
"      - -  XXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXoXo2X- -       ",
"      co'.voboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboboeo'.'.      ",
"      nomojXXoQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXQXMomomo        ",
"        NoNoio4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4Xg.NoNo        ",
"        -.-.,oxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXBo-.-.        ",
"        VoCoCoA.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.R.ZoCoCo          ",
"          &X&X- uXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXR.jX&XAo          ",
"            .X.XSoqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqX X.X.X            ",
"            5 5 5XqXW.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.W.DoCo5 Fo            ",
"              6.6.! :X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:X:Xu 6.6.              ",
"              Gob HoxX$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$X$XJo!.b Ko              ",
"                i i LoPoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoUoYoi i                 ",
"                  k.k.FX`.).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).).`.jXk.k.                  ",
"                  To: : hXRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoRoEo: : :                   ",
"                    Wot.t.DXE.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.E.iot.t.t.                    ",
"                      <.<.Qo!o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~o~oQX^o<.<.                      ",
"                        /o#.t.WXZ.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.* <.#.#.                        ",
"                          ` ` ` K (o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(o(oZ.1 ` ` `                           ",
"                            )o)o)o- _oh.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.h.L.`o)o)o<                             ",
"                              'oB B !.]o3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 7.KoB B N                               ",
"                                [o{o{oL ) 3 }o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}o}oh.! ` {o{o|o                                ",
"                                    , , , : 8. O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O O OJ A..O, , ,                                     ",
"                                      XOoOoOoO6.xX:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.OO+OoOoOoOoO                                      ",
"                                          oOoOoOoOLoXo}o@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@O@Oi.J.AooOoOoO#O                                          ",
"                                              $OoOoOoO{oNoroh./ / / / / / / / / / / / / / / / / / %O* &O6 oOoOoOoO                                              ",
"                                                *O$OoOoOoOoO, ; 4 =O>.m -OO O O O O O -Or >.;O:O>OoOoOoOoOoO,O                                                  ",
"                                                      <OoOoOoOoOoOoOoOoOoO)o1OLo2O1O)ooOoOoOoOoOoOoOoOoO#O                                                      ",
"                                                              XOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO$O                                                              ",
"                                                                          oO3OoO4O$O3O                                                                          ",
"                                                                                                                                                                "
};

/* XPM */
static char *button_seagreen_benji_pa_01_xpm[] = {
/* columns rows colors chars-per-pixel */
"80 80 398 2",
"lo c #0AAA8D",
"/  c #0AAA8E",
"jo c #0AAD8F",
"y  c #0AAD90",
"`  c #0AAD91",
"WX c #1FA28B",
"o. c #0AB093",
"io c #0AB395",
"Vo c #0AB396",
"*o c #25A78F",
"6o c #22A68F",
"AX c #2BA690",
"Oo c #31AB95",
"{X c #34AC96",
"g  c #4C9F90",
"8X c #0DC3A3",
"DX c #37AA96",
"TX c #3DAF9A",
"u  c #0DCCAA",
"5O c #0DCCAB",
"gX c #43AE9B",
".O c #0DCFAD",
"5  c #0DD2AF",
"3X c #49AD9C",
"&  c #0DD5B2",
";O c #0DD8B4",
".  c #0DDBB6",
"+  c #0DDBB7",
"P. c #6DB9AC",
"x. c #79BDB1",
"e. c #82C0B5",
"1  c #067763",
"<  c #067A65",
"P  c #067D67",
"Y  c #067D68",
"U  c #06806A",
"'  c #06836D",
"Eo c #068972",
"^o c #099980",
"$o c #099C81",
"Z  c #099F84",
";  c #09A287",
"lX c #09A589",
"go c #09A58A",
"bo c #09A88B",
"(X c #09A88C",
"ko c #09AB8F",
"`X c #21A48C",
"b  c #429A8A",
"7o c #27A992",
"9o c #2AAA94",
"/. c #3CA491",
"rX c #0CBE9F",
";o c #33AD96",
"*X c #0CC1A1",
"<X c #0CC4A4",
"'. c #0CC7A6",
")o c #0CCAA8",
"/o c #0CC7A7",
"]o c #0CCDAB",
"NX c #42AF9C",
"xX c #45B09D",
")  c #0CD9B6",
"4X c #57B3A3",
"4. c #7BBCB0",
"E  c #87C0B6",
"l  c #9CCAC2",
"qO c #057562",
"a. c #0B866F",
"g. c #0E8771",
"q  c #17816E",
">  c #088E76",
"Po c #088E77",
"Ko c #089178",
"!o c #089179",
"eX c #08947B",
"]. c #08977E",
"nX c #089A80",
")X c #0B9B81",
".X c #089A81",
"3o c #0B9E84",
"LX c #119D84",
"eo c #11A087",
"[  c #2C917F",
"PX c #1AA088",
"   c None",
"wo c #17A28A",
"L. c #299985",
"5o c #1AA38B",
"to c #0BB395",
"4  c #0BB396",
"H. c #0BB698",
"ZX c #29A58F",
"-X c #32A28E",
"u. c #0BB99A",
"iX c #32A590",
"SX c #2CA691",
"'X c #2CA992",
"VX c #2FA792",
"K. c #0BBC9D",
"Lo c #0BBF9F",
"T. c #0BBFA0",
"1. c #0BC2A2",
"oo c #35AC97",
"!X c #38AD98",
"F. c #56AE9E",
"qX c #53B3A1",
"V  c #0EDBB6",
"@  c #0EDBB7",
"*  c #0EDBB8",
"D. c #71BAAD",
"{  c #7DBBB0",
";. c #86C1B7",
"x  c #92C5BB",
"%O c #07806A",
"9O c #07866F",
"#O c #078971",
"n. c #078670",
"M  c #078972",
"Y. c #078C74",
"U. c #0A8A74",
"[. c #078C75",
"%. c #078F77",
"r  c #1C8471",
">. c #07927A",
"w  c #1F8573",
"n  c #258A78",
"1o c #0D9D83",
"Mo c #0AA588",
"D  c #2E907E",
"N  c #0AA88C",
"no c #0AAB8E",
"7  c #0AAB8F",
"fo c #0AAE91",
"Xo c #1FA38B",
"Bo c #0AB193",
"^X c #0AB496",
"aX c #0AB799",
"[o c #0ABA9B",
"UX c #2BA791",
"{. c #37A290",
":o c #2EAB94",
"0o c #2BAA94",
"%  c #0DCAA9",
"S. c #4CA998",
"B  c #0DCDAB",
"C  c #0DD0AD",
":O c #0DD3AF",
"=  c #0DD0AE",
"R  c #58A799",
"i. c #0DD3B0",
"<. c #0DD6B3",
"+. c #0DD9B4",
"_  c #0DD9B5",
"f. c #5EAFA0",
"6. c #67B2A5",
"}. c #64B7A8",
"Q  c #73B6AA",
">O c #067562",
"0O c #067563",
"9  c #067864",
"&O c #067865",
"2  c #067B66",
"+O c #067B67",
" O c #067E68",
"]  c #067E69",
"^  c #06816B",
"|o c #06816C",
"'o c #06846E",
"~  c #0C806C",
"*. c #0F846F",
"Co c #099D81",
".o c #099A80",
"k. c #099D83",
"#o c #09A084",
"ao c #09A085",
"Do c #09A086",
"tX c #09A387",
"3  c #09A388",
"do c #09A68A",
"Ro c #09A68B",
"zo c #09A98D",
",o c #1BA38A",
"dX c #219F89",
"yo c #0CB698",
"!  c #3C9989",
"~o c #0CC2A2",
"). c #0CC5A4",
"@O c #0CC8A6",
"9. c #0CC5A5",
"RX c #39AD98",
"XX c #0CC8A7",
"_. c #0CC8A8",
"R. c #0CCBA9",
"`o c #0CCBAA",
"C. c #0CCEAC",
"K  c #54A496",
"<O c #0CD1AE",
",. c #0CCEAD",
"I. c #51AC9C",
"uX c #4EB1A0",
"@. c #0CD7B4",
"X  c #0FDBB6",
"F  c #60AB9D",
"i  c #0FDBB7",
"o  c #0FDBB8",
"~. c #5DB3A3",
"7O c #057662",
"0  c #0E7C69",
"q. c #0E8570",
"m. c #088671",
"G. c #0B8A73",
"!. c #0B8D75",
"8  c #088C75",
"=X c #088F77",
"(. c #089279",
"Go c #08927A",
"sX c #08957C",
"XO c #08987E",
"Fo c #08957D",
"Ao c #08987F",
"So c #089880",
"GX c #089B82",
"T  c #268D7B",
"vo c #0BA88C",
"yX c #239E88",
"|X c #1DA28A",
"qo c #1DA58D",
"IX c #23A48D",
"-  c #0BB194",
"}o c #0BB497",
">o c #26A890",
"2o c #0BB798",
"6O c #0BB799",
"/X c #0BBA9B",
"bX c #0BBD9D",
"Ho c #0BBD9E",
"=o c #2FAB94",
"pX c #0BC0A0",
"jX c #0BC0A1",
"E. c #0BC3A2",
"-o c #35AD97",
"BX c #3EAD9A",
"fX c #4AB19F",
"M. c #53AB9C",
"(  c #0ED6B3",
"=. c #59AA9C",
"v  c #5FA99C",
"#  c #0EDCB7",
"5X c #53B1A1",
"@X c #5CB4A5",
"#X c #5FB5A6",
"^. c #68B8A9",
"c  c #7DB9AE",
"5. c #86C2B7",
"J  c #89C0B7",
"}  c #8FC5BB",
"H  c #98C8C0",
"z  c #9BC9C1",
"d  c #0A7C68",
"p. c #07846D",
",  c #07846E",
"s  c #078770",
"To c #078D74",
"8. c #078A73",
"Wo c #078D75",
"OX c #078D76",
"W. c #079077",
"e  c #258877",
"cX c #0D9880",
"QX c #109C83",
"<o c #109F84",
"ro c #0D9E85",
"a  c #0AA68A",
"xo c #0AA98B",
"1X c #0AA68B",
"co c #0AA98C",
"7X c #0AA98D",
"&o c #1CA38A",
"h. c #0AAC8F",
"No c #0AAC90",
"%X c #0AAF92",
":X c #2B9F8A",
"so c #0AB294",
"JX c #0AB295",
"t. c #3A9B8A",
"CX c #28A48F",
"}X c #28A790",
"YX c #34AB96",
"p  c #0DCBA9",
"oO c #0DCEAB",
"Z. c #0DCEAC",
"OO c #0DD1AE",
"-O c #0DD4B0",
"j. c #0DD1AF",
"$  c #0DD4B1",
"O. c #0DD7B3",
"#. c #0DD7B4",
"w. c #5BAC9E",
"B. c #5BAFA0",
"h  c #64AC9F",
"s. c #64B2A3",
"|  c #88C1B7",
"k  c #91C4BB",
"*O c #067662",
"$O c #067964",
",O c #067965",
"A  c #067C67",
"&. c #067F6A",
"{o c #06826D",
"v. c #0C8772",
".. c #1B8975",
"+X c #12927C",
":  c #09987F",
"Jo c #099B81",
"FX c #0C9980",
" X c #18947E",
"uo c #099E84",
"b. c #09A186",
"_X c #159F86",
"ho c #09A489",
"mo c #09A78A",
"f  c #30907F",
"po c #09A78B",
"vX c #09A78C",
"+o c #09AA8D",
"zX c #1E9F88",
"3. c #309684",
"Uo c #0CC0A0",
"MX c #33A994",
"Io c #0CC3A2",
"]X c #36AD97",
",X c #0CC3A3",
"&X c #0CC6A5",
"[X c #39AE99",
"`. c #0CC9A8",
"oX c #0CC9A9",
"$. c #0CCCAB",
"1O c #0CCFAD",
" . c #5AAA9C",
"|. c #63B6A8",
"j  c #7BB8AD",
"N. c #75BCAF",
"d. c #7EBFB3",
"-. c #8AC3B9",
"W  c #93C6BD",
"8O c #057763",
"m  c #087B66",
"_o c #08816C",
"t  c #117E6B",
"(o c #08846F",
"V. c #0B8872",
"2X c #089078",
"9X c #08937A",
"7. c #178975",
"6X c #0B917A",
">X c #08937B",
"hX c #08967D",
"KX c #08997F",
"0X c #11967E",
"Zo c #089C82",
"%o c #119F85",
"A. c #0BB295",
"0. c #0BB597",
"@o c #0BB899",
"2O c #0BB89A",
"EX c #2CA892",
"HX c #0BBB9C",
"8o c #29AA93",
":. c #419D8D",
"kX c #0BBE9F",
"~X c #32AA95",
"J. c #0ECEAB",
"$X c #4DAD9C",
"4O c #0ED7B3",
"3O c #0ED7B4",
"6  c #0EDAB7",
"z. c #5CAFA0",
"O  c #0EDAB8",
";X c #5CB5A5",
"c. c #62B1A4",
"r. c #7DBDB2",
"G  c #8FC3BA",
"=O c #077662",
"S  c #077C67",
"y. c #07826C",
"X. c #07856F",
"2. c #078871",
"l. c #0A8671",
"Q. c #078B73",
"Qo c #078B74",
"Yo c #079178",
"L  c #1C8673",
"mX c #0A987F",
" o c #109D84",
"I  c #0AA78B",
"wX c #1F9C85",
"4o c #13A188",
/* pixels */
"                                                                                                                                                                ",
"                                                                            . X o O                                                                             ",
"                                                              + @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ #                                                               ",
"                                                        + @ @ @ @ @ @ @ @ @ $ % % & @ @ @ @ @ @ @ @ @ @                                                         ",
"                                                  * @ @ @ @ @ = - ; : > , < 1 1 1 1 2 , > : 3 4 5 @ @ @ @ @ 6                                                   ",
"                                              @ @ @ @ @ % 7 8 9 9 9 9 9 9 0 q w e r t 9 9 9 9 9 9 > y u @ @ @ @ i                                               ",
"                                          * @ @ @ p a s < < d f g h j k l l l l l l l z x c v b n m < < M N B @ @ @ i                                           ",
"                                      V @ @ @ C Z A 2 S D F G H H H H H H H H H H H H H H H H H H J K L 2 2 P ; 5 @ @ @ @                                       ",
"                                    @ @ @ $ I U Y T R E W W W W W W W W W W W W W W W W W W W W W W W W Q ! ~ Y ^ / ( @ @ @                                     ",
"                                ) _ _ & ` ' ] [ { } } } } } } } } } } } } } } } } } } } } } } } } } } } } } |  ...] X.o.O._ _ +.                                ",
"                              @.#.#.$.%.&.*.=.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.;.:.&.&.>.,.#.#.#.                              ",
"                            <.<.<.1.2.^ 3.4.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.5.6.7.^ 8.9.<.<.O.                            ",
"                          $ $ $ 0., q.w.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.r.t.y.X.u.$ $ $                           ",
"                        i.i.5 N p.a.s.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.d.f.g.p.h.i.i.i.                        ",
"                      j.j.= k.X.l.z.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.c.v.X.b.j.j.j.                      ",
"                    C C C a n.m.M.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.B.V.n./ C C C                     ",
"                  C.Z.Z.A.2.2.S.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.D.F.G.2.H.Z.Z.J.                  ",
"                  $.$.K.8.M L.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.P.I.U.Y.T.$.$.                  ",
"                R.R.E.W.Q.!.~.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^./.Q.(.).R.R.                ",
"              _.`.'.].[.[.{.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.}.|. X[..XXX`.oX              ",
"              XXXX7 OXOX+X@X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X#X$XOXOX%XXXXX              ",
"            &X&X*X(.=X=X-X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X;X:X=X>X,X&X&X            ",
"            <X<X1X2X2X2X3X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X4X5X6X2X7X<X<X            ",
"          8X,XK.9X(.(.0XqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXqXwX(.eXrX,X,X          ",
"          *X*XtX9X9X9XyXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXuXiX9X9Xa *X*X          ",
"        pXpXaXsXsXsXsXdXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXfXgXsXsXhXu.pXjX        ",
"        kXkXlXhXhXhXhXzXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXcXhXhXvXkXkX        ",
"        bXK.nX].].].].mXMXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXNXBXVXCXZXAXSXDXNXNXNXNXNXNXNXNXFX].].GXbXbX        ",
"      HXHXJXKXKXKXKXKXKXKXLXPXIXUXYXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXTXRXEXWXLXKXKXKXKXKXKXKXQXIX!XTXTXTXTX~XKXKXKXKX^XHXHX      ",
"      /X/X(XnXnXnXnXnXnXnXnXnXnXnXnX)X_X`X'X]X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X[X{X}X|X onXnXnXnXnXnXnXnXnXnXnXnXnX.oXooo[XOo onXnXnXnX+o/X/X      ",
"      @o@o#o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o%o&o*o=o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o;o:o>o,o<o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o$o1o$o$o$o$o$o$ob.@o@o      ",
"    2oH.- k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.3o4o5o6o7o7o8o9o0o7o6oqowoeorok.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.k.toH.yo    ",
"    0.0./ uouououououououououououououououououououououououououououououououououououououououououououououououououououououououououououououououououououououoh.0.0.    ",
"    ioiopoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoao(Xioio    ",
"    sosodob.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.pososo    ",
"    o.o.lXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXtXdoo.o.    ",
"    fofogohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohohogofofo    ",
"  joy y gogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogogoy y     ",
"  7 7 / popopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopopo7 7 ko  ",
"  / lo7XN N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N N lolozo  ",
"  xoN cololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololololoN N vo  ",
"  1Xpobononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononononopopomo  ",
"  MolXlXNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNoNolXlX    ",
"    3 3 ` fofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofofoy 3 3     ",
"    ; ; y %X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%XNo; ;     ",
"    aoaojoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBoBonoaoao    ",
"    Z Z nososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososososoloZ Z     ",
"    k.k.(XVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVoVopok.k.    ",
"    Co$ob.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.#o$oZo    ",
"      nXnXtoH.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.H.- nXnX      ",
"      AoAono@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o@o7XAoSo      ",
"      ].].Dou.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.u.uo].].      ",
"      FosXhX@o/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/X/XH.sXsX        ",
"        eXeXfoK.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.K.h.eXeX        ",
"        GoGok.HoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoHoJoGoGo        ",
"        KoKoKosoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLoLo%XKoKo          ",
"          =X=X: pXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXpXLohX=XPo          ",
"            OXOXfo*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X*X7 OXOX            ",
"            Y.Y.9X*XIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoIoUoYoY.To            ",
"              8.8./ <X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<X<XRo8.8.              ",
"              EoM WoHo&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&X&XHXQoM M               ",
"                s s !o,X'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.~o=Xs s                 ",
"                  X.X.^o/oXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'.hXX.X.                  ",
"                  (o, , go)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)o)ob., , ,                   ",
"                    _oy.y.JX`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`o`ofoy.y.y.                    ",
"                      ^ ^ 'oHo]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o]o[o' ^ ^                       ",
"                        U &.{o}oZ.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Z.Bo|o&.&.                        ",
"                           O O OI .O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.O.OZ.tX O O O                          ",
"                            A A A XOoOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOu eXA A +O                            ",
"                              +O2 2 Qo@O5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 ).#O2 2 S                               ",
"                                $O$O$O%Oy 5 $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ j./ ] $O$O&O                                ",
"                                    1 1 1 , 0.& & & & & & & & & & & & & & & & & & & & & & & & & & & & & & & & & $ JXy.1 1 1                                     ",
"                                      *O*O*O*O8.Ho<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<.<./Xs *O*O*O=O                                      ",
"                                          *O*O*O*O!o@o-O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O;O:OH.> *O*O*O>O                                          ",
"                                              *O*O*O*O,OeXio<O_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 1OBo(.9 *O*O*O*O                                              ",
"                                                >O*O*O*O*O*O1 > 7 2O~oB 3O@ @ @ @ @ @ 4O5O~o6O/ 8 *O*O*O*O*O7O                                                  ",
"                                                      8O*O*O*O*O*O*O*O*O*OA n.Ko2X9OA *O*O*O*O*O*O*O*O*O0O                                                      ",
"                                                              *O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O*O                                                              ",
"                                                                          *O*O7OqO*O*O                                                                          ",
"                                                                                                                                                                "
};

////@end XPM images

/*!
 * frmNode type definition
 */

IMPLEMENT_CLASS( frmNode, wxMDIChildFrame )

/*!
 * frmNode event table definition
 */

BEGIN_EVENT_TABLE( frmNode, wxMDIChildFrame )

////@begin frmNode event table entries
  EVT_CLOSE( frmNode::OnCloseWindow )
  EVT_MENU_CLOSE( frmNode::OnMenuClose )
  EVT_WINDOW_DESTROY( frmNode::OnDestroy )

  EVT_BUTTON( ID_BUTTON_ON, frmNode::OnButtonOnClick )

  EVT_BUTTON( ID_BUTTON_OFF, frmNode::OnButtonOffClick )

////@end frmNode event table entries
	
	EVT_COMMAND( ID_FRMNODE, wxUPDATE_EVENT, frmNode::OnUpdate )
	EVT_COMMAND( ID_FRMNODE, wxS1_EVENT, frmNode::OnS1Update )
	EVT_COMMAND( ID_FRMNODE, wxS2_EVENT, frmNode::OnS2Update )
	EVT_COMMAND( ID_FRMNODE, wxSB_EVENT, frmNode::OnSBUpdate )
	EVT_COMMAND( ID_FRMNODE, wxBLOCK_EVENT, frmNode::OnBlockUpdate )
	EVT_COMMAND( ID_FRMNODE, wxDRIFT_EVENT, frmNode::OnDriftUpdate )
	EVT_COMMAND( ID_FRMNODE, wxPROGRESS_EVENT, frmNode::OnProgressUpdate )
	EVT_COMMAND( ID_FRMNODE, wxVOLTAGE_EVENT, frmNode::OnVoltageUpdate )
	EVT_COMMAND( ID_FRMNODE, wxTEMPERATURE_EVENT, frmNode::OnTemperatureUpdate )
	EVT_COMMAND( ID_FRMNODE, wxMESSAGE_EVENT, frmNode::OnWrkThreadMsg )
	EVT_COMMAND( ID_FRMNODE, wxLOG_EVENT, frmNode::OnLogMsg )
	EVT_COMMAND( ID_FRMNODE, wxTERMINATE_EVENT, frmNode::OnTerminateMsg )

END_EVENT_TABLE()

/*!
 * frmNode constructors
 */

frmNode::frmNode()
{
	m_nodeIndex = -1;		// No node controlled yet
	
	Init();
}

frmNode::frmNode( wxMDIParentFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	Init();
	Create( parent, id, caption, pos, size, style );
}

/*!
 * frmNode creator
 */

bool frmNode::Create( wxMDIParentFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin frmNode creation
  wxMDIChildFrame::Create( parent, id, caption, pos, size, style );

  this->SetBackgroundColour(wxColour(229, 229, 229));
  CreateControls();
  SetIcon(GetIconResource(wxT("wxwin32x321.xpm")));
////@end frmNode creation
    return true;
}

/*!
 * frmNode destructor
 */

frmNode::~frmNode()
{
////@begin frmNode destruction
////@end frmNode destruction
}

/*!
 * Member initialisation 
 */

void frmNode::Init()
{
////@begin frmNode member initialisation
  m_Bitmap_S1 = NULL;
  m_Bitmap_S2 = NULL;
  m_Bitmap_SB = NULL;
  m_Bitmap_Block = NULL;
  m_Bitmap_Drift = NULL;
  m_labelTemperature = NULL;
  m_Progressbar = NULL;
  m_labelVoltage = NULL;
////@end frmNode member initialisation
  m_bRun = true;    
}

/*!
 * Control creation for frmNode
 */

void frmNode::CreateControls()
{    
////@begin frmNode content construction
  frmNode* itemMDIChildFrame1 = this;

  wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(9, 3, 0, 0);
  itemMDIChildFrame1->SetSizer(itemFlexGridSizer2);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Bitmap_S1 = new wxStaticBitmap;
  m_Bitmap_S1->Create( itemMDIChildFrame1, ID_ANIMATIONCTRL1, itemMDIChildFrame1->GetBitmapResource(wxT("button-black_benji_park_01.png")), wxDefaultPosition, wxSize(80, 80), 0 );
  itemFlexGridSizer2->Add(m_Bitmap_S1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Bitmap_S2 = new wxStaticBitmap;
  m_Bitmap_S2->Create( itemMDIChildFrame1, ID_ANIMATIONCTRL, itemMDIChildFrame1->GetBitmapResource(wxT("button-blue_benji_park_01.png")), wxDefaultPosition, wxSize(80, 80), 0 );
  itemFlexGridSizer2->Add(m_Bitmap_S2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText9 = new wxStaticText;
  itemStaticText9->Create( itemMDIChildFrame1, wxID_STATIC, _("S1"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer2->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText11 = new wxStaticText;
  itemStaticText11->Create( itemMDIChildFrame1, wxID_STATIC, _("S2"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer2->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Bitmap_SB = new wxStaticBitmap;
  m_Bitmap_SB->Create( itemMDIChildFrame1, ID_ANIMATIONCTRL2, itemMDIChildFrame1->GetBitmapResource(wxT("button-green_benji_park_01.png")), wxDefaultPosition, wxSize(80, 80), 0 );
  itemFlexGridSizer2->Add(m_Bitmap_SB, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemMDIChildFrame1, wxID_STATIC, _("Slutnings beredd"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer2->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Bitmap_Block = new wxStaticBitmap;
  m_Bitmap_Block->Create( itemMDIChildFrame1, ID_ANIMATIONCTRL3, itemMDIChildFrame1->GetBitmapResource(wxT("button-red_benji_park_01.png")), wxDefaultPosition, wxSize(80, 80), 0 );
  itemFlexGridSizer2->Add(m_Bitmap_Block, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Bitmap_Drift = new wxStaticBitmap;
  m_Bitmap_Drift->Create( itemMDIChildFrame1, ID_STATICBITMAP, itemMDIChildFrame1->GetBitmapResource(wxT("button-seagreen_benji_pa_01.png")), wxDefaultPosition, wxSize(80, 80), 0 );
  itemFlexGridSizer2->Add(m_Bitmap_Drift, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText21 = new wxStaticText;
  itemStaticText21->Create( itemMDIChildFrame1, wxID_STATIC, _("Block"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer2->Add(itemStaticText21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxStaticText* itemStaticText23 = new wxStaticText;
  itemStaticText23->Create( itemMDIChildFrame1, wxID_STATIC, _("Drift"), wxDefaultPosition, wxDefaultSize, 0 );
  itemFlexGridSizer2->Add(itemStaticText23, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton27 = new wxButton;
  itemButton27->Create( itemMDIChildFrame1, ID_BUTTON_ON, _("ON"), wxDefaultPosition, wxSize(70, 50), 0 );
  itemFlexGridSizer2->Add(itemButton27, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton29 = new wxButton;
  itemButton29->Create( itemMDIChildFrame1, ID_BUTTON_OFF, _("OFF"), wxDefaultPosition, wxSize(70, 50), 0 );
  itemFlexGridSizer2->Add(itemButton29, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_labelTemperature = new wxStaticText;
  m_labelTemperature->Create( itemMDIChildFrame1, wxID_STATIC, _("20.3 C"), wxDefaultPosition, wxDefaultSize, 0 );
  m_labelTemperature->SetForegroundColour(wxColour(74, 146, 92));
  itemFlexGridSizer2->Add(m_labelTemperature, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_Progressbar = new wxGauge;
  m_Progressbar->Create( itemMDIChildFrame1, ID_GAUGE1, 100, wxDefaultPosition, wxSize(-1, 10), wxGA_HORIZONTAL );
  m_Progressbar->SetValue(1);
  if (ShowToolTips())
    m_Progressbar->SetToolTip(_("VSCP activity monitor"));
  itemFlexGridSizer2->Add(m_Progressbar, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_labelVoltage = new wxStaticText;
  m_labelVoltage->Create( itemMDIChildFrame1, wxID_STATIC, _("11.3V"), wxDefaultPosition, wxDefaultSize, 0 );
  m_labelVoltage->SetHelpText(_("System voltage Level"));
  if (ShowToolTips())
    m_labelVoltage->SetToolTip(_("System voltage Level"));
  m_labelVoltage->SetForegroundColour(wxColour(74, 146, 92));
  itemFlexGridSizer2->Add(m_labelVoltage, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Connect events and objects
  itemMDIChildFrame1->Connect(ID_FRMNODE, wxEVT_DESTROY, wxWindowDestroyEventHandler(frmNode::OnDestroy), NULL, this);
////@end frmNode content construction


}

/*!
 * Should we show tooltips?
 */

bool frmNode::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap frmNode::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin frmNode bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("button-black_benji_park_01.png"))
  {
    wxBitmap bitmap(button_black_benji_park_01_xpm);
    return bitmap;
  }
  else if (name == _T("button-blue_benji_park_01.png"))
  {
    wxBitmap bitmap(button_blue_benji_park_01_xpm);
    return bitmap;
  }
  else if (name == _T("button-green_benji_park_01.png"))
  {
    wxBitmap bitmap(button_green_benji_park_01_xpm);
    return bitmap;
  }
  else if (name == _T("button-red_benji_park_01.png"))
  {
    wxBitmap bitmap(button_red_benji_park_01_xpm);
    return bitmap;
  }
  else if (name == _T("button-seagreen_benji_pa_01.png"))
  {
    wxBitmap bitmap(button_seagreen_benji_pa_01_xpm);
    return bitmap;
  }
  return wxNullBitmap;
////@end frmNode bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon frmNode::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin frmNode icon retrieval
  wxUnusedVar(name);
  if (name == _T("wxwin32x321.xpm"))
  {
    wxIcon icon(wxwin32x32_xpm);
    return icon;
  }
  return wxNullIcon;
////@end frmNode icon retrieval
}






/////////////////////////////////////////////////////////////////// 
// setBitmap
//

void frmNode::setBitmap( wxStaticBitmap *pBitmap, int nBitmap )
{
	switch ( nBitmap ) {
	
		case LAMP_BLACK:
			{	
				wxBitmap bitmap(button_black_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_BLUE:
			{	
				wxBitmap bitmap(button_blue_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_GREEN:
			{	
				wxBitmap bitmap(button_green_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_PURPLE:
			{	
				wxBitmap bitmap(button_purple_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_RED:
			{	
				wxBitmap bitmap(button_red_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_SEAGREEN:
			{	
				wxBitmap bitmap(button_seagreen_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		case LAMP_YELLOW:
			{	
				wxBitmap bitmap(button_yellow_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
	
		default:
			{	
				wxBitmap bitmap(button_black_xpm);
				pBitmap->SetBitmap( bitmap );
			}
			break;
			
	}
	
	Refresh();
	
}

 
/////////////////////////////////////////////////////////////////// 
// OnUpdate
//

void frmNode::OnUpdate( wxCommandEvent &event )
{
	Refresh();
	event.Skip();
} 

/////////////////////////////////////////////////////////////////// 
// OnS1Update
//

void frmNode::OnS1Update( wxCommandEvent &event )
{
	setBitmap( m_Bitmap_S1, event.GetInt() );
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnS2Update
//

void frmNode::OnS2Update( wxCommandEvent &event )
{
	setBitmap( m_Bitmap_S2, event.GetInt() );
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnSBUpdate
//

void frmNode::OnSBUpdate( wxCommandEvent &event )
{
	setBitmap( m_Bitmap_SB, event.GetInt() );
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnBlockUpdate
//

void frmNode::OnBlockUpdate( wxCommandEvent &event )
{
	setBitmap( m_Bitmap_Block, event.GetInt() );
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnDriftUpdate
//

void frmNode::OnDriftUpdate( wxCommandEvent &event )
{
	event.SetInt(1);
	setBitmap( m_Bitmap_Drift, event.GetInt() );
	event.Skip();
}


/////////////////////////////////////////////////////////////////// 
// OnProgressUpdate
//

void frmNode::OnProgressUpdate( wxCommandEvent &event )
{
	unsigned char val = event.GetInt();
	
	if ( val > 100 ) val = 0;
	m_Progressbar->SetValue( val );
	event.Skip();
}

/////////////////////////////////////////////////////////////////// 
// OnTemperatureUpdate
//

void frmNode::OnTemperatureUpdate( wxCommandEvent &event )
{
	m_labelTemperature->SetLabel( event.GetString() );
	Refresh();
	event.Skip();
}
	
/////////////////////////////////////////////////////////////////// 
// OnVoltageUpdate
//
		
void frmNode::OnVoltageUpdate( wxCommandEvent &event )
{
	m_labelVoltage->SetLabel( event.GetString() );
	Refresh();
	event.Skip();
}

/////////////////////////////////////////////////////////////////// 
// OnWrkThreadMsg
//

void frmNode::OnWrkThreadMsg( wxCommandEvent &event )
{
	wxMessageBox( event.GetString() );
	event.Skip();
}

 

/////////////////////////////////////////////////////////////////// 
// OnLogMsg
//

void frmNode::OnLogMsg( wxCommandEvent &event )
{
  ::wxLogStatus( event.GetString() );
  //::wxLogMessage( event.GetString() );
  //m_labelLogMsg->SetLabel( event.GetString() );
	event.Skip();
}

/////////////////////////////////////////////////////////////////// 
// OnTerminateMsg
//

void frmNode::OnTerminateMsg( wxCommandEvent &event )
{
	m_bRun = false;
	Close();
	event.Skip();
}


/*!
 * wxEVT_MENU_CLOSE event handler for ID_FRMNODE
 */

void frmNode::OnMenuClose( wxMenuEvent& event )
{
	m_bRun = false;
	event.Skip();
}


/*!
 * wxEVT_DESTROY event handler for ID_FRMNODE
 */

void frmNode::OnDestroy( wxWindowDestroyEvent& event )
{
	m_bRun = false;
  wxMilliSleep( 500 );
	event.Skip();
}

 
//////////////////////////////////////////////////////////////////////////////
// wxEVT_CLOSE_WINDOW event handler for ID_FRMNODE
//

void frmNode::OnCloseWindow( wxCloseEvent& event )
{

  event.Skip();
}


//////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ON
//

void frmNode::OnButtonOnClick( wxCommandEvent& event )
{

  event.Skip();
}




//////////////////////////////////////////////////////////////////////////////
 // wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OFF
 //

void frmNode::OnButtonOffClick( wxCommandEvent& event )
{
  //wxCommandEvent eventS1( wxS1_EVENT, 10007 );
	//eventS1.SetInt( LAMP_SEAGREEN );
  //GetEventHandler()->ProcessEvent( eventS1 );
	//setBitmap( m_Bitmap_S1, LAMP_SEAGREEN );
	m_Bitmap_S1->SetBitmap( GetBitmapResource(wxT("button-seagreen_benji_pa_01.png") ) );
  event.Skip();
}

