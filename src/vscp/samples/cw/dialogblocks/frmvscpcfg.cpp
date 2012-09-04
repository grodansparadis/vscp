/////////////////////////////////////////////////////////////////////////////
// Name:        frmvscpcfg.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     24/02/2007 18:13:36
// RCS-ID:      
// Copyright:   (c) 2007 Ake Hedman, eurosource, <ake@eurosource.se>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmvscpcfg.h"
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

#include "frmvscpcfg.h"

////@begin XPM images
////@end XPM images

/*!
 * frmVSCPCfg type definition
 */

IMPLEMENT_CLASS( frmVSCPCfg, wxFrame )

/*!
 * frmVSCPCfg event table definition
 */

BEGIN_EVENT_TABLE( frmVSCPCfg, wxFrame )

////@begin frmVSCPCfg event table entries
////@end frmVSCPCfg event table entries

END_EVENT_TABLE()

/*!
 * frmVSCPCfg constructors
 */

frmVSCPCfg::frmVSCPCfg()
{
    Init();
}

frmVSCPCfg::frmVSCPCfg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

/*!
 * frmVSCPCfg creator
 */

bool frmVSCPCfg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin frmVSCPCfg creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end frmVSCPCfg creation
    return true;
}

/*!
 * frmVSCPCfg destructor
 */

frmVSCPCfg::~frmVSCPCfg()
{
////@begin frmVSCPCfg destruction
////@end frmVSCPCfg destruction
}

/*!
 * Member initialisation 
 */

void frmVSCPCfg::Init()
{
////@begin frmVSCPCfg member initialisation
////@end frmVSCPCfg member initialisation
}
/*!
 * Control creation for frmVSCPCfg
 */

void frmVSCPCfg::CreateControls()
{    
////@begin frmVSCPCfg content construction
    frmVSCPCfg* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxGrid* itemGrid3 = new wxGrid( itemFrame1, ID_GRID1, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemGrid3->SetDefaultColSize(50);
    itemGrid3->SetDefaultRowSize(25);
    itemGrid3->SetColLabelSize(25);
    itemGrid3->SetRowLabelSize(50);
    itemGrid3->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemBoxSizer2->Add(itemGrid3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, 0, 0);

////@end frmVSCPCfg content construction
}

/*!
 * Should we show tooltips?
 */

bool frmVSCPCfg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap frmVSCPCfg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin frmVSCPCfg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end frmVSCPCfg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon frmVSCPCfg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin frmVSCPCfg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end frmVSCPCfg icon retrieval
}
