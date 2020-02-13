/////////////////////////////////////////////////////////////////////////////
// Name:    		dlgselectunit.h
// Purpose:     
// Author:     	Ake Hedman
// Modified by: 
// Created:     Sat 28 Apr 2007 14:19:44 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource <ake@eurosource.se>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (https://www.vscp.org) 
//
// Copyright (C) 2000-2009 Ake Hedman, eurosource, <ake@eurosource.se>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
// 
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
// 
// Alternative licenses for VSCP & Friends may be arranged by contacting 
// eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgselectunit.h"
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

#include "dlgnodeinfo.h"
#include "dlgselectunit.h"

extern _nodeinfo g_nodes[];
extern int g_cntNodes;

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
 * dlgSelectUnit type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgSelectUnit, wxDialog )

/*!
 * dlgSelectUnit event table definition
 */

BEGIN_EVENT_TABLE( dlgSelectUnit, wxDialog )

////@begin dlgSelectUnit event table entries
  EVT_LISTBOX( ID_LISTBOX_UNITS, dlgSelectUnit::OnListboxUnitsSelected )

  EVT_BUTTON( ID_BUTTON_NEW, dlgSelectUnit::OnButtonNewClick )

  EVT_BUTTON( ID_BUTTON_EDIT, dlgSelectUnit::OnButtonEditClick )

  EVT_BUTTON( ID_BUTTON_DELETE, dlgSelectUnit::OnButtonDeleteClick )

  EVT_BUTTON( wxID_OK, dlgSelectUnit::OnOkClick )

////@end dlgSelectUnit event table entries

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////////
//  dlgSelectUnit constructors
//

dlgSelectUnit::dlgSelectUnit()
{
    Init();
}

dlgSelectUnit::dlgSelectUnit( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

////////////////////////////////////////////////////////////////////////////////////
// Dialog creator
//

bool dlgSelectUnit::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgSelectUnit creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("wxwin32x321.xpm")));
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
////@end dlgSelectUnit creation

    // Fill the dialog
    m_ctrlListBoxNodes->Clear();
    for ( int i=0; i<g_cntNodes; i++ ) {
        m_ctrlListBoxNodes->Append( g_nodes[ i ].m_strRealname, (void *)i );
    }
    
    if ( !g_cntNodes ) {
        m_selectedIndex = -1; // No selected node
    }
    else {
        m_selectedIndex = 
            (long)m_ctrlListBoxNodes->GetClientData( 0 );
        m_ctrlListBoxNodes->SetSelection( 0 );              
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// dlgSelectUnit destructor
//

dlgSelectUnit::~dlgSelectUnit()
{
////@begin dlgSelectUnit destruction
////@end dlgSelectUnit destruction
}

////////////////////////////////////////////////////////////////////////////////////
// Member initialisation 
//

void dlgSelectUnit::Init()
{
////@begin dlgSelectUnit member initialisation
  m_ctrlListBoxNodes = NULL;
////@end dlgSelectUnit member initialisation
}

////////////////////////////////////////////////////////////////////////////////////
// Control creation for Dialog
//

void dlgSelectUnit::CreateControls()
{    
////@begin dlgSelectUnit content construction
  dlgSelectUnit* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxArrayString m_ctrlListBoxNodesStrings;
  m_ctrlListBoxNodes = new wxListBox;
  m_ctrlListBoxNodes->Create( itemDialog1, ID_LISTBOX_UNITS, wxDefaultPosition, wxSize(300, 200), m_ctrlListBoxNodesStrings, wxLB_SINGLE|wxLB_SORT );
  m_ctrlListBoxNodes->SetStringSelection(_("0"));
  itemBoxSizer3->Add(m_ctrlListBoxNodes, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton6 = new wxButton;
  itemButton6->Create( itemDialog1, ID_BUTTON_NEW, _("New..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton7 = new wxButton;
  itemButton7->Create( itemDialog1, ID_BUTTON_EDIT, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton8 = new wxButton;
  itemButton8->Create( itemDialog1, ID_BUTTON_DELETE, _("Delete..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  itemBoxSizer5->Add(5, 20, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton10 = new wxButton;
  itemButton10->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton11 = new wxButton;
  itemButton11->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer5->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

////@end dlgSelectUnit content construction
}

////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgSelectUnit::ShowToolTips()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgSelectUnit::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dlgSelectUnit bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgSelectUnit bitmap retrieval
}

////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgSelectUnit::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dlgSelectUnit icon retrieval
  wxUnusedVar(name);
  if (name == _T("wxwin32x321.xpm"))
  {
    wxIcon icon(wxwin32x32_xpm);
    return icon;
  }
  return wxNullIcon;
////@end dlgSelectUnit icon retrieval
}

////////////////////////////////////////////////////////////////////////////////////
// readStringValue

unsigned char dlgSelectUnit::readStringValue( wxString& str )
{
  long val;
  
  str.MakeLower();
  if ( wxNOT_FOUND != str.Find(_("0x")) ) {
    str.ToLong( &val, 16 );
  }
  else {
    str.ToLong(&val);
  }
  
  return (unsigned char)val;
}      

////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_NEW
//

void dlgSelectUnit::OnButtonNewClick( wxCommandEvent& event )
{
  wxString str;
  
  if ( g_cntNodes < MAX_NUMBER_OF_NODES ) {
    
    dlgNodeInfo dlg( this );
    
    if ( wxID_OK == dlg.ShowModal() ) {
    
        int idx = g_cntNodes;
        
        // Write data back
        str = dlg.m_ctrlEditGUID15->GetValue();
        g_nodes[ idx ].m_GUID[15] = readStringValue( str );
        str = dlg.m_ctrlEditGUID14->GetValue();
        g_nodes[ idx ].m_GUID[14] = readStringValue( str ) ;
        str = (dlg.m_ctrlEditGUID13->GetValue());
        g_nodes[ idx ].m_GUID[13] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID12->GetValue());
        g_nodes[ idx ].m_GUID[12] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID11->GetValue());
        g_nodes[ idx ].m_GUID[11] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID10->GetValue());
        g_nodes[ idx ].m_GUID[10] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID9->GetValue());
        g_nodes[ idx ].m_GUID[9] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID8->GetValue());
        g_nodes[ idx ].m_GUID[8] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID7->GetValue());
        g_nodes[ idx ].m_GUID[7] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID6->GetValue());
        g_nodes[ idx ].m_GUID[6] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID5->GetValue());
        g_nodes[ idx ].m_GUID[5] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID4->GetValue());
        g_nodes[ idx ].m_GUID[4] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID3->GetValue());
        g_nodes[ idx ].m_GUID[3] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID2->GetValue());
        g_nodes[ idx ].m_GUID[2] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID1->GetValue());
        g_nodes[ idx ].m_GUID[1] = readStringValue( str );
        str = (dlg.m_ctrlEditGUID0->GetValue());
        g_nodes[ idx ].m_GUID[0] = readStringValue( str );
        
        g_nodes[ idx ].m_strRealname = dlg.m_ctrlEditRealName->GetValue();
        g_nodes[ idx ].m_strRealname = dlg.m_ctrlEditRealName->GetValue();
        g_nodes[ idx ].m_strHostname = dlg.m_ctrlEditHostname->GetValue();
      
        str = (dlg.m_ctrlEditZone->GetValue());
        g_nodes[ idx ].m_zone = readStringValue( str );
    
        m_ctrlListBoxNodes->Append( g_nodes[ idx ].m_strRealname, (void *)g_cntNodes );
        g_cntNodes++;
    }
  }
  else {
      wxMessageBox(_("Maximum number of nodes already defines!"));
  }

  event.Skip();
}


////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
//

void dlgSelectUnit::OnButtonEditClick( wxCommandEvent& event )
{
  long idx;
  long pos;
  wxString str;
  
  if ( wxNOT_FOUND != ( pos = m_ctrlListBoxNodes->GetSelection() ) ) {
    
    dlgNodeInfo dlg( this );
    
    idx = (long)m_ctrlListBoxNodes->GetClientData( pos );
      
    // write data
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[15] );
    dlg.m_ctrlEditGUID15->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[14] );
    dlg.m_ctrlEditGUID14->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[13] );
    dlg.m_ctrlEditGUID13->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[12] );
    dlg.m_ctrlEditGUID12->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[11] );
    dlg.m_ctrlEditGUID11->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[10] );
    dlg.m_ctrlEditGUID10->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[9] );
    dlg.m_ctrlEditGUID9->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[8] );
    dlg.m_ctrlEditGUID8->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[7] );
    dlg.m_ctrlEditGUID7->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[6] );
    dlg.m_ctrlEditGUID6->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[5] );
    dlg.m_ctrlEditGUID5->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[4] );
    dlg.m_ctrlEditGUID4->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[3] );
    dlg.m_ctrlEditGUID3->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[2] );
    dlg.m_ctrlEditGUID2->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[1] );
    dlg.m_ctrlEditGUID1->SetValue( str );
        
    str.Printf(_("%d"), g_nodes[ idx ].m_GUID[0] );
    dlg.m_ctrlEditGUID0->SetValue( str );
        
    dlg.m_ctrlEditRealName->SetValue( g_nodes[ idx ].m_strRealname );
    
    dlg.m_ctrlEditHostname->SetValue( g_nodes[ idx ].m_strHostname );
    
    str.Printf(_("%d"), g_nodes[ idx ].m_zone );
    dlg.m_ctrlEditZone->SetValue( str );
    
    if ( wxID_OK == dlg.ShowModal() ) {
    
      // Write data back
      str = dlg.m_ctrlEditGUID15->GetValue();
      g_nodes[ idx ].m_GUID[15] = readStringValue( str );
      str = dlg.m_ctrlEditGUID14->GetValue();
      g_nodes[ idx ].m_GUID[14] = readStringValue( str ) ;
      str = (dlg.m_ctrlEditGUID13->GetValue());
      g_nodes[ idx ].m_GUID[13] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID12->GetValue());
      g_nodes[ idx ].m_GUID[12] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID11->GetValue());
      g_nodes[ idx ].m_GUID[11] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID10->GetValue());
      g_nodes[ idx ].m_GUID[10] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID9->GetValue());
      g_nodes[ idx ].m_GUID[9] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID8->GetValue());
      g_nodes[ idx ].m_GUID[8] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID7->GetValue());
      g_nodes[ idx ].m_GUID[7] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID6->GetValue());
      g_nodes[ idx ].m_GUID[6] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID5->GetValue());
      g_nodes[ idx ].m_GUID[5] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID4->GetValue());
      g_nodes[ idx ].m_GUID[4] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID3->GetValue());
      g_nodes[ idx ].m_GUID[3] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID2->GetValue());
      g_nodes[ idx ].m_GUID[2] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID1->GetValue());
      g_nodes[ idx ].m_GUID[1] = readStringValue( str );
      str = (dlg.m_ctrlEditGUID0->GetValue());
      g_nodes[ idx ].m_GUID[0] = readStringValue( str );
        
      g_nodes[ idx ].m_strRealname = dlg.m_ctrlEditRealName->GetValue();
      g_nodes[ idx ].m_strHostname = dlg.m_ctrlEditHostname->GetValue();
      
      str = (dlg.m_ctrlEditZone->GetValue());
      g_nodes[ idx ].m_zone = readStringValue( str );
        
      m_ctrlListBoxNodes->SetString( pos, g_nodes[ idx ].m_strRealname );
      
    }
  }
  else {
    wxMessageBox(_("Please select one entry!"));
  }
  
  event.Skip();
}


////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_DELETE
//

void dlgSelectUnit::OnButtonDeleteClick( wxCommandEvent& event )
{
  int pos;
  
  if ( wxNOT_FOUND != ( pos = m_ctrlListBoxNodes->GetSelection() ) ) {
    if ( wxYES == wxMessageBox(
                  _("Are you shure that the Auto-Self node should be removed?"),
                  _("Delete Auto-Self node."),
                  wxYES_NO | wxICON_EXCLAMATION ) ) {
        // Delete the item              
        m_ctrlListBoxNodes->Delete( pos );
    }
  }
  event.Skip();
}


////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
//

void dlgSelectUnit::OnOkClick( wxCommandEvent& event )
{
  int pos;
  
  if ( wxNOT_FOUND == ( pos = m_ctrlListBoxNodes->GetSelection() ) ) {
    wxMessageBox(_("Need to make selection otherwise press Cancel to leave dialog."),
                  _("Auto-Self Control"),
                  wxOK | wxICON_EXCLAMATION );
    return;  
  }
  
  event.Skip();
  
}


////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_UNITS
//

void dlgSelectUnit::OnListboxUnitsSelected( wxCommandEvent& event )
{
  // Save Index
  m_selectedIndex = 
    (long)m_ctrlListBoxNodes->GetClientData( m_ctrlListBoxNodes->GetSelection() );
  event.Skip();
}

