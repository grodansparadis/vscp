/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnewcanalsession.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 12:10:45 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2017 
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
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgnewcanalsession.h"
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

#include "dlgnewcanalsession.h"

////@begin XPM images
////@end XPM images


/*!
 * dlgNewCanalSession type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgNewCanalSession, wxDialog )


/*!
 * dlgNewCanalSession event table definition
 */

BEGIN_EVENT_TABLE( dlgNewCanalSession, wxDialog )

////@begin dlgNewCanalSession event table entries
  EVT_INIT_DIALOG( dlgNewCanalSession::OnInitDialog )
  EVT_CLOSE( dlgNewCanalSession::OnCloseWindow )

  EVT_BUTTON( wxID_OK, dlgNewCanalSession::OnOKClick )

  EVT_BUTTON( wxID_CANCEL, dlgNewCanalSession::OnCANCELClick )

  EVT_BUTTON( ID_BUTTON_ADD, dlgNewCanalSession::OnButtonAddClick )

  EVT_BUTTON( ID_BUTTON_EDIT, dlgNewCanalSession::OnButtonEditClick )

  EVT_BUTTON( ID_BUTTON_REMOVE, dlgNewCanalSession::OnButtonRemoveClick )

////@end dlgNewCanalSession event table entries

END_EVENT_TABLE()


/*!
 * dlgNewCanalSession constructors
 */

dlgNewCanalSession::dlgNewCanalSession()
{
  Init();
}

dlgNewCanalSession::dlgNewCanalSession( wxWindow* parent, 
                                            wxWindowID id, 
                                            const wxString& caption, 
                                            const wxPoint& pos, 
                                            const wxSize& size, 
                                            long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * DlgNewCanalSession creator
 */

bool dlgNewCanalSession::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
////@begin dlgNewCanalSession creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
////@end dlgNewCanalSession creation
  return true;
}


/*!
 * dlgNewCanalSession destructor
 */

dlgNewCanalSession::~dlgNewCanalSession()
{
////@begin dlgNewCanalSession destruction
////@end dlgNewCanalSession destruction
}


/*!
 * Member initialisation
 */

void dlgNewCanalSession::Init()
{
////@begin dlgNewCanalSession member initialisation
  m_ctrlListInterfaces = NULL;
////@end dlgNewCanalSession member initialisation
}


/*!
 * Control creation for DlgNewCanalSession
 */

void dlgNewCanalSession::CreateControls()
{    
////@begin dlgNewCanalSession content construction
  dlgNewCanalSession* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxArrayString m_ctrlListInterfacesStrings;
  m_ctrlListInterfacesStrings.Add(_("Direct connection to local VSCP daemon"));
  m_ctrlListInterfacesStrings.Add(_("TCP/IP connection to VSCP daemon"));
  m_ctrlListInterfaces = new wxListBox;
  m_ctrlListInterfaces->Create( itemDialog1, ID_LISTBOX_INTERFACES, wxDefaultPosition, wxSize(300, -1), m_ctrlListInterfacesStrings, wxLB_SINGLE );
  itemBoxSizer2->Add(m_ctrlListInterfaces, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton5 = new wxButton;
  itemButton5->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton5->SetDefault();
  if (ShowToolTips())
    itemButton5->SetToolTip(_("Use selected interface"));
  itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton6 = new wxButton;
  itemButton6->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  if (ShowToolTips())
    itemButton6->SetToolTip(_("Go back without selecting any interface"));
  itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton;
  itemButton8->Create( itemDialog1, ID_BUTTON_ADD, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
  if (ShowToolTips())
    itemButton8->SetToolTip(_("Add new interface"));
  itemBoxSizer4->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton9 = new wxButton;
  itemButton9->Create( itemDialog1, ID_BUTTON_EDIT, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton9->SetDefault();
  if (ShowToolTips())
    itemButton9->SetToolTip(_("Edit selected interface"));
  itemBoxSizer4->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton10 = new wxButton;
  itemButton10->Create( itemDialog1, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
  if (ShowToolTips())
    itemButton10->SetToolTip(_("Remove selected interface"));
  itemBoxSizer4->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

////@end dlgNewCanalSession content construction
}


/*!
 * wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_CANAL_SESSION
 */

void dlgNewCanalSession::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_CANAL_SESSION in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_CANAL_SESSION in DlgNewCanalSession. 
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_NEW_CANAL_SESSION
 */

void dlgNewCanalSession::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_NEW_CANAL_SESSION in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_NEW_CANAL_SESSION in DlgNewCanalSession. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void dlgNewCanalSession::OnOKClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in DlgNewCanalSession. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void dlgNewCanalSession::OnCANCELClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in DlgNewCanalSession. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD
 */

void dlgNewCanalSession::OnButtonAddClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD in DlgNewCanalSession. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
 */

void dlgNewCanalSession::OnButtonEditClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT in DlgNewCanalSession. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE
 */

void dlgNewCanalSession::OnButtonRemoveClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE in DlgNewCanalSession.
  // Before editing this code, remove the block markers.
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE in DlgNewCanalSession. 
}


/*!
 * Should we show tooltips?
 */

bool dlgNewCanalSession::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgNewCanalSession::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgNewCanalSession bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgNewCanalSession bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgNewCanalSession::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgNewCanalSession icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
////@end dlgNewCanalSession icon retrieval
}
