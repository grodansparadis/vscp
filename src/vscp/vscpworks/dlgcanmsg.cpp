/////////////////////////////////////////////////////////////////////////////
// Name:        dlgcanmsg.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     25/02/2007 22:47:00
// RCS-ID:      
// Copyright:   (C) 2007-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgcanmsg.h"
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

#include "dlgcanmsg.h"

////@begin XPM images
#include "open.xpm"
#include "cut.xpm"
#include "NEW.xpm"
#include "filesaveas.xpm"
////@end XPM images

/*!
 * DlgCANMsg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DlgCANMsg, wxDialog )

/*!
 * DlgCANMsg event table definition
 */

BEGIN_EVENT_TABLE( DlgCANMsg, wxDialog )

////@begin DlgCANMsg event table entries
    EVT_INIT_DIALOG( DlgCANMsg::OnInitDialog )
    EVT_CLOSE( DlgCANMsg::OnCloseWindow )

////@end DlgCANMsg event table entries

END_EVENT_TABLE()

/*!
 * DlgCANMsg constructors
 */

DlgCANMsg::DlgCANMsg()
{
    Init();
}

DlgCANMsg::DlgCANMsg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * DlgCANMsg creator
 */

bool DlgCANMsg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DlgCANMsg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end DlgCANMsg creation
    return true;
}

/*!
 * DlgCANMsg destructor
 */

DlgCANMsg::~DlgCANMsg()
{
////@begin DlgCANMsg destruction
////@end DlgCANMsg destruction
}

/*!
 * Member initialisation 
 */

void DlgCANMsg::Init()
{
////@begin DlgCANMsg member initialisation
    m_ctrlGrid = NULL;
    m_btnEdit = NULL;
    m_btnDelete = NULL;
    m_btnLoad = NULL;
    m_btnSave = NULL;
    m_ctrlGridSend = NULL;
    m_btnSend = NULL;
    m_btnActivate = NULL;
    m_btnClear = NULL;
////@end DlgCANMsg member initialisation
}
/*!
 * Control creation for DlgCANMsg
 */

void DlgCANMsg::CreateControls()
{    
////@begin DlgCANMsg content construction
    DlgCANMsg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 2);

    m_ctrlGrid = new wxGrid( itemDialog1, ID_GRID_LOG, wxDefaultPosition, wxSize(800, 300), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_ctrlGrid->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
    m_ctrlGrid->SetDefaultColSize(100);
    m_ctrlGrid->SetDefaultRowSize(10);
    m_ctrlGrid->SetColLabelSize(10);
    m_ctrlGrid->SetRowLabelSize(30);
    m_ctrlGrid->CreateGrid(10000, 8, wxGrid::wxGridSelectRows);
    itemBoxSizer3->Add(m_ctrlGrid, 0, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 0);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_BOTTOM|wxALL, 5);

    wxBitmap m_btnEditBitmap(itemDialog1->GetBitmapResource(wxT("open.xpm")));
    m_btnEdit = new wxBitmapButton;
    m_btnEdit->Create( itemDialog1, ID_BITMAPBUTTON_EDIT, m_btnEditBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    if (ShowToolTips())
        m_btnEdit->SetToolTip(_("Edit line content"));
    itemBoxSizer7->Add(m_btnEdit, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBitmap m_btnDeleteBitmap(itemDialog1->GetBitmapResource(wxT("cut.xpm")));
    m_btnDelete = new wxBitmapButton;
    m_btnDelete->Create( itemDialog1, ID_BITMAPBUTTON_DELETE, m_btnDeleteBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    if (ShowToolTips())
        m_btnDelete->SetToolTip(_("Delete line"));
    itemBoxSizer7->Add(m_btnDelete, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer7->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxBitmap m_btnLoadBitmap(itemDialog1->GetBitmapResource(wxT("NEW.BMP")));
    m_btnLoad = new wxBitmapButton;
    m_btnLoad->Create( itemDialog1, ID_BITMAPBUTTON_LOAD, m_btnLoadBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    if (ShowToolTips())
        m_btnLoad->SetToolTip(_("Load set"));
    itemBoxSizer7->Add(m_btnLoad, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBitmap m_btnSaveBitmap(itemDialog1->GetBitmapResource(wxT("filesaveas.xpm")));
    m_btnSave = new wxBitmapButton;
    m_btnSave->Create( itemDialog1, ID_BITMAPBUTTON_SAVE, m_btnSaveBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    if (ShowToolTips())
        m_btnSave->SetToolTip(_("Save set"));
    itemBoxSizer7->Add(m_btnSave, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer13, 0, wxGROW|wxALL, 5);

    m_ctrlGridSend = new wxGrid( itemDialog1, ID_GRID_SEND, wxDefaultPosition, wxSize(750, 100), wxHSCROLL|wxVSCROLL );
    m_ctrlGridSend->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, false, wxT("")));
    m_ctrlGridSend->SetDefaultColSize(100);
    m_ctrlGridSend->SetDefaultRowSize(10);
    m_ctrlGridSend->SetColLabelSize(10);
    m_ctrlGridSend->SetRowLabelSize(30);
    m_ctrlGridSend->CreateGrid(5, 5, wxGrid::wxGridSelectRows);
    itemBoxSizer13->Add(m_ctrlGridSend, 0, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer15, 0, wxALIGN_BOTTOM|wxALL, 5);

    wxBitmap m_btnSendBitmap(wxNullBitmap);
    m_btnSend = new wxBitmapButton;
    m_btnSend->Create( itemDialog1, ID_BITMAPBUTTON_SEND, m_btnSendBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemBoxSizer15->Add(m_btnSend, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxBitmap m_btnActivateBitmap(wxNullBitmap);
    m_btnActivate = new wxBitmapButton;
    m_btnActivate->Create( itemDialog1, ID_BITMAPBUTTON_ACTIVATE, m_btnActivateBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemBoxSizer15->Add(m_btnActivate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBitmap m_btnClearBitmap(wxNullBitmap);
    m_btnClear = new wxBitmapButton;
    m_btnClear->Create( itemDialog1, ID_BITMAPBUTTON_CLEAR, m_btnClearBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBU_EXACTFIT );
    itemBoxSizer15->Add(m_btnClear, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end DlgCANMsg content construction
}

/*!
 * wxEVT_INIT_DIALOG event handler for ID_DIALOG_CAN_MESSAGE
 */

void DlgCANMsg::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_CAN_MESSAGE in DlgCANMsg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_CAN_MESSAGE in DlgCANMsg. 
}

/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_CAN_MESSAGE
 */

void DlgCANMsg::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_CAN_MESSAGE in DlgCANMsg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_CAN_MESSAGE in DlgCANMsg. 
}

/*!
 * Should we show tooltips?
 */

bool DlgCANMsg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap DlgCANMsg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DlgCANMsg bitmap retrieval
    wxUnusedVar(name);
    if (name == _T("open.xpm"))
    {
        wxBitmap bitmap(open_xpm);
        return bitmap;
    }
    else if (name == _T("cut.xpm"))
    {
        wxBitmap bitmap(cut_xpm);
        return bitmap;
    }
    else if (name == _T("NEW.BMP"))
    {
        wxBitmap bitmap(NEW_xpm);
        return bitmap;
    }
    else if (name == _T("filesaveas.xpm"))
    {
        wxBitmap bitmap(filesaveas_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end DlgCANMsg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DlgCANMsg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DlgCANMsg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end DlgCANMsg icon retrieval
}
