/////////////////////////////////////////////////////////////////////////////
// Name:        frmmessagelog.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Wed 25 Apr 2007 11:54:26 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmessagelog.h"
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

#include "frmmessagelog.h"

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

////@end XPM images

/*!
 * frmMessageLog type definition
 */

IMPLEMENT_CLASS( frmMessageLog, wxMDIChildFrame )

/*!
 * frmMessageLog event table definition
 */

BEGIN_EVENT_TABLE( frmMessageLog, wxMDIChildFrame )

////@begin frmMessageLog event table entries
////@end frmMessageLog event table entries

END_EVENT_TABLE()

/*!
 * frmMessageLog constructors
 */

frmMessageLog::frmMessageLog()
{
    Init();
}

frmMessageLog::frmMessageLog( wxMDIParentFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

/*!
 * frmMessageLog creator
 */

bool frmMessageLog::Create( wxMDIParentFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin frmMessageLog creation
  wxMDIChildFrame::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("wxwin32x321.xpm")));
  Centre();
////@end frmMessageLog creation
    return true;
}

/*!
 * frmMessageLog destructor
 */

frmMessageLog::~frmMessageLog()
{
////@begin frmMessageLog destruction
////@end frmMessageLog destruction
}

/*!
 * Member initialisation 
 */

void frmMessageLog::Init()
{
////@begin frmMessageLog member initialisation
////@end frmMessageLog member initialisation
}

/*!
 * Control creation for frmMessageLog
 */

void frmMessageLog::CreateControls()
{    
////@begin frmMessageLog content construction
  frmMessageLog* itemMDIChildFrame1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemMDIChildFrame1->SetSizer(itemBoxSizer2);

  wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 2, 0, 0);
  itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

  wxArrayString itemListBox4Strings;
  wxListBox* itemListBox4 = new wxListBox;
  itemListBox4->Create( itemMDIChildFrame1, ID_GRID_LOG, wxDefaultPosition, wxSize(585, 260), itemListBox4Strings, wxLB_SINGLE|wxSUNKEN_BORDER|wxFULL_REPAINT_ON_RESIZE );
  itemFlexGridSizer3->Add(itemListBox4, 1, wxGROW|wxGROW|wxRIGHT|wxTOP|wxBOTTOM, 0);

  wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
  itemBoxSizer2->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton6 = new wxBitmapButton;
  itemBitmapButton6->Create( itemMDIChildFrame1, ID_BITMAPBUTTON_CLEAR_LIST, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
  if (ShowToolTips())
    itemBitmapButton6->SetToolTip(_("Rensa lista"));
  itemFlexGridSizer5->Add(itemBitmapButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxBitmapButton* itemBitmapButton7 = new wxBitmapButton;
  itemBitmapButton7->Create( itemMDIChildFrame1, ID_BITMAPBUTTON1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
  itemFlexGridSizer5->Add(itemBitmapButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end frmMessageLog content construction
}

/*!
 * Should we show tooltips?
 */

bool frmMessageLog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap frmMessageLog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin frmMessageLog bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end frmMessageLog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon frmMessageLog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin frmMessageLog icon retrieval
  wxUnusedVar(name);
  if (name == _T("wxwin32x321.xpm"))
  {
    wxIcon icon(wxwin32x32_xpm);
    return icon;
  }
  return wxNullIcon;
////@end frmMessageLog icon retrieval
}
