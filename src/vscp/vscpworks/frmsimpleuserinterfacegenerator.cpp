/////////////////////////////////////////////////////////////////////////////
// Name:        frmsimpleuserinterfacegenerator.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/10/2012 14:35:37
// RCS-ID:      
/// Copyright:   (C) 2012-2018 
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
//  Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmsimpleuserinterfacegenerator.h"
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

#include "frmsimpleuserinterfacegenerator.h"

////@begin XPM images
/* XPM */
static const char *deffile_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 16 32 1",
"= c #97C4E7",
": c #72A8D2",
"1 c #FFFFFF",
"w c #839CB5",
"X c #6B98B8",
". c #5A89A6",
"@ c #3A749C",
", c #D1E5F5",
"< c #67A1CF",
"> c #F1F4F7",
"e c #85A7BC",
"% c #C3DDF1",
"0 c #749BB4",
"2 c #7EA6C0",
"; c #5F9BC8",
"  c None",
"O c #538DB3",
"- c #85BBE2",
"$ c #D6DFE7",
"9 c #EFF6FC",
"o c #6591AE",
"4 c #F7FBFD",
"8 c #FAFCFE",
"6 c #DAEAF7",
"7 c #E9F3FA",
"q c #FDFDFE",
"3 c #E2EFF8",
"# c #8EA9BC",
"& c #B6D5EE",
"* c #A5CCEA",
"5 c #F4F9FD",
"+ c #4581AA",
/* pixels */
"  ..XooO+@#$    ",
"  .%%&*=-;:;>   ",
"  .,,%&*=<1=X>  ",
"  #%%%%&*211=X  ",
"  #3-----<oXoO  ",
"  #3456,%&*=-O  ",
"  #3--------=O  ",
"  #355736,%&*o  ",
"  #3--------&o  ",
"  #38459736,%X  ",
"  #3--------,0  ",
"  #31q84597360  ",
"  #3--------3w  ",
"  #3111q84597e  ",
"  ##########e#  ",
"                "
};

/* XPM */
static const char *open_xpm[] = {
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
static const char *save_xpm[] = {
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
static const char *filesaveas_xpm[] = {
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
static const char *redo_xpm[] = {
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

/* XPM */
static const char *new_xpm[] = {
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
static const char * delete_xpm[] = {
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

////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUserInterfaceGenerator type definition
//

IMPLEMENT_CLASS( SimpleUserInterfaceGenerator, wxFrame )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUserInterfaceGenerator event table definition
//

BEGIN_EVENT_TABLE( SimpleUserInterfaceGenerator, wxFrame )

////@begin SimpleUserInterfaceGenerator event table entries
////@end SimpleUserInterfaceGenerator event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUserInterfaceGenerator constructors
//

SimpleUserInterfaceGenerator::SimpleUserInterfaceGenerator()
{
  Init();
}

SimpleUserInterfaceGenerator::SimpleUserInterfaceGenerator( wxWindow* parent, 
                                                                wxWindowID id, 
                                                                const wxString& caption, 
                                                                const wxPoint& pos, 
                                                                const wxSize& size, 
                                                                long style )
{
  Init();
  Create( parent, id, caption, pos, size, style );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUserInterfaceGenerator creator
//

bool SimpleUserInterfaceGenerator::Create( wxWindow* parent, 
                                                wxWindowID id, 
                                                const wxString& caption, 
                                                const wxPoint& pos, 
                                                const wxSize& size, 
                                                long style )
{
////@begin SimpleUserInterfaceGenerator creation
  wxFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  Centre();
////@end SimpleUserInterfaceGenerator creation
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUserInterfaceGenerator destructor
//

SimpleUserInterfaceGenerator::~SimpleUserInterfaceGenerator()
{
////@begin SimpleUserInterfaceGenerator destruction
////@end SimpleUserInterfaceGenerator destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void SimpleUserInterfaceGenerator::Init()
{
////@begin SimpleUserInterfaceGenerator member initialisation
  m_mdfTree = NULL;
  m_htmlInfoWnd = NULL;
////@end SimpleUserInterfaceGenerator member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for SimpleUserInterfaceGenerator
//

void SimpleUserInterfaceGenerator::CreateControls()
{    
////@begin SimpleUserInterfaceGenerator content construction
  SimpleUserInterfaceGenerator* itemFrame1 = this;

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

  wxPanel* itemPanel8 = new wxPanel;
  itemPanel8->Create( itemFrame1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

  wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
  itemPanel8->SetSizer(itemBoxSizer9);

  wxToolBar* itemToolBar10 = new wxToolBar;
  itemToolBar10->Create( itemPanel8, ID_TOOLBAR3, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL );
  wxBitmap itemtool11Bitmap(itemFrame1->GetBitmapResource(wxT("deffile.xpm")));
  wxBitmap itemtool11BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_NEW, wxEmptyString, itemtool11Bitmap, itemtool11BitmapDisabled, wxITEM_NORMAL, _("Create new MDF file"), wxEmptyString);
  wxBitmap itemtool12Bitmap(itemFrame1->GetBitmapResource(wxT("open.xpm")));
  wxBitmap itemtool12BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_LOAD, wxEmptyString, itemtool12Bitmap, itemtool12BitmapDisabled, wxITEM_NORMAL, _("Load MDF file"), wxEmptyString);
  wxBitmap itemtool13Bitmap(itemFrame1->GetBitmapResource(wxT("save.xpm")));
  wxBitmap itemtool13BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_SAVE, wxEmptyString, itemtool13Bitmap, itemtool13BitmapDisabled, wxITEM_NORMAL, _("Save MDF file"), wxEmptyString);
  wxBitmap itemtool14Bitmap(itemFrame1->GetBitmapResource(wxT("filesaveas.xpm")));
  wxBitmap itemtool14BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_UPLOAD, wxEmptyString, itemtool14Bitmap, itemtool14BitmapDisabled, wxITEM_NORMAL, _("Upload MDF file"), wxEmptyString);
  wxBitmap itemtool15Bitmap(itemFrame1->GetBitmapResource(wxT("redo.xpm")));
  wxBitmap itemtool15BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_DOWNLOAD, wxEmptyString, itemtool15Bitmap, itemtool15BitmapDisabled, wxITEM_NORMAL, _("Upload MDF file"), wxEmptyString);
  itemToolBar10->AddSeparator();
  wxBitmap itemtool17Bitmap(itemFrame1->GetBitmapResource(wxT("New1.xpm")));
  wxBitmap itemtool17BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_ADD_ITEM, wxEmptyString, itemtool17Bitmap, itemtool17BitmapDisabled, wxITEM_NORMAL, _("Add MDF item"), wxEmptyString);
  wxBitmap itemtool18Bitmap(itemFrame1->GetBitmapResource(wxT("delete.xpm")));
  wxBitmap itemtool18BitmapDisabled;
  itemToolBar10->AddTool(ID_TOOL_REMOVE_ITEM, wxEmptyString, itemtool18Bitmap, itemtool18BitmapDisabled, wxITEM_NORMAL, _("Remove MDF item"), wxEmptyString);
  itemToolBar10->Realize();
  itemBoxSizer9->Add(itemToolBar10, 0, wxGROW|wxALL, 5);

  m_mdfTree = new wxTreeCtrl;
  m_mdfTree->Create( itemPanel8, ID_TREECTRL, wxDefaultPosition, wxSize(400, 300), wxTR_HAS_BUTTONS |wxTR_FULL_ROW_HIGHLIGHT|wxTR_LINES_AT_ROOT|wxTR_ROW_LINES|wxTR_SINGLE );
  itemBoxSizer9->Add(m_mdfTree, 0, wxGROW|wxALL, 2);

  m_htmlInfoWnd = new wxHtmlWindow;
  m_htmlInfoWnd->Create( itemPanel8, ID_HTMLWINDOW2, wxDefaultPosition, wxSize(200, 150), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  m_htmlInfoWnd->SetPage(_("<html><h4>No data</h4></html>"));
  itemBoxSizer9->Add(m_htmlInfoWnd, 0, wxGROW|wxALL, 5);

////@end SimpleUserInterfaceGenerator content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool SimpleUserInterfaceGenerator::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap SimpleUserInterfaceGenerator::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin SimpleUserInterfaceGenerator bitmap retrieval
  wxUnusedVar(name);
  if (name == _T("deffile.xpm"))
  {
    wxBitmap bitmap(deffile_xpm);
    return bitmap;
  }
  else if (name == _T("open.xpm"))
  {
    wxBitmap bitmap(open_xpm);
    return bitmap;
  }
  else if (name == _T("save.xpm"))
  {
    wxBitmap bitmap(save_xpm);
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
  return wxNullBitmap;
////@end SimpleUserInterfaceGenerator bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon SimpleUserInterfaceGenerator::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin SimpleUserInterfaceGenerator icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end SimpleUserInterfaceGenerator icon retrieval
}
