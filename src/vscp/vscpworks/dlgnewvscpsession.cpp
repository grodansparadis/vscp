/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnewvscpsession.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 09:36:35 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://m2m.sourceforge.net) 
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
#pragma implementation "dlgnewvscpsession.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/notebook.h>
#include <wx/list.h>

#include "vscp.h"
#include "vscphelper.h"
#include "vscpworks.h"
#include "dlgnewvscpsession.h"
#include "dlgnewvscpsession_images.h"
#include "dlgvscpinterfacesettings.h"

extern appConfiguration g_Config;


/*!
 * dlgNewVSCPSession type definition
 */

IMPLEMENT_DYNAMIC_CLASS(dlgNewVSCPSession, wxDialog)


/*!
 * dlgNewVSCPSession event table definition
 */

BEGIN_EVENT_TABLE(dlgNewVSCPSession, wxDialog)

EVT_INIT_DIALOG(dlgNewVSCPSession::OnInitDialog)
EVT_WINDOW_DESTROY(dlgNewVSCPSession::OnDestroy)
EVT_LISTBOX(ID_LISTBOX_INTERFACES, dlgNewVSCPSession::OnListboxInterfacesSelected)
EVT_LISTBOX_DCLICK(ID_LISTBOX_INTERFACES, dlgNewVSCPSession::OnListboxInterfacesDoubleClicked)
EVT_BUTTON(wxID_OK, dlgNewVSCPSession::OnOKClick)
EVT_BUTTON(wxID_CANCEL, dlgNewVSCPSession::OnCANCELClick)
EVT_BUTTON(ID_BUTTON_ADD, dlgNewVSCPSession::OnButtonAddClick)
EVT_BUTTON(ID_BUTTON_EDIT, dlgNewVSCPSession::OnButtonEditClick)
EVT_BUTTON(ID_BUTTON_REMOVE, dlgNewVSCPSession::OnButtonRemoveClick)
EVT_BUTTON(ID_BUTTON, dlgNewVSCPSession::OnButtonCloneClick)

END_EVENT_TABLE()


//////////////////////////////////////////////////////////////////////////////// 
// ctor
//

dlgNewVSCPSession::dlgNewVSCPSession()
{
    Init();
}

//////////////////////////////////////////////////////////////////////////////// 
// ctor
//

dlgNewVSCPSession::dlgNewVSCPSession( wxWindow* parent, 
                                            wxWindowID id, 
                                            const wxString& caption, 
                                            const wxPoint& pos, 
                                            const wxSize& size, 
                                            long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

//////////////////////////////////////////////////////////////////////////////// 
// Create
//

bool dlgNewVSCPSession::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style)
{

    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    SetIcon(GetIconResource(wxT("fatbee_v2.ico")));
    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////// 
// dtor
//

dlgNewVSCPSession::~dlgNewVSCPSession()
{
    
}

//////////////////////////////////////////////////////////////////////////////// 
// Init
//

void dlgNewVSCPSession::Init()
{
    m_ctrlListInterfaces = NULL;
    m_bShowUnconnectedMode = true;
}

//////////////////////////////////////////////////////////////////////////////// 
// CreateControls
//

void dlgNewVSCPSession::CreateControls()
{
    dlgNewVSCPSession* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString m_ctrlListInterfacesStrings;
    m_ctrlListInterfaces = new wxListBox;
    m_ctrlListInterfaces->Create( itemDialog1, 
                                    ID_LISTBOX_INTERFACES, 
                                    wxDefaultPosition, 
                                    wxSize(300, -1), 
                                    m_ctrlListInterfacesStrings, 
                                    wxLB_SINGLE);
    itemBoxSizer2->Add(m_ctrlListInterfaces, 0, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* itemButton5 = new wxButton;
    itemButton5->Create( itemDialog1, 
                            wxID_OK, 
                            _("&OK"), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    itemButton5->SetDefault();
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton5->SetToolTip(_("Use selected interface"));
    itemBoxSizer4->Add( itemButton5, 
                            0, 
                            wxALIGN_CENTER_HORIZONTAL | wxALL, 
                            1 );

    wxButton* itemButton6 = new wxButton;
    itemButton6->Create( itemDialog1, 
                            wxID_CANCEL, 
                            _("&Cancel"), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton6->SetToolTip(_("Go back without selecting any interface"));
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxButton* itemButton8 = new wxButton;
    itemButton8->Create( itemDialog1, 
                            ID_BUTTON_ADD, 
                            _("Add..."), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton8->SetToolTip(_("Add new interface"));
    itemBoxSizer4->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    wxButton* itemButton9 = new wxButton;
    itemButton9->Create( itemDialog1, 
                            ID_BUTTON_EDIT, 
                            _("Edit..."), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    itemButton9->SetDefault();
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton9->SetToolTip(_("Edit selected interface"));
    itemBoxSizer4->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create( itemDialog1, 
                            ID_BUTTON_REMOVE, 
                            _("Remove"), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton10->SetToolTip(_("Remove selected interface"));
    itemBoxSizer4->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, 
                            ID_BUTTON, 
                            _("Clone"), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            0 );
    if (dlgNewVSCPSession::ShowToolTips())
        itemButton11->SetToolTip(_("Remove selected interface"));
    itemBoxSizer4->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);

    // Connect events and objects
    itemDialog1->Connect( ID_DIALOG_NEW_VSCP_SESSION, 
                            wxEVT_DESTROY, 
                            wxWindowDestroyEventHandler( dlgNewVSCPSession::OnDestroy), 
                                                            NULL, 
                                                            this );

}

//////////////////////////////////////////////////////////////////////////////// 
// OnInitDialog
//

void dlgNewVSCPSession::OnInitDialog(wxInitDialogEvent& event)
{
    event.Skip();

    fillListBox( wxString(_("") ) );
}

//////////////////////////////////////////////////////////////////////////////// 
// OnDestroy
//

void dlgNewVSCPSession::OnDestroy(wxWindowDestroyEvent& event)
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool dlgNewVSCPSession::ShowToolTips()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// GetBitmapResource
//

wxBitmap dlgNewVSCPSession::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

//////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon dlgNewVSCPSession::GetIconResource(const wxString& name)
{
    // Icon retrieval
    wxUnusedVar(name);
    if (name == _T("fatbee_v2.ico")) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }
    return wxNullIcon;
}

//////////////////////////////////////////////////////////////////////////////
// OnOKClick
//

void dlgNewVSCPSession::OnOKClick(wxCommandEvent& event)
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// OnCANCELClick
//

void dlgNewVSCPSession::OnCANCELClick(wxCommandEvent& event)
{
    cleanupListbox();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// OnButtonAddClick
//

void dlgNewVSCPSession::OnButtonAddClick(wxCommandEvent& event)
{
    wxString strDecription;
    dlgVscpInterfaceSettings dlg(this);
    dlg.SetTitle(_("Add new VSCP/CANAL interface"));

    if (wxID_OK == dlg.ShowModal()) {

        // We add the driver to the structure
        if (dlg.m_DriverDescription->GetValue().Length()) {

            // A new CANAL driver
            canal_interface *pInfo = new canal_interface;
            if (NULL != pInfo) {
                strDecription = pInfo->m_strDescription = dlg.m_DriverDescription->GetValue();
                pInfo->m_strPath = dlg.m_PathToDriver->GetValue();
                pInfo->m_strConfig = dlg.m_DriverConfigurationString->GetValue();
                dlg.m_DriverFlags->GetValue().ToULong(&pInfo->m_flags);
                g_Config.m_canalIfList.Append(pInfo);

                // Write the configuration
                ::wxGetApp().writeConfiguration();

            }
        } 
        else if (dlg.m_RemoteServerDescription->GetValue().Length()) {

            // A new remote host
            vscp_interface *pInfo = new vscp_interface;
            if (NULL != pInfo) {

                strDecription = pInfo->m_strDescription = dlg.m_RemoteServerDescription->GetValue();

                pInfo->m_strHost = dlg.m_RemoteServerURL->GetValue();
                pInfo->m_strHost.Trim(false);
                pInfo->m_strHost.Trim();
                pInfo->m_strUser = dlg.m_RemoteServerUsername->GetValue();
                pInfo->m_strUser.Trim(false);
                pInfo->m_strUser.Trim();
                pInfo->m_strPassword = dlg.m_RemoteServerPassword->GetValue();
                pInfo->m_strPassword.Trim(false);
                pInfo->m_strPassword.Trim();
                pInfo->m_bLevel2 = dlg.m_fullLevel2->GetValue();
                wxString str;

                // Interface name
                pInfo->m_strInterfaceName = dlg.m_RemoteInterfaceName->GetValue();

                // Filter
                memcpy(&pInfo->m_vscpfilter, &dlg.m_vscpfilter, sizeof( vscpEventFilter));

                g_Config.m_vscpIfList.Append(pInfo);

                // Write the configuration
                ::wxGetApp().writeConfiguration();

            }
        } else {
            wxMessageBox(_("No driver added as a needed description is not found."));
        }

        fillListBox(strDecription);


    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// OnButtonEditClick
//

void dlgNewVSCPSession::OnButtonEditClick(wxCommandEvent& event)
{
    int selidx = -1;

    if (wxNOT_FOUND != (selidx = m_ctrlListInterfaces->GetSelection())) {

        if ((0 == selidx) && m_bShowUnconnectedMode) {
            wxMessageBox(_("Can't edit this line."));
        } 
        else {

            both_interface *pBoth = (both_interface *) m_ctrlListInterfaces->GetClientData(selidx);
            if (NULL != pBoth) {

                dlgVscpInterfaceSettings dlg(this);

                if (INTERFACE_CANAL == pBoth->m_type) {

                    dlg.SetTitle(_("Edit VSCP interface"));
                    dlg.m_DriverDescription->SetValue(pBoth->m_pcanalif->m_strDescription);
                    dlg.m_PathToDriver->SetValue(pBoth->m_pcanalif->m_strPath);
                    dlg.m_DriverConfigurationString->SetValue(pBoth->m_pcanalif->m_strConfig);
                    dlg.m_DriverFlags->SetValue(wxString::Format(_("%lu"), pBoth->m_pcanalif->m_flags));
                    dlg.GetBookCtrl()->SetSelection(0);
                    dlg.GetBookCtrl()->RemovePage(1);

                } 
                else if (INTERFACE_VSCP == pBoth->m_type) {

                    dlg.m_RemoteServerDescription->SetValue(pBoth->m_pvscpif->m_strDescription);
                    dlg.m_RemoteServerURL->SetValue(pBoth->m_pvscpif->m_strHost);
                    dlg.m_RemoteServerUsername->SetValue(pBoth->m_pvscpif->m_strUser);
                    dlg.m_RemoteServerPassword->SetValue(pBoth->m_pvscpif->m_strPassword);
                    dlg.m_RemoteInterfaceName->SetValue(pBoth->m_pvscpif->m_strInterfaceName);
                    memcpy(&dlg.m_vscpfilter, &pBoth->m_pvscpif->m_vscpfilter, sizeof( vscpEventFilter));

                    dlg.GetBookCtrl()->SetSelection(1);
                    dlg.GetBookCtrl()->RemovePage(0);
                } 
                else {
                    wxMessageBox(_("Unknown interface type!"), _("Edit Interface"), wxICON_STOP);
                    return;
                }

                // Show the dialog
                if (wxID_OK == dlg.ShowModal()) {

                    if (INTERFACE_CANAL == pBoth->m_type) {

                        pBoth->m_pcanalif->m_strDescription = dlg.m_DriverDescription->GetValue();
                        pBoth->m_pcanalif->m_strPath = dlg.m_PathToDriver->GetValue();
                        pBoth->m_pcanalif->m_strConfig = dlg.m_DriverConfigurationString->GetValue();
                        dlg.m_DriverFlags->GetValue().ToULong(&pBoth->m_pcanalif->m_flags);

                    } 
                    else if (INTERFACE_VSCP == pBoth->m_type) {

                        pBoth->m_pvscpif->m_strDescription = dlg.m_RemoteServerDescription->GetValue();
                        pBoth->m_pvscpif->m_strHost = dlg.m_RemoteServerURL->GetValue();
                        pBoth->m_pvscpif->m_strUser = dlg.m_RemoteServerUsername->GetValue();
                        pBoth->m_pvscpif->m_strPassword = dlg.m_RemoteServerPassword->GetValue();
                        pBoth->m_pvscpif->m_strInterfaceName = dlg.m_RemoteInterfaceName->GetValue();
                        pBoth->m_pvscpif->m_bLevel2 = dlg.m_fullLevel2->GetValue();

                        memcpy(&pBoth->m_pvscpif->m_vscpfilter, &dlg.m_vscpfilter, sizeof( vscpEventFilter));

                    }

                    // Write the configuration
                    ::wxGetApp().writeConfiguration();

                    fillListBox(wxString(_("")));

                    m_ctrlListInterfaces->SetSelection(selidx);
                }
            } 
            else {
                wxMessageBox(_("No data associated with listbox line"), _("Edit Interface"), wxICON_STOP);
            }
        } // 0 == selidx
    }
    else {
        wxMessageBox(_("You need to select one interface to edit before this operation can be performed."),
                _("Edit interface"),
                wxICON_INFORMATION);
    }

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// OnButtonRemoveClick
//

void dlgNewVSCPSession::OnButtonRemoveClick(wxCommandEvent& event)
{
    int selidx = -1;
    if (wxNOT_FOUND != (selidx = m_ctrlListInterfaces->GetSelection())) {

        if ((0 == selidx) && m_bShowUnconnectedMode) {
            wxMessageBox(_("Can't remove this line."));
        } 
        else {
            if (wxYES == wxMessageBox(_("Do you really want to remove interface?"),
                    _("Confirm"),
                    wxYES_NO | wxCANCEL)) {
                both_interface *pBoth = 
                    (both_interface *) m_ctrlListInterfaces->GetClientData(selidx);
                if (NULL != pBoth) {
                    if ((INTERFACE_CANAL == pBoth->m_type) && (NULL != pBoth->m_pcanalif)) {
                        if (g_Config.m_canalIfList.DeleteObject(pBoth->m_pcanalif)) {
                            delete pBoth->m_pcanalif;
                            fillListBox(wxString(_("")));
                            ::wxGetApp().writeConfiguration();
                        }
                    } 
                    else if ( ( INTERFACE_VSCP == pBoth->m_type ) && 
                                ( NULL != pBoth->m_pvscpif ) ) {
                        if (g_Config.m_vscpIfList.DeleteObject(pBoth->m_pvscpif)) {
                            delete pBoth->m_pvscpif;
                            fillListBox(wxString(_("")));
                            ::wxGetApp().writeConfiguration();
                        }
                    }
                }
            }
        }
    }
    else {
        wxMessageBox(_("You need to select one interface to remove before this operation can be performed."),
                _("Remove interface"),
                wxICON_INFORMATION);
    }

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// fillListBox
//

void dlgNewVSCPSession::fillListBox(const wxString& strDecription)
{
    cleanupListbox();

    if ( m_bShowUnconnectedMode ) {
        both_interface *pBoth = new both_interface; // Dummy for unconnected mode
        wxASSERT(NULL != pBoth);
        pBoth->m_pcanalif = NULL;
        pBoth->m_pvscpif = NULL;
        m_ctrlListInterfaces->Append(_("Unconnected Mode"), pBoth);
    }

    // Start with CANAL 
    LIST_CANAL_IF::iterator iterCanal;
    for ( iterCanal = g_Config.m_canalIfList.begin(); 
            iterCanal != g_Config.m_canalIfList.end(); 
            ++iterCanal ) {
        canal_interface *pIf = *iterCanal;
        both_interface *pBoth = new both_interface;
        if ( NULL != pBoth ) {
            pBoth->m_type = INTERFACE_CANAL;
            pBoth->m_pcanalif = pIf;
            m_ctrlListInterfaces->Append( _("CANAL: ") + 
                                            pIf->m_strDescription, pBoth );
        }
    }

    // VSCP TCP/IP interface
    LIST_VSCP_IF::iterator iterTcpip;
    for ( iterTcpip = g_Config.m_vscpIfList.begin(); 
            iterTcpip != g_Config.m_vscpIfList.end(); 
            ++iterTcpip ) {
        vscp_interface *pIf = *iterTcpip;
        both_interface *pBoth = new both_interface;
        if (NULL != pBoth) {
            pBoth->m_type = INTERFACE_VSCP;
            pBoth->m_pvscpif = pIf;
            m_ctrlListInterfaces->Append( _("TCP/IP: ") + 
                                            pIf->m_strDescription, pBoth );
        }
    }

    if ( !strDecription.IsEmpty() ) {
        if ( !m_ctrlListInterfaces->SetStringSelection( _("CANAL: ") + 
                                                        strDecription ) ) {
            m_ctrlListInterfaces->SetStringSelection( _("TCP/IP: ") + 
                                                        strDecription);
        }
    } else {
        // Select first item in list if items are available
        if ( m_ctrlListInterfaces->GetCount() ) {
            m_ctrlListInterfaces->Select(0);
        }
    }

}

////////////////////////////////////////////////////////////////////////////////
// OnButtonCloneClick
//

void dlgNewVSCPSession::OnButtonCloneClick(wxCommandEvent& event)
{
    int selidx = -1;

    if (wxNOT_FOUND != (selidx = m_ctrlListInterfaces->GetSelection())) {

        if ((0 == selidx) && m_bShowUnconnectedMode) {
            wxMessageBox(_("Can't edit this line."));
        } 
        else {

            both_interface *pBoth = 
                (both_interface *) m_ctrlListInterfaces->GetClientData(selidx);
            if (NULL != pBoth) {

                if (INTERFACE_CANAL == pBoth->m_type) {

                    // A new CANAL driver
                    canal_interface *pInfo = new canal_interface;
                    if (NULL != pInfo) {
                        pInfo->m_strDescription = 
                                wxGetTextFromUser(_("Enter new description"));
                        pInfo->m_strPath = pBoth->m_pcanalif->m_strPath;
                        pInfo->m_strConfig = pBoth->m_pcanalif->m_strConfig;
                        pInfo->m_flags = pBoth->m_pcanalif->m_flags;
                        g_Config.m_canalIfList.Append(pInfo);
                    }

                } 
                else if (INTERFACE_VSCP == pBoth->m_type) {

                    // A new remote host
                    vscp_interface *pInfo = new vscp_interface;
                    if (NULL != pInfo) {
                        pInfo->m_strDescription = 
                                wxGetTextFromUser(_("Enter description"));
                        pInfo->m_strHost = pBoth->m_pvscpif->m_strHost;
                        pInfo->m_strUser = pBoth->m_pvscpif->m_strUser;
                        pInfo->m_strPassword = pBoth->m_pvscpif->m_strPassword;
                        pInfo->m_strInterfaceName = pBoth->m_pvscpif->m_strInterfaceName;
                        pInfo->m_bLevel2 = pBoth->m_pvscpif->m_bLevel2;
                        memcpy( &pInfo->m_vscpfilter, 
                                    &pBoth->m_pvscpif->m_vscpfilter, 
                                    sizeof( vscpEventFilter));
                        g_Config.m_vscpIfList.Append(pInfo);
                    }

                } 
                else {
                    wxMessageBox( _("Unknown interface type!"), 
                                    _("Edit Interface"), wxICON_STOP);
                    return;
                }

                // Write the configuration
                ::wxGetApp().writeConfiguration();

                fillListBox(wxString(_("")));

                m_ctrlListInterfaces->SetSelection(selidx);

            } 
            else {
                wxMessageBox( _("No data associated with listbox line"), 
                                _("Edit Interface"), wxICON_STOP);
            }
        } // 0 == selidx
    }
    else {
        wxMessageBox(_("You need to select one interface to edit before "
                       "this operation can be performed."),
                _("Edit interface"),
                wxICON_INFORMATION);
    }
}


//////////////////////////////////////////////////////////////////////////////
// cleanupListbox
//

void dlgNewVSCPSession::cleanupListbox(void)
{
    int n;

    // Must be something in the lixtbic
    if (0 == (n = m_ctrlListInterfaces->GetCount())) return;

    for (unsigned int i = 0; i < m_ctrlListInterfaces->GetCount(); i++) {
        both_interface *pBoth = 
                    (both_interface *) m_ctrlListInterfaces->GetClientData(i);
        if (NULL != pBoth) delete pBoth;
    }

    // Clear the list
    m_ctrlListInterfaces->Clear();
}

//////////////////////////////////////////////////////////////////////////////// 
// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_INTERFACES
//

void dlgNewVSCPSession::OnListboxInterfacesSelected(wxCommandEvent& event)
{

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////////// 
// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_INTERFACES
//

void dlgNewVSCPSession::OnListboxInterfacesDoubleClicked(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
    return;
}




