/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscptrmit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/11/2007 17:46:01
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
#pragma implementation "dlgvscptrmit.h"
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

#include <wx/tokenzr.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include "vscptxobj.h"
#include "dlgvscptrmit_images.h"
#include "dlgvscptrmit.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement type definition
//

IMPLEMENT_DYNAMIC_CLASS( dlgVSCPTrmitElement, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement event table definition
//

BEGIN_EVENT_TABLE( dlgVSCPTrmitElement, wxDialog )

    EVT_INIT_DIALOG( dlgVSCPTrmitElement::OnInitDialog )
    EVT_CLOSE( dlgVSCPTrmitElement::OnCloseWindow )
    EVT_COMBOBOX( ID_StrVscpClass, dlgVSCPTrmitElement::OnStrVscpClassSelected )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement constructors
//

dlgVSCPTrmitElement::dlgVSCPTrmitElement()
{
  Init();
}

dlgVSCPTrmitElement::dlgVSCPTrmitElement( wxWindow* parent, 
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
// dlgVSCPTrmitElement creator
//

bool dlgVSCPTrmitElement::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon( GetIconResource(wxT("fatbee_v2.ico") ) );
    if ( GetSizer() ) {
        GetSizer()->SetSizeHints(this);
    }

    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement destructor
//

dlgVSCPTrmitElement::~dlgVSCPTrmitElement()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void dlgVSCPTrmitElement::Init()
{
    m_wxChkActive = NULL;
    m_wxStrName = NULL;
    m_wxComboClass = NULL;
    m_wxComboType = NULL;
    m_wxChoicePriority = NULL;
    m_wxChkDefaultGUID = NULL;
    m_wxStrGUID15 = NULL;
    m_wxStrGUID14 = NULL;
    m_wxStrGUID13 = NULL;
    m_wxStrGUID12 = NULL;
    m_wxStrGUID11 = NULL;
    m_wxStrGUID10 = NULL;
    m_wxStrGUID9 = NULL;
    m_wxStrGUID8 = NULL;
    m_wxStrGUID7 = NULL;
    m_wxStrGUID6 = NULL;
    m_wxStrGUID5 = NULL;
    m_wxStrGUID4 = NULL;
    m_wxStrGUID3 = NULL;
    m_wxStrGUID2 = NULL;
    m_wxStrGUID1 = NULL;
    m_wxStrGUID0 = NULL;
    m_wxStrData = NULL;
    m_wxStrCount = NULL;
    m_wxStrPeriod = NULL;
    m_wxComboTrigger = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for dlgVSCPTrmitElement
//

void dlgVSCPTrmitElement::CreateControls()
{    
    // Transmit line dialog
    dlgVSCPTrmitElement* itemDialogTxEdit = this;

    // Vertical sizer
    wxBoxSizer* itemBoxVerticalSizer = new wxBoxSizer(wxVERTICAL);
    itemDialogTxEdit->SetSizer(itemBoxVerticalSizer);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxVerticalSizer->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxALL, 5);

    // Separator item
    itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_TOP|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    itemBoxSizer6->Add(itemFlexGridSizer, 1, wxALIGN_TOP|wxALL, 1);

    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

    m_wxChkActive = new wxCheckBox;
    m_wxChkActive->Create( itemDialogTxEdit, ID_CHECKBOX1, _("Active"), wxDefaultPosition, wxDefaultSize, 0 );
    m_wxChkActive->SetValue(true);
    if ( dlgVSCPTrmitElement::ShowToolTips() ) {
        m_wxChkActive->SetToolTip(_("Tick to activate the transmission event"));
    }
    m_wxChkActive->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Sans")));
    itemFlexGridSizer->Add(m_wxChkActive, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create( itemDialogTxEdit, wxID_STATIC, _("Name :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText10->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText10, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText10, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 1);
#endif    

    m_wxStrName = new wxTextCtrl;
    m_wxStrName->Create( itemDialogTxEdit, ID_StrName, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    
    if ( dlgVSCPTrmitElement::ShowToolTips() ) {
        m_wxStrName->SetToolTip(_("All transmission event should have a\n descriptive name.\n Set this name here."));
    }
    
    itemFlexGridSizer->Add(m_wxStrName, 2, wxGROW|wxALIGN_TOP|wxALL, 1);

    // Separator space
    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);
    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

    wxStaticText* itemStaticText14 = new wxStaticText;
    itemStaticText14->Create( itemDialogTxEdit, wxID_STATIC, _("Class :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText14->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )   
    itemFlexGridSizer->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL | wxADJUST_MINSIZE, 1);
#endif   

    wxArrayString m_wxComboClassStrings;
    m_wxComboClass = new wxComboBox;
    m_wxComboClass->Create( itemDialogTxEdit, ID_StrVscpClass, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboClassStrings, wxCB_READONLY );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxComboClass->SetToolTip(_("Select the VSCP class for the \ntransmission event here."));
    itemFlexGridSizer->Add(m_wxComboClass, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText16 = new wxStaticText;
    itemStaticText16->Create( itemDialogTxEdit, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText16->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )     
    itemFlexGridSizer->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    wxArrayString m_wxComboTypeStrings;
    m_wxComboType = new wxComboBox;
    m_wxComboType->Create( itemDialogTxEdit, ID_StrVscpType, wxEmptyString, wxDefaultPosition, wxSize(300, -1), m_wxComboTypeStrings, wxCB_READONLY );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxComboType->SetToolTip(_("Select the VSCP type for the \ntransmission event here."));
    itemFlexGridSizer->Add(m_wxComboType, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText18 = new wxStaticText;
    itemStaticText18->Create( itemDialogTxEdit, wxID_STATIC, _("Priority :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText18->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )     
    itemFlexGridSizer->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    wxArrayString m_wxChoicePriorityStrings;
    m_wxChoicePriorityStrings.Add(_("0 - Highest"));
    m_wxChoicePriorityStrings.Add(_("1 - Even higher"));
    m_wxChoicePriorityStrings.Add(_("2 - Higher"));
    m_wxChoicePriorityStrings.Add(_("3 - Normal high"));
    m_wxChoicePriorityStrings.Add(_("4 - Normal low"));
    m_wxChoicePriorityStrings.Add(_("5 - Lower"));
    m_wxChoicePriorityStrings.Add(_("6 - Even lower"));
    m_wxChoicePriorityStrings.Add(_("7 - Lowest"));
    m_wxChoicePriority = new wxChoice;
    m_wxChoicePriority->Create( itemDialogTxEdit, ID_WxChoicePriority, wxDefaultPosition, wxSize(120, -1), m_wxChoicePriorityStrings, 0 );
    m_wxChoicePriority->SetStringSelection(_("0"));
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxChoicePriority->SetToolTip(_("Select a priority for the \ntransmission event here. \nMost receiving applications will ignore this field."));
    itemFlexGridSizer->Add(m_wxChoicePriority, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    // Separators
    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);
    itemFlexGridSizer->Add(5, 20, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);
    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemDialogTxEdit, wxID_STATIC, _("Originating GUID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText23->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText23, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 0);
#else
    itemFlexGridSizer->Add(itemStaticText23, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 0);
#endif    

    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

    m_wxChkDefaultGUID = new wxCheckBox;
    m_wxChkDefaultGUID->Create( itemDialogTxEdit, ID_CHKBOX_USE_DEFAULT, _("Use default GUID"), wxDefaultPosition, wxDefaultSize, 0 );
    m_wxChkDefaultGUID->SetValue(true);
    m_wxChkDefaultGUID->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));
    itemFlexGridSizer->Add(m_wxChkDefaultGUID, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemDialogTxEdit, wxID_STATIC, _("15 - 8 :"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText26->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )     
    itemFlexGridSizer->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer->Add(itemBoxSizer27, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID15 = new wxTextCtrl;
    m_wxStrGUID15->Create( itemDialogTxEdit, ID_StrGUID15, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID15->SetToolTip(_("GUID byte 15"));
    itemBoxSizer27->Add(m_wxStrGUID15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID14 = new wxTextCtrl;
    m_wxStrGUID14->Create( itemDialogTxEdit, ID_WxStrGUID14, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID14->SetToolTip(_("GUID byte 14"));
    itemBoxSizer27->Add(m_wxStrGUID14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID13 = new wxTextCtrl;
    m_wxStrGUID13->Create( itemDialogTxEdit, ID_WxStrGUID13, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID13->SetToolTip(_("GUID byte 13"));
    itemBoxSizer27->Add(m_wxStrGUID13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID12 = new wxTextCtrl;
    m_wxStrGUID12->Create( itemDialogTxEdit, ID_WxStrGUID12, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID12->SetToolTip(_("GUID byte 12"));
    itemBoxSizer27->Add(m_wxStrGUID12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID11 = new wxTextCtrl;
    m_wxStrGUID11->Create( itemDialogTxEdit, ID_WxStrGUID11, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID11->SetToolTip(_("GUID byte 11"));
    itemBoxSizer27->Add(m_wxStrGUID11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID10 = new wxTextCtrl;
    m_wxStrGUID10->Create( itemDialogTxEdit, ID_WxStrGUID10, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID10->SetToolTip(_("GUID byte 10"));
    itemBoxSizer27->Add(m_wxStrGUID10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID9 = new wxTextCtrl;
    m_wxStrGUID9->Create( itemDialogTxEdit, ID_WxStrGUID9, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID9->SetToolTip(_("GUID byte 9"));
    itemBoxSizer27->Add(m_wxStrGUID9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID8 = new wxTextCtrl;
    m_wxStrGUID8->Create( itemDialogTxEdit, ID_WxStrGUID8, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID8->SetToolTip(_("GUID byte 8"));
    itemBoxSizer27->Add(m_wxStrGUID8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText36 = new wxStaticText;
    itemStaticText36->Create( itemDialogTxEdit, wxID_STATIC, _("7 - 0 :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText36->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer->Add(itemBoxSizer37, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID7 = new wxTextCtrl;
    m_wxStrGUID7->Create( itemDialogTxEdit, ID_WxStrGUID7, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID7->SetToolTip(_("GUID byte 7"));
    itemBoxSizer37->Add(m_wxStrGUID7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID6 = new wxTextCtrl;
    m_wxStrGUID6->Create( itemDialogTxEdit, ID_WxStrGUID6, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID6->SetToolTip(_("GUID byte 6"));
    itemBoxSizer37->Add(m_wxStrGUID6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID5 = new wxTextCtrl;
    m_wxStrGUID5->Create( itemDialogTxEdit, ID_WxStrGUID5, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID5->SetToolTip(_("GUID byte 5"));
    itemBoxSizer37->Add(m_wxStrGUID5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID4 = new wxTextCtrl;
    m_wxStrGUID4->Create( itemDialogTxEdit, ID_WxStrGUID4, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID4->SetToolTip(_("GUID byte 4"));
    itemBoxSizer37->Add(m_wxStrGUID4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID3 = new wxTextCtrl;
    m_wxStrGUID3->Create( itemDialogTxEdit, ID_WxStrGUID3, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID3->SetToolTip(_("GUID byte 3"));
    itemBoxSizer37->Add(m_wxStrGUID3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID2 = new wxTextCtrl;
    m_wxStrGUID2->Create( itemDialogTxEdit, ID_WxStrGUID2, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    m_wxStrGUID2->SetHelpText(_("GUID byte 2"));
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID2->SetToolTip(_("GUID byte 2"));
    itemBoxSizer37->Add(m_wxStrGUID2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID1 = new wxTextCtrl;
    m_wxStrGUID1->Create( itemDialogTxEdit, ID_WxStrGUID1, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID1->SetToolTip(_("GUID byte 1"));
    itemBoxSizer37->Add(m_wxStrGUID1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_wxStrGUID0 = new wxTextCtrl;
    m_wxStrGUID0->Create( itemDialogTxEdit, ID_WxStrGUID0, _("0"), wxDefaultPosition, wxSize(40, -1), wxTE_RIGHT );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrGUID0->SetToolTip(_("GUID byte 0"));
    itemBoxSizer37->Add(m_wxStrGUID0, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemDialogTxEdit, wxID_STATIC, _("Data :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText46->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )       
    itemFlexGridSizer->Add(itemStaticText46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    m_wxStrData = new wxTextCtrl;
    m_wxStrData->Create( itemDialogTxEdit, ID_StrVscpData, wxEmptyString, wxDefaultPosition, wxSize(-1, 50), wxTE_MULTILINE );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrData->SetToolTip(_("You enter data for the transmission event\n as a comma separated list. \nYou can arrange the data over several lines. \nHexadecimal values should be preceded with '0x'."));
    itemFlexGridSizer->Add(m_wxStrData, 5, wxGROW|wxALIGN_TOP|wxALL, 1);

    // Separators
    itemFlexGridSizer->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);
    itemFlexGridSizer->Add(5, 20, 0, wxALIGN_LEFT|wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 2);

    wxStaticText* itemStaticText50 = new wxStaticText;
    itemStaticText50->Create( itemDialogTxEdit, wxID_STATIC, _("Count :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText50->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText50, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText50, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    m_wxStrCount = new wxTextCtrl;
    m_wxStrCount->Create( itemDialogTxEdit, ID_StrCount, _("1"), wxDefaultPosition, wxSize(60, 25), 0 );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrCount->SetToolTip(_("Set event transmission event here. \nThis value sets how many times \nthis transmission event will be sent \nwhen triggered or doubleclicked."));
    itemFlexGridSizer->Add(m_wxStrCount, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

    wxStaticText* itemStaticText52 = new wxStaticText;
    itemStaticText52->Create( itemDialogTxEdit, wxID_STATIC, _("Period :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText52->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText52, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    m_wxStrPeriod = new wxTextCtrl;
    m_wxStrPeriod->Create( itemDialogTxEdit, ID_StrPeriod, _("0"), wxDefaultPosition, wxSize(60, 25), 0 );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxStrPeriod->SetToolTip(_("Set transmission event period here expressed as milliseconds. \nThe transmission event period is  interval between automatically sent events."));
    itemFlexGridSizer->Add(m_wxStrPeriod, 2, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 1);

    wxStaticText* itemStaticText54 = new wxStaticText;
    itemStaticText54->Create( itemDialogTxEdit, wxID_STATIC, _("Trigger :"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText54->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
#if ( ( wxMAJOR_VERSION >= 3 ) && ( wxMINOR_VERSION >= 1 ) )    
    itemFlexGridSizer->Add(itemStaticText54, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
#else
    itemFlexGridSizer->Add(itemStaticText54, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);
#endif    

    wxArrayString m_wxComboTriggerStrings;
    m_wxComboTrigger = new wxComboBox;
    m_wxComboTrigger->Create( itemDialogTxEdit, ID_ComboTrigger, wxEmptyString, wxDefaultPosition, wxSize(200, 25), m_wxComboTriggerStrings, wxCB_DROPDOWN );
    if (dlgVSCPTrmitElement::ShowToolTips())
        m_wxComboTrigger->SetToolTip(_("Select a predefined trigger here that will do send of this transmission event."));
    itemFlexGridSizer->Add(m_wxComboTrigger, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer56 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer56, 0, wxALIGN_BOTTOM|wxALL, 1);

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer56->Add(itemBoxSizer57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton58 = new wxButton;
    itemButton58->Create( itemDialogTxEdit, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(itemButton58, 0, wxALIGN_RIGHT|wxALL, 1);

    wxButton* itemButton59 = new wxButton;
    itemButton59->Create( itemDialogTxEdit, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(itemButton59, 0, wxALIGN_RIGHT|wxALL, 1);

    fillClassCombo();

    // Select first item
    m_wxComboClass->SetSelection( 0 );

    fillTypeCombo( 0 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
//

void dlgVSCPTrmitElement::OnInitDialog( wxInitDialogEvent& event )
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
//

void dlgVSCPTrmitElement::OnCloseWindow( wxCloseEvent& event )
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool dlgVSCPTrmitElement::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap dlgVSCPTrmitElement::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon dlgVSCPTrmitElement::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == wxT("fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }

    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getEvent
//

bool dlgVSCPTrmitElement::getEventData( VscpTXObj *pObj )
{
    wxString str;
  
    // Check pointer
    if ( NULL == pObj ) return false;
  
    pObj->m_bActive = m_wxChkActive->GetValue();
    pObj->m_wxStrName = m_wxStrName->GetValue(); 
    pObj->m_bUseDefaultGUID = m_wxChkDefaultGUID->GetValue();
    str = m_wxStrCount->GetValue();
    pObj->m_count = vscp_readStringValue( str );  
    str = m_wxStrPeriod->GetValue();
    pObj->m_period = vscp_readStringValue( str ); 
    str = m_wxComboTrigger->GetValue();
    pObj->m_trigger = vscp_readStringValue( str );  
  
	pObj->m_Event.head = ( m_wxChoicePriority->GetCurrentSelection() << 5 );

    pObj->m_Event.timestamp = 0;
    if ( wxNOT_FOUND == m_wxComboClass->GetSelection() ) {
        str = m_wxComboClass->GetValue();
        pObj->m_Event.vscp_class = vscp_readStringValue( str );
    }
    else {
        pObj->m_Event.vscp_class = 
        (uintptr_t)m_wxComboClass->GetClientData( m_wxComboClass->GetSelection() );
    }
  
    if ( wxNOT_FOUND == m_wxComboType->GetSelection() ) {
        str = m_wxComboType->GetValue();
        pObj->m_Event.vscp_type = vscp_readStringValue( str );
    }
    else {
        pObj->m_Event.vscp_type = 
        (uintptr_t)m_wxComboType->GetClientData( m_wxComboType->GetSelection() );
    }

	// Priority
	pObj->m_Event.head &= ~VSCP_HEADER_PRIORITY_MASK;
	pObj->m_Event.head |= ( m_wxChoicePriority->GetSelection() << 5 );
  
    pObj->m_bUseDefaultGUID = m_wxChkDefaultGUID->GetValue();
    if ( pObj->m_bUseDefaultGUID ) {
        for ( int i=0; i<16; i++ ) {
            pObj->m_Event.GUID[ i ] = 0;
        }
    }
    else {
        str = m_wxStrGUID0->GetValue();
        pObj->m_Event.GUID[ 0 ] = vscp_readStringValue( str );
        str = m_wxStrGUID1->GetValue();
        pObj->m_Event.GUID[ 1 ] = vscp_readStringValue( str );
        str = m_wxStrGUID2->GetValue();
        pObj->m_Event.GUID[ 2 ] = vscp_readStringValue( str );
        str = m_wxStrGUID3->GetValue();
        pObj->m_Event.GUID[ 3 ] = vscp_readStringValue( str );
        str = m_wxStrGUID4->GetValue();
        pObj->m_Event.GUID[ 4 ] = vscp_readStringValue( str );
        str = m_wxStrGUID5->GetValue();
        pObj->m_Event.GUID[ 5 ] = vscp_readStringValue( str );
        str = m_wxStrGUID6->GetValue();
        pObj->m_Event.GUID[ 6 ] = vscp_readStringValue( str );
        str = m_wxStrGUID7->GetValue();
        pObj->m_Event.GUID[ 7 ] = vscp_readStringValue( str );
        str = m_wxStrGUID8->GetValue();
        pObj->m_Event.GUID[ 8 ] = vscp_readStringValue( str );
        str = m_wxStrGUID9->GetValue();
        pObj->m_Event.GUID[ 9 ] = vscp_readStringValue( str );
        str = m_wxStrGUID10->GetValue();
        pObj->m_Event.GUID[ 10 ] = vscp_readStringValue( str );
        str = m_wxStrGUID11->GetValue();
        pObj->m_Event.GUID[ 11 ] = vscp_readStringValue( str );
        str = m_wxStrGUID12->GetValue();
        pObj->m_Event.GUID[ 12 ] = vscp_readStringValue( str );
        str = m_wxStrGUID13->GetValue();
        pObj->m_Event.GUID[ 13 ] = vscp_readStringValue( str );
        str = m_wxStrGUID14->GetValue();
        pObj->m_Event.GUID[ 14 ] = vscp_readStringValue( str );
        str = m_wxStrGUID15->GetValue();
        pObj->m_Event.GUID[ 15 ] = vscp_readStringValue( str );
    }
  
    // Data is allowed to scan multiple lines and to be in hex
    // or decimal form
    str = m_wxStrData->GetValue();
    vscp_setVscpEventDataFromString( &pObj->m_Event, str );
  
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// writeEventData
//

bool dlgVSCPTrmitElement::writeEventData( VscpTXObj *pObj )
{
	wxString str;
	VSCPInformation vscpinfo;
    unsigned int i;

	// Check pointer
	if ( NULL == pObj ) return false;
  
	m_wxChkActive->SetValue( pObj->m_bActive );
  
	m_wxStrName->SetValue( pObj->m_wxStrName ); 
  
	m_wxChkDefaultGUID->SetValue( pObj->m_bUseDefaultGUID );
  
	str.Printf(_("%lu"), pObj->m_count );
	m_wxStrCount->SetValue( str );
  
	str.Printf(_("%lu"), pObj->m_period );
	m_wxStrPeriod->SetValue( str );
  
	str.Printf(_("%lu"), pObj->m_trigger );
	m_wxComboTrigger->SetValue( str );
   
    for ( i=0; i<m_wxComboClass->GetCount(); i++ ) {
        if ( pObj->m_Event.vscp_class == (unsigned long)m_wxComboClass->GetClientData( i ) ) {
           m_wxComboClass->SetSelection( i );
           fillTypeCombo( pObj->m_Event.vscp_class );
           break;
        }
    }

    for ( i=0; i<m_wxComboType->GetCount(); i++ ) {
        if ( MAKE_CLASSTYPE_LONG( pObj->m_Event.vscp_class, 
                                   pObj->m_Event.vscp_type ) == (unsigned long)m_wxComboType->GetClientData( i ) ) {
           m_wxComboType->SetSelection( i );
           break;
        }
    }


	// Priority
	m_wxChoicePriority->SetSelection( ( pObj->m_Event.head & VSCP_HEADER_PRIORITY_MASK ) >> 5 );
 
	m_wxChkDefaultGUID->SetValue( pObj->m_bUseDefaultGUID );

	if ( pObj->m_bUseDefaultGUID ) {
		for ( int i=0; i<16; i++ ) {
			pObj->m_Event.GUID[ i ] = 0;
		}
	}

	str.Printf(_("%d"), pObj->m_Event.GUID[ 0 ] );
	m_wxStrGUID0->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 1 ] );
	m_wxStrGUID1->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 2 ] );
	m_wxStrGUID2->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 3 ] );
	m_wxStrGUID3->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 4 ] );
	m_wxStrGUID4->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 5 ] );
	m_wxStrGUID5->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 6 ] );
	m_wxStrGUID6->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 7 ] );
	m_wxStrGUID7->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 8 ] );
	m_wxStrGUID8->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 9 ] );
	m_wxStrGUID9->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 10 ] );
	m_wxStrGUID10->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 11 ] );
	m_wxStrGUID11->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 12 ] );
	m_wxStrGUID12->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 13 ] );
	m_wxStrGUID13->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 14 ] );
	m_wxStrGUID14->SetValue( str );
	str.Printf(_("%d"), pObj->m_Event.GUID[ 15 ] );
	m_wxStrGUID15->SetValue( str );
  
	// Data is allowed to scan multiple lines and to be in hex
	// or decimal form
    str.Empty();
	vscp_writeVscpDataToString( &pObj->m_Event, str );
	m_wxStrData->SetValue( str );
  
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_StrVscpClass
//

void dlgVSCPTrmitElement::OnStrVscpClassSelected( wxCommandEvent& event )
{
    int idx = m_wxComboClass->GetSelection();
    if ( wxNOT_FOUND != idx ) {
        fillTypeCombo( (unsigned long) m_wxComboClass->GetClientData( idx ) );
    }
    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillClassCombo
//

void dlgVSCPTrmitElement::fillClassCombo( void )
{
    VSCPInformation vscpInfo;
  
    // Clear the combo
    vscpInfo.fillClassDescriptions( (wxControlWithItems *)m_wxComboClass, WITH_DECIMAL_SUFFIX );
   
    // Select first item
    m_wxComboClass->SetSelection( 0 );
  
    // Fill the type combo to 
    fillTypeCombo( (unsigned long) m_wxComboClass->GetClientData( 0 ) );
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fillTypeCombo
//

void dlgVSCPTrmitElement::fillTypeCombo( unsigned int vscp_class )
{
    VSCPInformation vscpInfo;
  
    // Clear the combo
    vscpInfo.fillTypeDescriptions( (wxControlWithItems *)m_wxComboType, 
                                        vscp_class,
                                        WITH_DECIMAL_SUFFIX );

    // Select first item
    m_wxComboType->SetSelection( 0 );
  
}
