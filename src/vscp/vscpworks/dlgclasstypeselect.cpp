/////////////////////////////////////////////////////////////////////////////
//  Name:        dlgclasstypeselect.cpp
//  Purpose:     
//  Author:      Ake Hedman
//  Modified by: 
//  Created:     Sat 30 Jun 2007 14:08:14 CEST
//  RCS-ID:      
//  Copyright:   (C) 2007-2018                       
//  Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//  Licence:     
//  This program is free software; you can redistribute it and/or  
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version
//  2 of the License, or (at your option) any later version.
// 
//  This file is part of the VSCP (http://www.vscp.org) 
// 
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this file see the file COPYING.  If not, write to
//  the Free Software Foundation, 59 Temple Place - Suite 330,
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
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include "dlgclasstypeselect.h"


extern VSCPInformation g_vscpinfo;


/*
 * dlgClassTypeSelect type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgClassTypeSelect, wxDialog )


/*
 * dlgClassTypeSelect event table definition
 */

BEGIN_EVENT_TABLE( dlgClassTypeSelect, wxDialog )

    EVT_COMBOBOX( ID_COMBOBOX, dlgClassTypeSelect::OnComboboxClassSelected )
    EVT_COMBOBOX( ID_COMBOBOX1, dlgClassTypeSelect::OnComboboxTypeSelected )
    EVT_BUTTON( wxID_OK, dlgClassTypeSelect::OnButtonOKClick )
    EVT_BUTTON( wxID_CANCEL, dlgClassTypeSelect::OnButtonCANCELClick )

END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
// Ctor
//

dlgClassTypeSelect::dlgClassTypeSelect()
{
    Init();
}

dlgClassTypeSelect::dlgClassTypeSelect( wxWindow* parent, wxWindowID id, 
                                            const wxString& caption, 
                                            const wxPoint& pos, 
                                            const wxSize& size, 
                                            long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


///////////////////////////////////////////////////////////////////////////////
// Ctor
//

bool dlgClassTypeSelect::Create( wxWindow* parent, 
                                    wxWindowID id, const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Dtor
//

dlgClassTypeSelect::~dlgClassTypeSelect()
{

}


///////////////////////////////////////////////////////////////////////////////
// Init
//

void dlgClassTypeSelect::Init()
{
    m_ctrlComboType = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// CreateControls
//

void dlgClassTypeSelect::CreateControls()
{    
    wxBusyCursor wait;

    dlgClassTypeSelect* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, 
                                                            wxID_STATIC, 
                                                            _("VSCP Class"), 
                                                            wxDefaultPosition, 
                                                            wxDefaultSize, 
                                                            0 );
    itemStaticText3->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemComboBox4Strings;
    m_ctrlComboClass = new wxComboBox( itemDialog1, 
                                            ID_COMBOBOX, 
                                            wxEmptyString, 
                                            wxDefaultPosition, 
                                            wxDefaultSize, 
                                            itemComboBox4Strings, 
                                            wxCB_DROPDOWN );
    itemBoxSizer2->Add(m_ctrlComboClass, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, 
                                                        wxID_STATIC, 
                                                        _("VSCP Type"), 
                                                        wxDefaultPosition, 
                                                        wxDefaultSize, 
                                                        0 );
    itemStaticText5->SetFont( wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlComboTypeStrings;
    m_ctrlComboType = new wxComboBox( itemDialog1, 
                                            ID_COMBOBOX1, 
                                            wxEmptyString, 
                                            wxDefaultPosition, 
                                            wxDefaultSize, 
                                            m_ctrlComboTypeStrings, 
                                            wxCB_DROPDOWN );
    itemBoxSizer2->Add(m_ctrlComboType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticTextInfo = new wxStaticText( itemDialog1, 
                    wxID_STATIC, 
                    _("Select the 'general' type to have a wildcard for a class."), 
                    wxDefaultPosition, 
                    wxDefaultSize, 
                    0 );
    itemBoxSizer2->Add(itemStaticTextInfo, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    fillClassCombo();

    // Select first item
    m_ctrlComboClass->SetSelection( 0 );

    fillTypeCombo( 0 );
}


///////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool dlgClassTypeSelect::ShowToolTips()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// GetBitmapResource
//

wxBitmap dlgClassTypeSelect::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

///////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon dlgClassTypeSelect::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonOKClick
//

void dlgClassTypeSelect::OnButtonOKClick( wxCommandEvent& event )
{

    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonCANCELClick
//

void dlgClassTypeSelect::OnButtonCANCELClick( wxCommandEvent& event )
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnComboboxClassSelected
//

void dlgClassTypeSelect::OnComboboxClassSelected( wxCommandEvent& event )
{
    int idx = m_ctrlComboClass->GetSelection();
    if ( wxNOT_FOUND != idx ) {
        fillTypeCombo( (unsigned long) m_ctrlComboClass->GetClientData( idx ) );
    }

    event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnComboboxTypeSelected
//

void dlgClassTypeSelect::OnComboboxTypeSelected( wxCommandEvent& event )
{

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillClassCombo
//

void dlgClassTypeSelect::fillClassCombo( void )
{
    // Clear the combo
    g_vscpinfo.fillClassDescriptions( (wxControlWithItems *)m_ctrlComboClass, WITH_DECIMAL_SUFFIX );
   
    // Select first item
    m_ctrlComboClass->SetSelection( 0 );
  
    // Fill the type combo to 
    fillTypeCombo( (unsigned long)m_ctrlComboClass->GetClientData( 0 ) );
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillTypeCombo
//

void dlgClassTypeSelect::fillTypeCombo( unsigned int vscp_class )
{
    // Clear the combo
    g_vscpinfo.fillTypeDescriptions( (wxControlWithItems *)m_ctrlComboType, 
                                        vscp_class,
                                        WITH_DECIMAL_SUFFIX );

    // Select first item
    m_ctrlComboType->SetSelection( 0 );
  
}