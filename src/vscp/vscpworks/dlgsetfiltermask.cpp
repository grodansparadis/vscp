
/////////////////////////////////////////////////////////////////////////////
// Name:        dialogsetfiltermask.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     21/03/2009 00:45:10
// RCS-ID:      
// Copyright:   (C) 2007-2018 
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
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgsetfiltermask.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vscp.h"
#include "vscphelper.h"
#include "vscpeventhelper.h"
#include "dlgsetfiltermask.h"

VSCPInformation geventinfo;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogSetfiltermask type definition
//

IMPLEMENT_DYNAMIC_CLASS( DialogSetfiltermask, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogSetfiltermask event table definition
//

BEGIN_EVENT_TABLE( DialogSetfiltermask, wxDialog )

    EVT_COMBOBOX( ID_COMBOBOX, DialogSetfiltermask::OnComboboxSelected )
    EVT_BUTTON( ID_BUTTON_ADD_EVENT, DialogSetfiltermask::OnButtonAddEventClick )
    EVT_BUTTON( ID_BUTTON_REMOVE_EVENT, DialogSetfiltermask::OnButtonRemoveEventClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogSetfiltermask constructors
//

DialogSetfiltermask::DialogSetfiltermask()
{
    Init();
}

DialogSetfiltermask::DialogSetfiltermask( wxWindow* parent, 
                                                wxWindowID id, 
                                                const wxString& caption, 
                                                const wxPoint& pos, 
                                                const wxSize& size, 
                                                long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogSetfiltermask creator
//

bool DialogSetfiltermask::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->SetSizeHints(this);
    }

    Centre();
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogSetfiltermask destructor
//

DialogSetfiltermask::~DialogSetfiltermask()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DialogSetfiltermask::Init()
{
    m_wxComboClass = NULL;
    m_wxComboType = NULL;
    m_statusText = NULL;
    m_listboxEvents = NULL;

    m_classmask = 0;
    m_classfilter = 0;
    m_typemask = 0;
    m_typefilter = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DialogSetfiltermask
//

void DialogSetfiltermask::CreateControls()
{    
    DialogSetfiltermask* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemDialog1, wxID_STATIC, _("Class"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_wxComboClassStrings;
    m_wxComboClass = new wxComboBox;
    m_wxComboClass->Create( itemDialog1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(400, -1), m_wxComboClassStrings, wxCB_DROPDOWN );
    itemBoxSizer3->Add(m_wxComboClass, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemDialog1, wxID_STATIC, _("Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_wxComboTypeStrings;
    m_wxComboType = new wxComboBox;
    m_wxComboType->Create( itemDialog1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxSize(400, -1), m_wxComboTypeStrings, wxCB_DROPDOWN );
    itemBoxSizer6->Add(m_wxComboType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create( itemDialog1, ID_BUTTON_ADD_EVENT, _("Add event"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, ID_BUTTON_REMOVE_EVENT, _("Remove selected events"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_statusText = new wxStaticText;
    m_statusText->Create( itemDialog1, wxID_STATIC, _("---"), wxDefaultPosition, wxSize(400, -1), 0 );
    itemBoxSizer12->Add(m_statusText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_listboxEventsStrings;
    m_listboxEvents = new wxListBox;
    m_listboxEvents->Create( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(400, 300), m_listboxEventsStrings, wxLB_SINGLE );
    itemBoxSizer14->Add(m_listboxEvents, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton;
    itemButton17->Create( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton18 = new wxButton;
    itemButton18->Create( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    VSCPInformation m_info;
    wxString strBuf;

    // Fill in all VSCP Classes
    VSCPHashClass::iterator it;
    for ( it = geventinfo.getClassHashPointer()->begin(); 
            it != geventinfo.getClassHashPointer()->end(); 
            ++it ) {
        uint32_t key = it->first;
        wxString value = it->second;
        if ( key < 512 ) {
            strBuf.Printf( _(" - %d"), key );
            strBuf = value + strBuf;
            int sel = m_wxComboClass->Append( strBuf );
            m_wxComboClass->SetClientData( sel, 
                    reinterpret_cast<void*>( key ) );  // OK Don't worry I'm a HACKER and this is a HACK
        }
    }

    // Select first item
    m_wxComboClass->Select( 0 );

    wxCommandEvent event;
    OnComboboxSelected( event );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool DialogSetfiltermask::ShowToolTips()
{
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap DialogSetfiltermask::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DialogSetfiltermask::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
//

void DialogSetfiltermask::OnComboboxSelected( wxCommandEvent& event )
{
    int idxSelected;
    wxString strBuf;

    m_wxComboType->Clear();

    if ( wxNOT_FOUND != ( idxSelected = m_wxComboClass->GetSelection() ) ) {

        uint32_t selClass = (uintptr_t)m_wxComboClass->GetClientData( idxSelected );

        // Fill in all VSCP types
        VSCPHashType::iterator it;
        for ( it = geventinfo.getTypeHashPointer()->begin(); 
                it != geventinfo.getTypeHashPointer()->end(); 
                ++it ) {
            uint32_t key = it->first;
            wxString value = it->second;

            if ( ( selClass == ( key >> 16 ) ) ) {
                strBuf.Printf( _(" - %d"), ( key & 0xff ) );
                strBuf = value + strBuf;
                int selIdx = m_wxComboType->Append( strBuf );
                m_wxComboType->SetClientData( selIdx, 
                        reinterpret_cast<void*>( key ) );    // OK Don't worry I'm a HACKER and this is a HACK
            }
        }

        // Select first item
        m_wxComboType->Select( 0 );
    }

    event.Skip();
 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_EVENT
//

void DialogSetfiltermask::OnButtonAddEventClick( wxCommandEvent& event )
{
    int selidxType = m_wxComboType->GetSelection();
    uint32_t classtype = (uintptr_t)m_wxComboType->GetClientData( selidxType );

    // Check if the event is already added
    bool bFound = false;
    for ( unsigned int i=0; i<m_listboxEvents->GetCount(); i++ ) {
        if ( classtype == (uintptr_t)m_listboxEvents->GetClientData( i ) ) {
            bFound = true;
            break;
        }
    }

    if ( !bFound ) {
        wxString strBuf;

        strBuf = m_wxComboType->GetString( selidxType );
#ifdef WIN32	
        strBuf.Printf( _("Class = %s, Type = %s"), 
                        geventinfo.getClassDescription( classtype >> 16 ),
                        geventinfo.getTypeDescription( classtype >> 16, classtype & 0xff ) );
#else
	strBuf = _("Class = ");
	strBuf += geventinfo.getClassDescription( classtype >> 16 );
	strBuf += _(", Type = ");
	strBuf += geventinfo.getTypeDescription( classtype >> 16, classtype & 0xff );
#endif

        int selidx = m_listboxEvents->Append( strBuf );
        m_listboxEvents->SetClientData( selidx, 
                reinterpret_cast<void*>( classtype ) );    // OK Don't worry I'm a HACKER and this is a HACK 
    }

    // Update filter/masks
    calculateFilterMask();

    event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE_EVENT
//

void DialogSetfiltermask::OnButtonRemoveEventClick( wxCommandEvent& event )
{
    int selidx = m_listboxEvents->GetSelection();

    if ( wxNOT_FOUND != selidx ) {
        m_listboxEvents->Delete( selidx );
    }

    // Update filter/masks
    calculateFilterMask();
    
    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculateFilterMask
//

void DialogSetfiltermask::calculateFilterMask( void )
{
    // Reset filter masks
    m_classmask = 0xffff;
    m_classfilter = 0;
    m_typemask = 0xffff;
    m_typefilter = 0;

    // Nothing to do if no items
    if ( 0 == m_listboxEvents->GetCount() ) return;

    for ( int i = 0; i < 16; i++ ) {

        uint32_t classtype = (uintptr_t)m_listboxEvents->GetClientData( 0 );
        uint8_t lastClassBit = (uint8_t)( ( classtype >> 16 ) & (1 << i ) );
        uint8_t lastTypeBit  = (uint8_t)( ( classtype >>  0 ) & (1 << i ) );

        for ( unsigned int j = 0; j < m_listboxEvents->GetCount(); j++ ) {
            
            classtype = (uintptr_t)m_listboxEvents->GetClientData( j );
            uint16_t vscp_class = ( classtype >> 16 ) & 0xffff;
            uint16_t vscp_type  = ( classtype >>  0 ) & 0x00ff;

            // Class mask
            // Is this bit different then the last one?
            if ( ( vscp_class & ( 1 << i ) ) != lastClassBit ) {
                // Yes they are different. This means this
                // is a don't care bit for the mask
                // we set mask to zero
                m_classmask &= ~( 1 << i );
            }

            // Type mask
            // Is this bit different then the last one?
            if ( ( vscp_type & ( 1<<i ) ) != lastTypeBit ) {
                // Yes they are different. This means this
                // is a don't care bit for the mask
                // we set mask to zero
                m_typemask &= ~( 1 << i );
            }
        }

        // Class filter
        // If the class mask is set, consider the class value in the filter.
        if ( 0 != ( m_classmask & ( 1 << i ) ) ) {
            m_classfilter |= lastClassBit;
        }

        // Type filter
        // If the type mask is set, consider the type value in the filter.
        if ( 0 != ( m_typemask & ( 1 << i ) ) ) {
            m_typefilter |= lastTypeBit;
        }        
    }

    wxString strBuf;

    strBuf.Printf( _("class mask=%04x class filter=%04x type mask=%04x type filter=%04x" ),
                    m_classmask, m_classfilter, m_typemask, m_typefilter );
    m_statusText->SetLabel( strBuf );

}
