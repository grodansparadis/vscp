/////////////////////////////////////////////////////////////////////////////
// Name:        dialogabstractionedit.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     26/05/2009 20:48:56
// RCS-ID:      
// Copyright:   (C) 2009-2018 
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
#pragma implementation "dlgabstractionedit.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "dlgabstractionedit.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogAbstractionEdit type definition
//

IMPLEMENT_DYNAMIC_CLASS( DialogAbstractionEdit, wxDialog )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DialogAbstractionEdit event table definition
//

BEGIN_EVENT_TABLE( DialogAbstractionEdit, wxDialog )
     EVT_BUTTON( ID_BUTTON18, DialogAbstractionEdit::OnButtonDefaultClick )
END_EVENT_TABLE()


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DialogAbstractionEdit constructors
    //

    DialogAbstractionEdit::DialogAbstractionEdit()
{
    Init();
}

DialogAbstractionEdit::DialogAbstractionEdit( wxWindow* parent, 
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
// DialogAbstractionEdit creator
//

bool DialogAbstractionEdit::Create( wxWindow* parent, 
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
// DialogAbstractionEdit destructor
//

DialogAbstractionEdit::~DialogAbstractionEdit()
{
    ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DialogAbstractionEdit::Init()
{
    m_nType = 0;
    m_abstractionName = NULL;
    m_abstractionId = NULL;
    m_abstractionType = NULL;
    m_abstractionWidth = NULL;
    m_abstractionRegisterPage = NULL;
    m_abstractionRegisterOffset = NULL;
    m_abstractionDescription = NULL;
    m_abstractionHelp = NULL;
    m_abstractionAccessRights = NULL;
    m_abstractionDefaultValue = NULL;
    m_singleValueLabel = NULL;
    m_abstractionValue = NULL;
    m_multipleValueLabel = NULL;
    m_abstractionComboValue = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DialogAbstractionEdit
//

void DialogAbstractionEdit::CreateControls()
{
    DialogAbstractionEdit* itemDialogAbstraction = this;

    wxGridSizer* itemGridSizerWindow = new wxGridSizer(0, 2, 0, 0); 
    itemDialogAbstraction->SetSizer( itemGridSizerWindow );
    
    
    // Name

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create(itemDialogAbstraction, wxID_STATIC, _("Name :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)      
    itemStaticText6->SetFont( wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold() ) );
#else 
    itemStaticText6->SetFont( wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif
    itemGridSizerWindow->Add( itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);    

    m_abstractionName = new wxStaticText;
    m_abstractionName->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionName, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2 );

    
    
    // id

    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create(itemDialogAbstraction, wxID_STATIC, _("Id :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText10->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold() ) );
#else
    itemStaticText10->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif    
    itemGridSizerWindow->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionId = new wxStaticText;
    m_abstractionId->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionId, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    // Type

    wxStaticText* itemStaticText14 = new wxStaticText;
    itemStaticText14->Create(itemDialogAbstraction, wxID_STATIC, _("Type :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText14->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText14->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionType = new wxStaticText;
    m_abstractionType->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionType, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    // Width

    wxStaticText* itemStaticText18 = new wxStaticText;
    itemStaticText18->Create(itemDialogAbstraction, wxID_STATIC, _("Width :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText18->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText18->SetFont( wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif    
    itemGridSizerWindow->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionWidth = new wxStaticText;
    m_abstractionWidth->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionWidth, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    // Page

    wxStaticText* itemStaticText22 = new wxStaticText;
    itemStaticText22->Create(itemDialogAbstraction, wxID_STATIC, _("Register page :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText22->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText22->SetFont( wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
#endif    
    itemGridSizerWindow->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionRegisterPage = new wxStaticText;
    m_abstractionRegisterPage->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionRegisterPage, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    // Offset

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create(itemDialogAbstraction, wxID_STATIC, _("Register Offset :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText26->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText26->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionRegisterOffset = new wxStaticText;
    m_abstractionRegisterOffset->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionRegisterOffset, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    // Description text

    wxStaticText* itemStaticText30 = new wxStaticText;
    itemStaticText30->Create(itemDialogAbstraction, wxID_STATIC, _("Description :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)     
    itemStaticText30->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText30->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add( itemStaticText30, 
                                0, 
                                wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 
                                1);

    m_abstractionDescription = new wxStaticText;
    m_abstractionDescription->Create( itemDialogAbstraction, 
                                        wxID_STATIC,
                                        _("-----"),
                                        wxDefaultPosition, 
                                        wxSize(500, 20), 
#if wxCHECK_VERSION(3, 1, 0)                                       
                                        wxST_ELLIPSIZE_END );
#else
                                        0 );    
#endif                                        
                                        
    //m_abstractionDescription->Wrap(500);
    itemGridSizerWindow->Add(m_abstractionDescription, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    // Help text
    
    wxStaticText* itemStaticText34 = new wxStaticText;
    itemStaticText34->Create( itemDialogAbstraction, 
                                wxID_STATIC, 
                                _("Help :"), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                0 );
#if  wxCHECK_VERSION(2, 9, 5)     
    itemStaticText34->SetFont(wxFont( wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText34->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add( itemStaticText34, 
                                0, 
                                wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 
                                1 );

    m_abstractionHelp = new wxStaticText;
    m_abstractionHelp->Create( itemDialogAbstraction, 
                                wxID_STATIC, 
                                _("-----"), 
                                wxDefaultPosition, 
                                wxSize(500, 60), 
#if wxCHECK_VERSION(3, 1, 0)            
                                wxST_ELLIPSIZE_END );
#else
                                0 );
#endif    
    itemGridSizerWindow->Add( m_abstractionHelp, 
                                0, 
                                wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 
                                1 );

    // Access rights

    wxStaticText* itemStaticText38 = new wxStaticText;
    itemStaticText38->Create(itemDialogAbstraction, wxID_STATIC, _("Access rights :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText38->SetFont(wxFont(wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText38->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add(itemStaticText38, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionAccessRights = new wxStaticText;
    m_abstractionAccessRights->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(200, -1), 0);
    itemGridSizerWindow->Add(m_abstractionAccessRights, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    
    
    
    // Default

    wxStaticText* itemStaticText42 = new wxStaticText;
    itemStaticText42->Create(itemDialogAbstraction, wxID_STATIC, _("Default value :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    itemStaticText42->SetFont(wxFont(wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    itemStaticText42->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add(itemStaticText42, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_abstractionDefaultValue = new wxStaticText;
    m_abstractionDefaultValue->Create(itemDialogAbstraction, wxID_STATIC, _("-----"), wxDefaultPosition, wxSize(170, -1), 0);
    itemGridSizerWindow->Add(m_abstractionDefaultValue, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    // Single Value

    m_singleValueLabel = new wxStaticText;
    m_singleValueLabel->Create(itemDialogAbstraction, wxID_STATIC, _("Value :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    m_singleValueLabel->SetFont(wxFont(wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else
    m_singleValueLabel->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif    
    itemGridSizerWindow->Add( m_singleValueLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1 );
    
    wxBoxSizer* itemBoxSizerValue = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizerWindow->Add( itemBoxSizerValue, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1 );

    m_abstractionValue = new wxTextCtrl;
    m_abstractionValue->Create(itemDialogAbstraction, ID_TEXTCTRL_ABSTRACTION_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizerValue->Add( m_abstractionValue, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1 );
    
    wxButton* itemButtonDefault = new wxButton;
    itemButtonDefault->Create(itemDialogAbstraction, ID_BUTTON18, _("Set default value"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizerValue->Add( itemButtonDefault, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);


    
    // Select value

    m_multipleValueLabel = new wxStaticText;
    m_multipleValueLabel->Create(itemDialogAbstraction, wxID_STATIC, _("Value :"), wxDefaultPosition, wxDefaultSize, 0);
#if  wxCHECK_VERSION(2, 9, 5)    
    m_multipleValueLabel->SetFont(wxFont(wxFontInfo(-1).FaceName("Tahoma").Bold()));
#else 
    m_multipleValueLabel->SetFont(wxFont( -1, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ));
#endif
    itemGridSizerWindow->Add(m_multipleValueLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    

    wxArrayString m_abstractionComboValueStrings;
    m_abstractionComboValue = new wxChoice;
    m_abstractionComboValue->Create(itemDialogAbstraction, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, m_abstractionComboValueStrings, 0);
    itemGridSizerWindow->Add(m_abstractionComboValue, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    
    
    
    // OK/CANCEL buttons

    wxButton* itemButton54 = new wxButton;
    itemButton54->Create(itemDialogAbstraction, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
    itemGridSizerWindow->Add(itemButton54, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxButton* itemButton55 = new wxButton;
    itemButton55->Create(itemDialogAbstraction, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    itemGridSizerWindow->Add(itemButton55, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    itemGridSizerWindow->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5); 
    itemGridSizerWindow->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool DialogAbstractionEdit::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap DialogAbstractionEdit::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DialogAbstractionEdit::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transfers data to the window
//

bool DialogAbstractionEdit::TransferDataToWindow( CMDF_Abstraction *pAbstraction, wxString &strValue )
{
    wxString str;

    if ( NULL != pAbstraction) {

        // Save the abstraction
        m_pAbstraction = pAbstraction;

        // Fill in the data
        m_abstractionName->SetLabel( pAbstraction->m_strName );
        m_abstractionName->SetToolTip( pAbstraction->m_strDescription );

        m_abstractionId->SetLabel( pAbstraction->m_strID );
        m_abstractionId->SetToolTip( pAbstraction->m_strDescription );

        wxString strType;
        switch ( pAbstraction->m_nType ) {

        case type_string: 
            strType = _("String");
            break;

        case type_bitfield:
            strType = _("Bitfield");
            break;

        case type_boolval:
            strType = _("Boolean");
            break;

        case type_int8_t:
            strType = _("Signed 8-bit integer");
            break;

        case type_uint8_t:
            strType = _("Unsigned 8-bit integer");
            break;

        case type_int16_t:
            strType = _("Signed 16-bit integer");
            break;

        case type_uint16_t:
            strType = _("Unsigned 16-bit integer");
            break;

        case type_int32_t:
            strType = _("Signed 32-bit integer");
            break;

        case type_uint32_t:
            strType = _("Unsigned 32-bit integer");
            break;

        case type_int64_t:
            strType = _("Signed 64-bit integer");
            break;

        case type_uint64_t:
            strType = _("Unsigned 64-bit integer");
            break;

        case type_float:
            strType = _("Float");
            break;

        case type_double:
            strType = _("Double");
            break;

        case type_date:
            strType = _("Date");
            break;

        case type_time:
            strType = _("Time");
            break;

        case type_guid:
            strType = _("GUID");
            break;

        case type_unknown:

        default:
            strType = _("Unknown Type");
            break;
        }


        str.Printf(_("%d"), pAbstraction->m_nType );
        m_abstractionType->SetLabel( str + _(" - ") + strType );

        str.Printf(_("%d"), pAbstraction->m_nWidth );
        m_abstractionWidth->SetLabel( str );

        str.Printf(_("%d"), pAbstraction->m_nPage );
        m_abstractionRegisterPage->SetLabel( str );

        str.Printf(_("%d"), pAbstraction->m_nOffset );
        m_abstractionRegisterOffset->SetLabel( str );

        m_abstractionDescription->SetLabel( pAbstraction->m_strDescription );
        m_abstractionDescription->SetToolTip( pAbstraction->m_strDescription ); 

        m_abstractionHelp->SetLabel( pAbstraction->m_strHelp );
        m_abstractionHelp->SetToolTip( pAbstraction->m_strHelp ); 

        if ( pAbstraction->m_strDefault.Length() ) {
            m_abstractionDefaultValue->SetLabel( pAbstraction->m_strDefault );
        }
        else {
            m_abstractionDefaultValue->SetLabel( strValue );
        }

        // Access
        str = _("");
        if ( pAbstraction->m_nAccess & MDF_ACCESS_READ ){
            str = _("r");
        }
        if ( pAbstraction->m_nAccess & MDF_ACCESS_WRITE ){
            str += _("w");
        }
        else {
            // Read only
            m_abstractionComboValue->Enable( false );
            m_multipleValueLabel->Enable( false );
            m_abstractionValue->Enable( false );
        }
        m_abstractionAccessRights->SetLabel( str );

        // Set value to read only if value is not writable
        if ( wxNOT_FOUND == str.Find(_("w")) ) {
            m_abstractionValue->SetEditable( false );
            //m_abstractionComboValue->SetEditable( false );
        }
        else {
            m_abstractionValue->SetEditable( true );
        }

        // Check if value only is selectable from a list of 
        // values
        if ( pAbstraction->m_list_value.GetCount() ) {

            // Hide the single value static
            m_abstractionValue->Show( false );
            m_singleValueLabel->Show( false );

            // Fill possible values into combo
            MDF_VALUE_LIST::iterator iter;
            for ( iter = pAbstraction->m_list_value.begin(); 
                iter != pAbstraction->m_list_value.end(); ++iter ) {
                    CMDF_ValueListValue *pMdfValue = *iter;
                    m_abstractionComboValue->Append( pMdfValue->m_strValue );
                    m_abstractionComboValue->SetStringSelection( strValue );
            }

        }
        // Just single value
        else {

            // Hide the multiple value combo
            m_abstractionComboValue->Show( false );
            m_multipleValueLabel->Show( false );

            m_abstractionValue->SetValue( strValue );

        }

    }

    return true;
    //return wxDialog::TransferDataToWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transfers data from the window
//

bool DialogAbstractionEdit::TransferDataFromWindow( wxString &strValue )
{
    strValue = _("");

    if ( NULL != m_pAbstraction ) {

        if ( m_abstractionComboValue->GetCount() ) {
            strValue = m_abstractionComboValue->GetStringSelection();
        }
        else {
            strValue = m_abstractionValue->GetValue();
        }

    } // Valid pointer

    return wxDialog::TransferDataFromWindow();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonDefaultClick
//

void DialogAbstractionEdit::OnButtonDefaultClick( wxCommandEvent& event )
{
    if ( m_abstractionComboValue->GetCount() ) {
        for ( unsigned int i=0; i<m_pAbstraction->m_list_value.GetCount(); i++ ) {
            if ( wxNOT_FOUND != m_abstractionComboValue->GetString( i )
                .Find( m_abstractionDefaultValue->GetLabel() ) ) {
                    m_abstractionComboValue->SetSelection( i );
            }
        }
    }
    else {
        m_abstractionValue->SetLabel( m_abstractionDefaultValue->GetLabel() );
    }

    event.Skip();
}

