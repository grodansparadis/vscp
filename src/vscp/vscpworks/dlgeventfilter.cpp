/////////////////////////////////////////////////////////////////////////////
//  Name:        dlgeventfilter.cpp
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

#ifdef WIN32
#include <winsock2.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/dynlib.h>
#include <wx/progdlg.h>

#include "vscp.h"
#include "vscphelper.h"
#include "vscpeventhelper.h"
#include "vscpworks.h"
#include "dlgclasstypeselect.h"
#include "dlgeventfilter.h"

extern VSCPInformation g_vscpinfo;

/*
 * EventFilter type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgEventFilter, wxDialog )


/*
 * EventFilter event table definition
 */

BEGIN_EVENT_TABLE( dlgEventFilter, wxDialog )

    EVT_LISTBOX( ID_LISTBOX_DISPLAY, dlgEventFilter::OnListboxDisplaySelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX_DISPLAY, dlgEventFilter::OnListboxDisplayDoubleClicked )
    EVT_BUTTON( ID_BUTTON_ADD_DISPLAY_EVENT, dlgEventFilter::OnButtonAddDisplayEventClick )
    EVT_LISTBOX( ID_LISTBOX_FILTER, dlgEventFilter::OnListboxFilterSelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX_FILTER, dlgEventFilter::OnListboxFilterDoubleClicked )
    EVT_BUTTON( ID_BUTTON_ADD_FILTER_EVENT, dlgEventFilter::OnButtonAddFilterEventClick )
    EVT_RADIOBUTTON( ID_RADIOBUTTON_DISPLAY, dlgEventFilter::OnRadiobuttonDisplaySelected )
    EVT_RADIOBUTTON( ID_RADIOBUTTON_FILTER, dlgEventFilter::OnRadiobuttonFilterSelected )
    EVT_CHECKBOX( ID_CHECKBOX_ENABLE, dlgEventFilter::OnCheckboxEnableClick )
    EVT_BUTTON( wxID_OK, dlgEventFilter::OnButtonOkClick )
    EVT_BUTTON( wxID_CANCEL, dlgEventFilter::OnButtonCancelClick )
    EVT_BUTTON( ID_BUTTON_LOAD, dlgEventFilter::OnButtonLoadClick )
    EVT_BUTTON( ID_BUTTON_SAVE, dlgEventFilter::OnButtonSaveClick )

    EVT_MENU( Menu_Popup_Display_Add, dlgEventFilter::OnButtonAddDisplayEventClick )
    EVT_MENU( Menu_Popup_Display_Remove, dlgEventFilter::OnRemoveDisplayRows )

    EVT_MENU( Menu_Popup_Filter_Add, dlgEventFilter::OnButtonAddFilterEventClick )
    EVT_MENU( Menu_Popup_Filter_Remove, dlgEventFilter::OnRemoveFilterRows )

END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
// Ctor's
//

dlgEventFilter::dlgEventFilter()
{
    Init();
}



dlgEventFilter::dlgEventFilter( wxWindow* parent, 
                                wxWindowID id, 
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

bool dlgEventFilter::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
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

dlgEventFilter::~dlgEventFilter()
{

}


///////////////////////////////////////////////////////////////////////////////
// Init
//

void dlgEventFilter::Init()
{
    m_ctrlListDisplay = NULL;
    m_ctrlListFilter = NULL;
    m_ctrlRadioDisplay = NULL;
    m_ctrlRadioFilter = NULL;
    m_ctrlCheckBoxEnable = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// CreateControls
//

void dlgEventFilter::CreateControls()
{    
    dlgEventFilter* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Display event(s) in this list"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlListDisplayStrings;
    m_ctrlListDisplay = new wxListBox( itemDialog1, ID_LISTBOX_DISPLAY, wxDefaultPosition, wxDefaultSize, m_ctrlListDisplayStrings, wxLB_MULTIPLE );
    if (dlgEventFilter::ShowToolTips())
        m_ctrlListDisplay->SetToolTip(_("Right click to add/remove/clone"));
    itemBoxSizer2->Add(m_ctrlListDisplay, 0, wxGROW|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_BUTTON_ADD_DISPLAY_EVENT, _("Add display event..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetDefault();
    if (dlgEventFilter::ShowToolTips())
        itemButton5->SetToolTip(_("Add event that should be displayed"));
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter away event(s) in this list"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText7->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_ctrlListFilterStrings;
    m_ctrlListFilter = new wxListBox( itemDialog1, ID_LISTBOX_FILTER, wxDefaultPosition, wxDefaultSize, m_ctrlListFilterStrings, wxLB_MULTIPLE );
    if (dlgEventFilter::ShowToolTips())
        m_ctrlListFilter->SetToolTip(_("Right click to add/remove/clone"));
    itemBoxSizer2->Add(m_ctrlListFilter, 0, wxGROW|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_BUTTON_ADD_FILTER_EVENT, _("Add filter event..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    if (dlgEventFilter::ShowToolTips())
        itemButton9->SetToolTip(_("Add event to filter out"));
    itemBoxSizer2->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ctrlRadioDisplay = new wxRadioButton( itemDialog1, ID_RADIOBUTTON_DISPLAY, _("Display"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlRadioDisplay->SetValue(false);
    if (dlgEventFilter::ShowToolTips())
        m_ctrlRadioDisplay->SetToolTip(_("Show only events in display list"));
    itemBoxSizer10->Add(m_ctrlRadioDisplay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ctrlRadioFilter = new wxRadioButton( itemDialog1, ID_RADIOBUTTON_FILTER, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlRadioFilter->SetValue(true);
    if (dlgEventFilter::ShowToolTips())
        m_ctrlRadioFilter->SetToolTip(_("Filter away events in filter list"));
    itemBoxSizer10->Add(m_ctrlRadioFilter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer10->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ctrlCheckBoxEnable = new wxCheckBox( itemDialog1, ID_CHECKBOX_ENABLE, _("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ctrlCheckBoxEnable->SetValue(false);
    if (dlgEventFilter::ShowToolTips())
        m_ctrlCheckBoxEnable->SetToolTip(_("Enable/disable filter"));
    itemBoxSizer10->Add(m_ctrlCheckBoxEnable, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetDefault();
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    //itemButton17->SetDefault();
    itemBoxSizer15->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton19 = new wxButton( itemDialog1, ID_BUTTON_LOAD, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
    //itemButton19->SetDefault();
    if (dlgEventFilter::ShowToolTips())
        itemButton19->SetToolTip(_("Load filter set from disk"));
    itemBoxSizer15->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, ID_BUTTON_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
    //itemButton20->SetDefault();
    if (dlgEventFilter::ShowToolTips())
        itemButton20->SetToolTip(_("Save filter set to disk"));
    itemBoxSizer15->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Connect events and objects
    m_ctrlListDisplay->Connect(ID_LISTBOX_DISPLAY, wxEVT_RIGHT_DOWN, wxMouseEventHandler(dlgEventFilter::OnRightDown), NULL, this);
    m_ctrlListFilter->Connect(ID_LISTBOX_FILTER, wxEVT_RIGHT_DOWN, wxMouseEventHandler(dlgEventFilter::OnRightDown), NULL, this);
}


///////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool dlgEventFilter::ShowToolTips()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// GetBitmapResource
//

wxBitmap dlgEventFilter::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

///////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon dlgEventFilter::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonAddDisplayEventClick
//

void dlgEventFilter::OnButtonAddDisplayEventClick( wxCommandEvent& event )
{
    uint16_t vscp_class = 0;
    uint16_t vscp_type = 0;

    dlgClassTypeSelect dlg(this);
    if (wxID_OK == dlg.ShowModal()) {
        
        int idxClass = dlg.m_ctrlComboClass->GetSelection();
        if ( wxNOT_FOUND != idxClass ) {
            vscp_class = (unsigned long)dlg.m_ctrlComboClass->GetClientData( idxClass );
        }

        int idxType = dlg.m_ctrlComboType->GetSelection();
        if ( wxNOT_FOUND != idxType ) {
            vscp_type = (unsigned long)dlg.m_ctrlComboType->GetClientData( idxType );
        }

        uint32_t clientdata = ( ( vscp_class<<16 ) + vscp_type );

        for ( uint16_t i=0; i<m_ctrlListDisplay->GetCount(); i++ ) {
            if ( clientdata == (unsigned long)m_ctrlListDisplay->GetClientData( i ) ) {
                wxMessageBox( _("Class/Type is allready defined.") );
               goto error;
            }
        }

        wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
        if ( 0 != vscp_type ) {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, vscp_type ) + _(" - ") +
                             str;
        }
        else {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
        }
        
        int n = m_ctrlListDisplay->Append( str, 
                reinterpret_cast<void*>( clientdata ) ); // OK Don't worry
 

    }

error:

    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonAddFilterEventClick
//

void dlgEventFilter::OnButtonAddFilterEventClick( wxCommandEvent& event )
{
    uint16_t vscp_class = 0;
    uint16_t vscp_type = 0;

    dlgClassTypeSelect dlg(this);
    if (wxID_OK == dlg.ShowModal()) {
        
        int idxClass = dlg.m_ctrlComboClass->GetSelection();
        if ( wxNOT_FOUND != idxClass ) {
            vscp_class = (unsigned long)dlg.m_ctrlComboClass->GetClientData( idxClass );
        }

        int idxType = dlg.m_ctrlComboType->GetSelection();
        if ( wxNOT_FOUND != idxType ) {
            vscp_type = (unsigned long)dlg.m_ctrlComboType->GetClientData( idxType );
        }

        uint32_t clientdata = ( ( vscp_class<<16 ) + vscp_type );

        for ( uint16_t i=0; i<m_ctrlListFilter->GetCount(); i++ ) {
            if ( clientdata == (unsigned long)m_ctrlListFilter->GetClientData( i ) ) {
                wxMessageBox( _("Class/Type is allready defined.") );
               goto error;
            }
        }

        wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
        if ( 0 != vscp_type ) {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, vscp_type ) + _(" - ") +
                             str;
        }
        else {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
        }
        
        int n = m_ctrlListFilter->Append( str, 
                reinterpret_cast<void*>( clientdata ) ); // OK Don't worry

    }

error:
    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonOkClick
//

void dlgEventFilter::OnButtonOkClick( wxCommandEvent& event )
{

    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonCancelClick
//

void dlgEventFilter::OnButtonCancelClick( wxCommandEvent& event )
{
    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonLoadClick
//

void dlgEventFilter::OnButtonLoadClick( wxCommandEvent& event )
{
    wxString str;
    wxXmlDocument doc;

    if ( m_ctrlListDisplay->GetCount() || m_ctrlListFilter->GetCount() ) {
        if (wxYES == wxMessageBox(_("Should current rows be removed before loading new filter from file?"),
                                    _("Remove rows?"),
                                    wxYES_NO | wxICON_QUESTION)) {
            m_ctrlListDisplay->Clear();
            m_ctrlListFilter->Clear();
        }
    }

    // First find a path to read RX data from
    wxFileDialog dlg(this,
            _("Choose file to load filter from "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("filter"),
            _("Filter (*.rxfilter)|*.rxfilter|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;
        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Prepare to loading filters..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);


        if (!doc.Load(dlg.GetPath())) {
            return;
        }

        // start processing the XML file xxxx
        if (doc.GetRoot()->GetName() != wxT("vscprxfilter")) {
            wxMessageBox(_("File with invalid format."));
            return;
        }

        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if ( child->GetName() == wxT("item") ) {

                long val;
                uint16_t vscp_class = 0;
                uint16_t vscp_type = 0;
                uint8_t filtertype = FILTER_MODE_DISPLAY;

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {
            
                    if ( subchild->GetName() == wxT("class")) {
                        str = subchild->GetNodeContent();
                        str.ToLong( &val );
                        vscp_class = val;
                    }
                    else if ( subchild->GetName() == wxT("type")) {
                        str = subchild->GetNodeContent();
                        str.ToLong( &val );
                        vscp_type = val;
                    }
                    else if ( subchild->GetName() == wxT("filter")) {
                        str = subchild->GetNodeContent();
                        str.ToLong( &val );
                        filtertype = val;
                    }

                    subchild = subchild->GetNext();

                } // while subchild


                if ( FILTER_MODE_DISPLAY == filtertype ) {

                    uint32_t clientdata = (vscp_class<<16) + vscp_type;
                
                    wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
                    if ( 0 != vscp_type ) {
                        str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, vscp_type ) + _(" - ") +
                             str;
                    }
                    else {
                        str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
                    }
        
                    m_ctrlListDisplay->Append( str, 
                            reinterpret_cast<void*>( clientdata ) ); // OK Don't worry

                }
                else if ( FILTER_MODE_FILTER == filtertype ) {

                    uint32_t clientdata = (vscp_class<<16) + vscp_type;
                
                    wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
                    if ( 0 != vscp_type ) {
                        str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, vscp_type ) + _(" - ") +
                             str;
                    }
                    else {
                        str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
                    }
        
                    m_ctrlListFilter->Append( str, 
                            reinterpret_cast<void*>( clientdata ) ); // OK Don't worry
                
                }

            }

            child = child->GetNext();

        } // while child
    }

    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnButtonSaveClick
//

void dlgEventFilter::OnButtonSaveClick( wxCommandEvent& event )
{
    wxString str;

    // First find a path to save RX data to
    wxFileDialog dlg(this,
            _("Please choose a file to save this filter to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("filter"),
            _("Filter (*.rxfilter)|*.rxfilter|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;

        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Saving filter..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // Filter data start
        pFileStream->Write("<vscprxfilter>\n", strlen("<vscprxfilter>\n"));
      
        if ( m_ctrlListDisplay->GetCount() ) {
            for ( size_t i=0; i<m_ctrlListDisplay->GetCount(); i++ ) {
                uint32_t clientdata = (unsigned long)m_ctrlListDisplay->GetClientData( i );
                pFileStream->Write("<item>\n", strlen("<item>\n"));    
                pFileStream->Write("<class>", strlen("<class>"));
                str.Printf( _("%d"), clientdata>>16 );
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</class>\n", strlen("</class>\n"));
                pFileStream->Write("<type>", strlen("<type>"));
                str.Printf( _("%d"), clientdata & 0xffff );
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</type>\n", strlen("</type>\n"));
                pFileStream->Write("<filter>0</filter>\n", strlen("<filter>0</filter>\n"));
                pFileStream->Write("</item>\n", strlen("</item>\n"));
            }
        }

        if ( m_ctrlListFilter->GetCount() ) {
            for ( size_t i=0; i<m_ctrlListFilter->GetCount(); i++ ) {
                uint32_t clientdata = (unsigned long)m_ctrlListFilter->GetClientData( i );
                pFileStream->Write("<item>\n", strlen("<item>\n"));    
                pFileStream->Write("<class>", strlen("<class>"));
                str.Printf( _("%d"), clientdata>>16 );
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</class>\n", strlen("</class>\n"));
                pFileStream->Write("<type>", strlen("<type>"));
                str.Printf( _("%d"), clientdata & 0xffff );
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</type>\n", strlen("</type>\n"));
                pFileStream->Write("<filter>1</filter>\n", strlen("<filter>1</filter>\n"));
                pFileStream->Write("</item>\n", strlen("</item>\n"));
            }
        }

        // Transmission set end
        pFileStream->Write("</vscprxfilter>\n", strlen("</vscprxfilter>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnListboxDisplaySelected
//

void dlgEventFilter::OnListboxDisplaySelected( wxCommandEvent& event )
{

    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnListboxDisplayDoubleClicked
//

void dlgEventFilter::OnListboxDisplayDoubleClicked( wxCommandEvent& event )
{

    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnRightDown
//

void dlgEventFilter::OnRightDown( wxMouseEvent& event )
{
    wxMenu menu;
    wxPoint pos = ClientToScreen(event.GetPosition());

    if (ID_LISTBOX_DISPLAY == event.GetId()) {
        menu.Append( Menu_Popup_Display_Add, _T("Add") );
        menu.Append( Menu_Popup_Display_Remove, _T("Remove") );
        menu.AppendSeparator();
    } 
    else if (ID_LISTBOX_FILTER == event.GetId()) {
        menu.Append( Menu_Popup_Filter_Add, _T("Add") );
        menu.Append( Menu_Popup_Filter_Remove, _T("Remove") );
        menu.AppendSeparator();
    }

    PopupMenu(&menu/*, pos.x, pos.y*/);
    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnListboxFilterSelected
//

void dlgEventFilter::OnListboxFilterSelected( wxCommandEvent& event )
{

    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnListboxFilterDoubleClicked
//

void dlgEventFilter::OnListboxFilterDoubleClicked( wxCommandEvent& event )
{

    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnCheckboxEnableClick
//

void dlgEventFilter::OnCheckboxEnableClick( wxCommandEvent& event )
{
    m_bfilterActive = m_ctrlCheckBoxEnable->GetValue();
    event.Skip();
}


///////////////////////////////////////////////////////////////////////////////
// OnRadiobuttonFilterSelected
//

void dlgEventFilter::OnRadiobuttonFilterSelected( wxCommandEvent& event )
{
    m_nfilterMode = FILTER_MODE_FILTER;
    event.Skip(); 
}


///////////////////////////////////////////////////////////////////////////////
// OnRadiobuttonDisplaySelected
//

void dlgEventFilter::OnRadiobuttonDisplaySelected( wxCommandEvent& event )
{
    m_nfilterMode = FILTER_MODE_DISPLAY;
    event.Skip(); 
}

///////////////////////////////////////////////////////////////////////////////
// OnRemoveDisplayRows
//

void dlgEventFilter::OnRemoveDisplayRows( wxCommandEvent& event )
{
    wxBusyCursor wait;

    if ( m_ctrlListDisplay->GetCount() ) {
        wxArrayInt selrows;
        m_ctrlListDisplay->GetSelections( selrows );
        if (selrows.GetCount()) {
            for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                m_ctrlListDisplay->Delete( i );
            }
        }
    }

    event.Skip();
}

///////////////////////////////////////////////////////////////////////////////
// OnRemoveFilterRows
//

void dlgEventFilter::OnRemoveFilterRows( wxCommandEvent& event )
{
    wxBusyCursor wait;

    if ( m_ctrlListFilter->GetCount() ) {
        wxArrayInt selrows;
        m_ctrlListFilter->GetSelections( selrows );
        if (selrows.GetCount()) {
            for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                m_ctrlListFilter->Delete( i );
            }
        }
    }

    event.Skip();
}