/////////////////////////////////////////////////////////////////////////////
// Name:        frmScanfordevices.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     23/05/2009 17:40:41
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
#pragma implementation "frmscanfordevices.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <list>

#include "frmdeviceconfig.h"
#include "wx/imaglist.h"

#include <wx/tokenzr.h>
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/listimpl.cpp>

#include "vscpworks.h"
#include <canal.h>
#include <vscp.h>
#include <vscphelper.h>
#include "dlgabout.h"
#include "frmscanfordevices_images.h"
#include "frmscanfordevices.h"

extern appConfiguration g_Config;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices type definition
//

IMPLEMENT_CLASS(frmScanforDevices, wxFrame)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices event table definition
//

BEGIN_EVENT_TABLE(frmScanforDevices, wxFrame)

    EVT_CLOSE( frmScanforDevices::OnCloseWindow )
    EVT_MENU( ID_MENU_VSCPWORKS_EXIT, frmScanforDevices::OnMenuitemExitClick )
    EVT_MENU( ID_MENUITEM_HELP, frmScanforDevices::OnMenuitemHelpClick )
    EVT_MENU( ID_MENUITEM_HELP_FAQ, frmScanforDevices::OnMenuitemHelpFaqClick )
    EVT_MENU( ID_MENUITEM_HELP_SC, frmScanforDevices::OnMenuitemHelpScClick )
    EVT_MENU( ID_MENUITEM_HELP_THANKS, frmScanforDevices::OnMenuitemHelpThanksClick )
    EVT_MENU( ID_MENUITEM_HELP_CREDITS, frmScanforDevices::OnMenuitemHelpCreditsClick )
    EVT_MENU( ID_MENUITEM_HELP_VSCP_SITE, frmScanforDevices::OnMenuitemHelpVscpSiteClick )
    EVT_MENU( ID_MENUITEM_HELP_ABOUT, frmScanforDevices::OnMenuitemHelpAboutClick )
    EVT_TREE_SEL_CHANGED( ID_TREE_DEVICE, frmScanforDevices::OnTreeDeviceSelChanged )
    EVT_TREE_ITEM_RIGHT_CLICK( ID_TREE_DEVICE, frmScanforDevices::OnTreeDeviceItemRightClick )
    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW3, frmScanforDevices::OnHtmlwindow3LinkClicked )
    EVT_BUTTON( ID_BUTTON_SCAN, frmScanforDevices::OnButtonScanClick )

    EVT_MENU(Menu_Popup_GetNodeInfo, frmScanforDevices::getNodeInfo)
    EVT_MENU(Menu_Popup_OpenConfig, frmScanforDevices::openConfiguration)

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices constructors
//

frmScanforDevices::frmScanforDevices()
{
    Init();
}

frmScanforDevices::frmScanforDevices( wxWindow* parent,
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
// frmScanforDevices creator
//

bool frmScanforDevices::Create( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style )
{
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon( GetIconResource( wxT("fatbee_v2.ico") ) );
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmScanforDevices destructor
//

frmScanforDevices::~frmScanforDevices()
{
    ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmScanforDevices::Init()
{
    m_pPanel = NULL;
    m_labelInterface = NULL;
    m_DeviceTree = NULL;
    m_htmlWnd = NULL;
    m_slowAlgorithm = NULL;
    m_ctrlEditFrom = NULL;
    m_ctrlEditTo = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for frmScanforDevices
//

void frmScanforDevices::CreateControls()
{
    frmScanforDevices* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->AppendSeparator();
    itemMenu3->Append(ID_MENU_VSCPWORKS_EXIT, _("Exit"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    
    wxMenu* itemMenu6 = new wxMenu;
    itemMenu6->Append(ID_MENUITEM14, _("Scan for nodes..."), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu6, _("Tools"));
    
    wxMenu* itemMenu8 = new wxMenu;
    itemMenu8->Append(ID_MENUITEM_HELP, _("VSCP-Works Help"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MENUITEM_HELP_FAQ, _("Frequently Asked Questions"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MENUITEM_HELP_SC, _("Keyboard shortcuts"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->AppendSeparator();
    itemMenu8->Append(ID_MENUITEM_HELP_THANKS, _("Thanks..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MENUITEM_HELP_CREDITS, _("Credits..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->AppendSeparator();
    itemMenu8->Append(ID_MENUITEM_HELP_VSCP_SITE, _("Go to VSCP site"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->AppendSeparator();
    itemMenu8->Append(ID_MENUITEM_HELP_ABOUT, _("About"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu8, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    m_pPanel = new wxPanel;
    m_pPanel->Create( itemFrame1,
                        ID_PANEL_DEVICE_SCAN, 
                        wxDefaultPosition, 
                        wxDefaultSize, 
                        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxVERTICAL);
    m_pPanel->SetSizer(itemBoxSizer20);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer20->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_labelInterface = new wxStaticText;
    m_labelInterface->Create( m_pPanel, 
                                wxID_STATIC, 
                                _("Searching on interface: "), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                0 );
    m_labelInterface->SetName(wxT("m_staticInterfaceName"));
    itemBoxSizer21->Add( m_labelInterface, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer20->Add(itemBoxSizer23, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer23->Add(itemBoxSizer24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( m_pPanel, wxID_STATIC, _("Found devices"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DeviceTree = new wxTreeCtrl;
    m_DeviceTree->Create( m_pPanel, 
                            ID_TREE_DEVICE, 
                            wxDefaultPosition, 
                            wxSize(250, 340), 
                            wxTR_SINGLE | wxSIMPLE_BORDER );
    m_DeviceTree->SetBackgroundColour( wxColour(231, 235, 184) );
    itemBoxSizer24->Add( m_DeviceTree, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer23->Add( itemBoxSizer27, 0, wxGROW | wxALL, 5 );

    wxStaticText* itemStaticText28 = new wxStaticText;
    itemStaticText28->Create( m_pPanel, 
                                wxID_STATIC, 
                                _("Device information"), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                0 );
    itemBoxSizer27->Add( itemStaticText28, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_htmlWnd = new wxHtmlWindow;
    m_htmlWnd->Create( m_pPanel, 
                            ID_HTMLWINDOW3, 
                            wxDefaultPosition, 
                            wxSize(400, 340), 
                            wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    itemBoxSizer27->Add(m_htmlWnd, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer23->Add(itemBoxSizer30, 0, wxALIGN_BOTTOM|wxALL, 5);

    m_slowAlgorithm = new wxCheckBox;
    m_slowAlgorithm->Create( m_pPanel, ID_CHECKBOX4, _("Slow"), wxDefaultPosition, wxDefaultSize, 0 );
    m_slowAlgorithm->SetValue(false);
    itemBoxSizer30->Add(m_slowAlgorithm, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText32 = new wxStaticText;
    itemStaticText32->Create( m_pPanel, wxID_STATIC, _("Scan from"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(itemStaticText32, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_ctrlEditFrom = new wxTextCtrl;
    m_ctrlEditFrom->Create( m_pPanel, ID_TEXTCTRL40, _("1"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(m_ctrlEditFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText34 = new wxStaticText;
    itemStaticText34->Create( m_pPanel, wxID_STATIC, _("Scan to"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(itemStaticText34, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_ctrlEditTo = new wxTextCtrl;
    m_ctrlEditTo->Create( m_pPanel, ID_TEXTCTRL, _("255"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(m_ctrlEditTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer30->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 20);

    wxButton* itemButton37 = new wxButton;
    itemButton37->Create( m_pPanel, ID_BUTTON_SCAN, _("Scan"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(itemButton37, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStatusBar* itemStatusBar38 = new wxStatusBar;
    itemStatusBar38->Create( m_pPanel, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar38->SetFieldsCount(2);
    itemBoxSizer20->Add(itemStatusBar38, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Connect events and objects
    m_DeviceTree->Connect(ID_TREE_DEVICE, wxEVT_LEFT_DCLICK, wxMouseEventHandler(frmScanforDevices::OnLeftDClick), NULL, this);

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmScanforDevices::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmScanforDevices::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmScanforDevices::GetIconResource(const wxString& name)
{
    // Icon retrieval
    wxUnusedVar(name);
    if (name == _T("fatbee_v2.ico")) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }
    return wxNullIcon;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enableInterface 
//

bool frmScanforDevices::enableInterface(void)
{
    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Opening interface..."),
                                    100,
                                    this,
                                    wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

    progressDlg.Pulse(_("opening interface..."));

    if ( 0 != m_csw.doCmdOpen() ) {

        progressDlg.Pulse(_("Interface is open."));

        if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

            progressDlg.Pulse( _("Checking if interface is working...") );

            // We use a dll.
            //
            // Verify that we have a connection with the interface
            // We do that by using the status command.
            canalStatus canalstatus;
            if ( CANAL_ERROR_SUCCESS != m_csw.doCmdStatus( &canalstatus ) ) {
                wxString strbuf;
                strbuf.Printf(
                        _("Unable to open interface to driver. errorcode=%lu, suberrorcode=%lu, Description: %s"),
                        canalstatus.lasterrorcode,
                        canalstatus.lasterrorsubcode,
                        canalstatus.lasterrorstr);
                wxLogStatus(strbuf);

                wxMessageBox(_("No response received from interface. May not work correctly!"));

            }

        } 
        else if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {

            //m_staticComboText->SetLabel(_("Interface"));

            progressDlg.Pulse(_("Checking if interface is working..."));

            // Test
            if (CANAL_ERROR_SUCCESS != m_csw.doCmdNOOP()) {
                wxMessageBox(_("Interface test command failed. May not work correctly!"));
            }

            // TCP/IP interface
            progressDlg.Pulse(_("TCP/IP Interface Open"));

            ///////////////////////////////////////////
            // Fill listbox with available interfaces
            ///////////////////////////////////////////

            // Get the interface list
            std::deque<std::string> strarray;

            // Get VSCP interface list
            progressDlg.Pulse(_("TCP/IP Interface Open"));

            m_csw.getTcpIpInterface()->doCmdInterfaceList( strarray );

            if (strarray.size()) {

                //m_comboNodeID->Clear();
                for (unsigned int i = 0; i < strarray.size(); i++) {
                    wxStringTokenizer tkz(strarray[i], _(","));
                    wxString strOrdinal = tkz.GetNextToken();
                    wxString strType = tkz.GetNextToken();
                    wxString strGUID = tkz.GetNextToken();
                    wxString strDescription = tkz.GetNextToken();
                    wxString strLine = strGUID;
                    strLine += _(" ");
                    strLine += _(" Type = ");
                    strLine += strType;
                    strLine += _(" - ");
                    strLine += strDescription;

                }

            } 
            else {
                wxMessageBox(_("No interfaces found!"));
            }

        } // TCP/IP interface

    } 
    else {
        progressDlg.Pulse(_("Failed to open Interface."));
        wxMessageBox(_("Failed to Open Interface."));

        return false;
    }

    // Move to top of zorder
    Raise();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disableInterface
//

bool frmScanforDevices::disableInterface(void)
{
    m_csw.doCmdClose();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemExitClick
//

void frmScanforDevices::OnMenuitemExitClick(wxCommandEvent& event)
{
    Close();
    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmScanforDevices::OnCloseWindow(wxCloseEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpClick
//

void frmScanforDevices::OnMenuitemHelpClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpFaqClick
//

void frmScanforDevices::OnMenuitemHelpFaqClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscp_faq"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpScClick
//

void frmScanforDevices::OnMenuitemHelpScClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpThanksClick
//

void frmScanforDevices::OnMenuitemHelpThanksClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpCreditsClick
//

void frmScanforDevices::OnMenuitemHelpCreditsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpVscpSiteClick
//

void frmScanforDevices::OnMenuitemHelpVscpSiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemHelpAboutClick
//

void frmScanforDevices::OnMenuitemHelpAboutClick(wxCommandEvent& event)
{
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonScanClick
//

void frmScanforDevices::OnButtonScanClick(wxCommandEvent& event)
{
    bool bSlowAlgorithm = false;
    uint8_t val;
    uint8_t reg[256];
    CMDF mdf;
    wxString url;
    wxTreeItemId newitem;

    wxBusyCursor wait;
    
    bSlowAlgorithm = m_slowAlgorithm->GetValue();
    
    uint8_t scanFrom = vscp_readStringValue(m_ctrlEditFrom->GetValue());
    uint8_t scanTo = vscp_readStringValue(m_ctrlEditTo->GetValue());
    
    if ( scanFrom >=  scanTo ) {
        wxMessageBox(_("Node to scan from must be less then to"));
        return;
    }

    m_DeviceTree->DeleteAllItems();
    m_htmlWnd->SetPage( _("<html><body></body></html>") );
    m_htmlWnd->Update();

    wxProgressDialog progressDlg(_("Scanning for VSCP devices"),
            _("Reading Registers"),
            2*(scanTo-scanFrom+1),
            this,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE  | wxPD_CAN_ABORT);

    wxTreeItemId rootItem = m_DeviceTree->AddRoot(_("Found device(s)"));
    m_DeviceTree->ExpandAll();

    // Fetch GUID for the interface
	if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {
		fetchIterfaceGUID();
	}

    if ( bSlowAlgorithm ) {

        for ( int i = scanFrom; i <= scanTo; i++ ) {

            if (!progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i))) {
                if (m_DeviceTree->GetCount()) {
                    wxTreeItemIdValue cookie;
                    wxTreeItemId item = m_DeviceTree->GetFirstChild(m_DeviceTree->GetRootItem(), cookie);
                    if ( item.IsOk() ) m_DeviceTree->SelectItem( item );
                }
                ::wxEndBusyCursor();
                break;
            }

            if ( USE_DLL_INTERFACE == m_csw.getDeviceType() ) {

                // Empty input queue
                canalMsg canalmsg;
                while ( m_csw.getDllInterface()->doCmdDataAvailable() ) {
                    if ( CANAL_ERROR_SUCCESS != m_csw.getDllInterface()->doCmdReceive( &canalmsg ) ) break;
                }

                if ( CANAL_ERROR_SUCCESS == 
                    m_csw.getDllInterface()->readLevel1Register( i, 0, 0xd0, &val ) ) {

                    newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), i));
                    m_DeviceTree->ExpandAll();
                    memset(reg, 0, sizeof(reg));

                    scanElement *pElement = new scanElement;
                    if (NULL != pElement) {
                        pElement->m_bLoaded = false;
                        pElement->m_nodeid = i;
                        //pElement->m_html = str; 
                        memset(pElement->m_reg, 0, 256);
                        m_DeviceTree->SetItemData(newitem, pElement);
                    }
                }

            } 
            else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

                cguid destguid;
                destguid = m_ifguid;
                destguid.setLSB(i);

                // Empty input queue
                m_csw.getTcpIpInterface()->doCmdClear();

                if ( CANAL_ERROR_SUCCESS ==  
                    m_csw.getTcpIpInterface()->readLevel2Register( 0xd0,
                                                                    0,
                                                                    &val,
                                                                    m_ifguid,
                                                                    &destguid ) ) {

                    newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), i));
                    m_DeviceTree->ExpandAll();
                    
                    scanElement *pElement = new scanElement;
                    if (NULL != pElement) {
                        pElement->m_bLoaded = false;
                        pElement->m_nodeid = i;
                        //pElement->m_html = str; 
                        memset(pElement->m_reg, 0, 256);
                        m_DeviceTree->SetItemData(newitem, pElement);
                    }
                     
                }

            }

            ::wxSafeYield();

        } // for
    }
    else { // Fast Algorithm

        vscpEventEx eventex;

		if (USE_DLL_INTERFACE == m_csw.getDeviceType()) {

            // Empty input queue
            canalMsg canalmsg;
            while ( m_csw.getDllInterface()->doCmdDataAvailable() ) {
                if ( CANAL_ERROR_SUCCESS != m_csw.getDllInterface()->doCmdReceive( &canalmsg ) ) break;
            }

			// Send read register to all nodes.
			for ( int i = scanFrom; i <= scanTo; i++ ) {

#ifdef WIN32				
				progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i));
#endif				

				eventex.vscp_class = VSCP_CLASS1_PROTOCOL;
				eventex.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;
				eventex.sizeData = 2;		// nodeid + register to read
				eventex.data[ 0 ] = i;		// nodeid
				eventex.data[ 1 ] = 0xd0;	// Register to read
				m_csw.doCmdSend( &eventex );
                wxMilliSleep( 20 );

			}


			// Check for replies
			wxLongLong resendTime = ::wxGetLocalTimeMillis();
       
			std::list<int> found_list;
			bool bLevel2 = false;
			uint8_t cnt = 0; 
			while (true) {
            
				progressDlg.Pulse( wxString::Format(_("Found %d"), found_list.size()));

                int n;
				while ( ( n = m_csw.doCmdDataAvailable() ) ) {           // Message available

					if ( CANAL_ERROR_SUCCESS == m_csw.doCmdReceive( &eventex ) ) { // Valid event                
#if 0                    
							{
                                wxString str;
                                str = wxString::Format(_("Received Event: count =%d, class=%d type=%d size=%d data= "), 
                                                            n,
                                                            eventex.vscp_class, 
                                                            eventex.vscp_type, 
                                                            eventex.sizeData );
                                for ( int ii = 0; ii < eventex.sizeData; ii++ ) {
                                    str += wxString::Format(_("%02X "), eventex.data[ii] );
                                }
								wxLogDebug(str);
							}
#endif                    
							// Level I Read reply?
							if ( ( VSCP_CLASS1_PROTOCOL == eventex.vscp_class ) &&
									( VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type ) ) {
								if ( 0xd0 == eventex.data[ 0 ] ) { // Requested register?
									// Add nickname to list 
									found_list.push_back( eventex.GUID[15] );
								}
							}

						} // valid event

                    wxYield();    

				} // Event is available

				if ((::wxGetLocalTimeMillis() - resendTime) > 3000 ) {

					// Take away duplicates
					found_list.unique();
                
					wxTreeItemId newitem;
					for( std::list<int>::iterator list_iter = found_list.begin(); 
					        list_iter != found_list.end(); list_iter++) {
                    
						newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), *list_iter));
						m_DeviceTree->ExpandAll();
                    
						scanElement *pElement = new scanElement;
						if (NULL != pElement) {
							pElement->m_bLoaded = false;
							pElement->m_nodeid = *list_iter;
							pElement->m_html = _("Right click on item to load info about node."); 
							memset(pElement->m_reg, 0, 256);
							m_DeviceTree->SetItemData(newitem, pElement);
						}
					}
					break;

				} // Timeout

                wxYield();

			} // while

		
		} // TCP/IP
		else if (USE_TCPIP_INTERFACE == m_csw.getDeviceType()) {

            // Empty input queue
            m_csw.getTcpIpInterface()->doCmdClear();
           
			// Read register at all nodes.
			for ( int i=scanFrom; i<=scanTo; i++ ) {
            
				cguid destguid;
				destguid.setLSB(i);
                
				eventex.head = VSCP_PRIORITY_NORMAL;
				eventex.timestamp = 0;
				eventex.obid = 0;
                
				// Check if a specific interface is used
				if ( !m_ifguid.isNULL() ) {

					progressDlg.Update(i, wxString::Format(_("Checking for device %d"), i));

					eventex.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
					eventex.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

					memset(eventex.GUID, 0, 16);// We use GUID for interface 
					eventex.sizeData = 16 + 2;  // Interface GUID + nodeid + register to read

					m_ifguid.writeGUID(eventex.data);

					eventex.data[ 16 ] = i;     // nodeid
					eventex.data[ 17 ] = 0xd0;  // Register to read

					m_csw.doCmdSend( &eventex );
                    wxMilliSleep( 10 );

				}
                else {
                    wxMessageBox( _("No interface specified. Please select one") );   
                    goto error;
                }

                wxYield();
			
			} // for
                

			// Check for replies
			wxLongLong resendTime = ::wxGetLocalTimeMillis();
       
			std::list<int> found_list;
			bool bLevel2 = false;
			uint8_t cnt = 0; 
			while ( true ) {
            
				progressDlg.Pulse( wxString::Format(_("Found %d"), found_list.size() ) );

                int n;
                while ( n = ( m_csw.doCmdDataAvailable() ) ) {      // Message available

					if ( CANAL_ERROR_SUCCESS == m_csw.doCmdReceive( &eventex ) ) { // Valid event
                    
#if 0                    
							{
							wxString str;
								str = wxString::Format(_("Received Event: count=%d class=%d type=%d size=%d data=%d %d"), 
									n,eventex.vscp_class, eventex.vscp_type, eventex.sizeData, eventex.data[15], eventex.data[16] );
								wxLogDebug(str);
							}
#endif                    

						// Level I Read reply?
						if ( ( VSCP_CLASS1_PROTOCOL == eventex.vscp_class ) &&
								(VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type)) {
							if (eventex.data[ 0 ] == 0xd0) { // Requested register?
								// Add nickname to list 
								found_list.push_back( eventex.GUID[ 15 ] );
							} // Check for correct node
						}                        // Level II 512 Read reply?
						else if ( (VSCP_CLASS2_LEVEL1_PROTOCOL == eventex.vscp_class) &&
							      (VSCP_TYPE_PROTOCOL_RW_RESPONSE == eventex.vscp_type)) {

							if (0xd0 == eventex.data[ 16 ] ) {
                            
								// Add nickname to list 
								found_list.push_back( eventex.GUID[ 15 ] );
							}

						}       // Level II Read reply?
						else if (m_ifguid.isNULL() && bLevel2 &&
								(VSCP_CLASS2_PROTOCOL == eventex.vscp_class) &&
								(VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == 
							eventex.vscp_type)) {

							// from us
							uint32_t retreg = (eventex.data[ 0 ] << 24) +
												(eventex.data[ 1 ] << 16) +
												(eventex.data[ 2 ] << 8) +
												eventex.data[ 3 ];

							// Register we requested?
							if (retreg == 0xffffffd0) {
								// Add nickname to list 
								found_list.push_back( eventex.data[ 15 ] );
							}
						}

					} // valid event

                    wxYield();

				} // while: Data is available

				if ( (::wxGetLocalTimeMillis() - resendTime) > 6000 ) {

					// Take away duplicates
					found_list.unique();
                
					wxTreeItemId newitem;
					for ( std::list<int>::iterator list_iter = found_list.begin(); 
							list_iter != found_list.end(); list_iter++) {
                    
						newitem = m_DeviceTree->AppendItem(rootItem, wxString::Format(_("Node with nickname=%d"), *list_iter));
						m_DeviceTree->ExpandAll();
                    
						scanElement *pElement = new scanElement;
						if (NULL != pElement) {
							pElement->m_bLoaded = false;
							pElement->m_nodeid = *list_iter;
							pElement->m_html = _("Right click on item to load info about node. Double click to open configuration window."); 
							memset(pElement->m_reg, 0, 256);
							m_DeviceTree->SetItemData( newitem, pElement );
						}
					}
					break;

                    wxYield();
            
				} // while
        
                wxYield();

			} // while
        
		} // TCP/IP i/f     
		    
        
    }  // fast

    if ( m_DeviceTree->GetCount() ) {
        m_DeviceTree->SelectItem( m_DeviceTree->GetRootItem() );
    }

error:

    Raise();
    event.Skip(false);
   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTreeDeviceItemRightClick
//

void frmScanforDevices::OnTreeDeviceItemRightClick(wxTreeEvent& event)
{
    // Ask if we should display info
    wxMenu menu;

    menu.Append(Menu_Popup_GetNodeInfo, _T("Update node info from MDF..."));
    menu.Append(Menu_Popup_OpenConfig, _T("Open configuration window..."));
    PopupMenu(&menu);

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnLeftDClick
//

void frmScanforDevices::OnLeftDClick( wxMouseEvent& event )
{
    wxCommandEvent eventDummy;
    openConfiguration( eventDummy );
    event.Skip(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getNodeInfo
//

void frmScanforDevices::getNodeInfo( wxCommandEvent& event )
{
    CMDF mdf;
    wxString url;

    wxBusyCursor wait;

    scanElement *pElement =
            (scanElement *) m_DeviceTree->GetItemData(m_DeviceTree->GetSelection());
    
    if (NULL != pElement) {
        
        wxProgressDialog progressDlg(_("Getting node info."),
                        _("Reading Registers"),
                        256,
                        this,
                        wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE  | wxPD_CAN_ABORT);

		if (INTERFACE_CANAL == m_interfaceType) {
		
            if ( CANAL_ERROR_SUCCESS == 
                m_csw.getDllInterface()->readRegistersfromLevel1Device( pElement->m_nodeid, 
                                                                            0x80,   // strat reg
                                                                            0,      // page
                                                                            128,    // count
                                                                            pElement->m_reg + 0x80 ) ) {
		
			    uint8_t preg_url[33];
		        memset( preg_url, 0, sizeof(preg_url));
			    memcpy( preg_url, pElement->m_reg + 0xe0, 32 );
			    bool bmdf = m_csw.loadMDF( this,
			    	                        preg_url,
				    	                    url,
					    	                &mdf );
                
			    pElement->m_html = vscp_getDeviceHtmlStatusInfo( pElement->m_reg, bmdf ? &mdf : NULL );
			    m_htmlWnd->SetPage(pElement->m_html);
        
			    // Mark as loaded
			    pElement->m_bLoaded = true;

            }
            else {
                wxMessageBox(_("Failed to read registers!"));
            }

		} 
        else if (INTERFACE_VSCP == m_interfaceType) {
        
			cguid destguid;
            destguid = m_ifguid;
			destguid.setNicknameID( pElement->m_nodeid );

            if ( CANAL_ERROR_SUCCESS == 
                    m_csw.getTcpIpInterface()->readLevel2Registers( 0x80,       // reg
                                                                        0,      // Page
												                        128,    // count
                                                                        pElement->m_reg + 0x80,
                                                                        m_ifguid,
												                        &destguid,
												                        false ) ) {
	            uint8_t preg_url[33];
		        memset( preg_url, 0, sizeof(preg_url));
			    memcpy( preg_url, pElement->m_reg + 0xe0, 32 );
			    bool bmdf = m_csw.loadMDF( this,
				                            preg_url,
					                        url,
						                    &mdf );
                
			    pElement->m_html = vscp_getDeviceHtmlStatusInfo( pElement->m_reg, 
                                                                    bmdf ? &mdf : NULL );
			    m_htmlWnd->SetPage(pElement->m_html);
        
			    // Mark as loaded
			    pElement->m_bLoaded = true;

            }
            else {
                wxMessageBox(_("Failed to read registers!"));
            }

		}

	}

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// openConfiguration
//

void frmScanforDevices::openConfiguration( wxCommandEvent& event )
{
    wxBusyCursor wait;
    
    scanElement *pElement =
        ( scanElement * )m_DeviceTree->GetItemData( m_DeviceTree->GetSelection() );

    frmDeviceConfig *subframe = new frmDeviceConfig(this, 2000, _("VSCP Configuration window"));

    if (INTERFACE_CANAL == m_interfaceType) {

        // Hide the Level II checkbox
        subframe->m_bLevel2->Show( false );

        // Init node id combo
        wxRect rc = subframe->m_comboNodeID->GetRect();
#ifdef WIN32						
        rc.SetWidth( 60 );
#else
        rc.SetWidth( 80 );
#endif	
        subframe->m_comboNodeID->SetSize(rc);

        for (int i = 1; i < 256; i++) {
            subframe->m_comboNodeID->Append( wxString::Format(_("0x%02x"), i) );
        }

        subframe->m_comboNodeID->SetValue( _("0x01") );
        subframe->SetTitle( _("Scan for nodes (CANAL) - ") +
                                m_canalif.m_strDescription );

        subframe->m_csw.setInterface( m_canalif.m_strDescription,
                                        m_canalif.m_strPath,
                                        m_canalif.m_strConfig,
                                        m_canalif.m_flags, 
                                        0, 
                                        0 );

        scanElement *pElement =
                (scanElement *) m_DeviceTree->GetItemData( m_DeviceTree->GetSelection() );
        if ( NULL != pElement ) {
            subframe->m_comboNodeID->SetSelection( pElement->m_nodeid - 1 );
        }

        // Close our interface
        m_csw.doCmdClose();

        // Connect to device bus
        subframe->enableInterface();

        // subframe->OnInterfaceActivate( ev );
        wxCommandEvent ev;
        subframe->OnButtonUpdateClick( ev );

        // Move window on top
        subframe->Raise();

        // Show the VSCP configuration windows
        subframe->Show( true );

        // Close the scan window
        Show( false );

    } 
    else if ( INTERFACE_VSCP == m_interfaceType ) {

        wxString str;
        cguid destguid;
        destguid = m_ifguid;
        destguid.setNicknameID( pElement->m_nodeid );

        // Fill the combo
        for (int i = 1; i < 256; i++) {
            cguid guid;

            guid = m_ifguid;
            guid.setNicknameID( i );
            guid.toString( str );
            subframe->m_comboNodeID->Append( str );
        }

        destguid.toString( str );
        subframe->m_comboNodeID->SetValue(str);

        subframe->SetTitle( _("Scan for nodes (TCP/IP)- ") +
                                m_vscpif.m_strDescription);

        // If server username is given and no password is entered we ask for it.
        if (m_vscpif.m_strPassword.IsEmpty() &&
                !m_vscpif.m_strUser.IsEmpty()) {
            m_vscpif.m_strPassword =
                    ::wxGetTextFromUser(_("Please enter password"),
                    _("Connection Test"));
        }

        // Init node id combo
        wxRect rc = subframe->m_comboNodeID->GetRect();
#ifdef WIN32						
        rc.SetWidth( 410 );
#else
        rc.SetWidth( 410 );
#endif						
        subframe->m_comboNodeID->SetSize( rc );

        //subframe->m_csw = m_interfaceType;
        subframe->m_csw.setInterface( m_vscpif.m_strHost,
                                        m_vscpif.m_strUser,
                                        m_vscpif.m_strPassword );
        subframe->m_ifguid = m_ifguid;

        // Close our interface
        m_csw.doCmdClose();

        // Connect to host
        subframe->enableInterface();

        subframe->m_comboNodeID->SetSelection( pElement->m_nodeid - 1 );

        // Overwrite interface GUID to keep the same as in the previous connection
        (void)subframe->m_csw.getTcpIpInterface()->doCmdSetGUID(subframe->m_ifguid);

        // subframe->OnInterfaceActivate( ev );
        wxCommandEvent ev;
        subframe->OnButtonUpdateClick( ev );

        // Move window on top
        subframe->Raise();

        // Show the VSCP configuration windows
        subframe->Show( true );

        // Close the scan window
        Show( false );

    }

    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnTreeDeviceSelChanged
//

void frmScanforDevices::OnTreeDeviceSelChanged(wxTreeEvent& event)
{
    scanElement *pElement = (scanElement *) m_DeviceTree->GetItemData(m_DeviceTree->GetSelection());
    if (NULL != pElement) m_htmlWnd->SetPage(pElement->m_html);
    event.Skip(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW3
//

void frmScanforDevices::OnHtmlwindow3LinkClicked(wxHtmlLinkEvent& event)
{
    if (event.GetLinkInfo().GetHref().StartsWith(_("http://"))) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

bool frmScanforDevices::fetchIterfaceGUID(void) 
{
    wxString str;

    if (!m_csw.isOpen()) {
        wxMessageBox(_("TCP/IP connection to daemon must be open."));
        return false;
    }

    if (USE_TCPIP_INTERFACE != m_csw.getDeviceType()) {
        wxMessageBox(_("Interfaces can only be fetched from the VSCP daemon."));
        return false;
    }

    // Get the interface list
    std::deque<std::string> ifarray;
    if (CANAL_ERROR_SUCCESS ==
            m_csw.getTcpIpInterface()->doCmdInterfaceList( ifarray ) ) {

        if  (ifarray.size() ) {

            for ( unsigned int i = 0; i < ifarray.size(); i++ ) {

                wxStringTokenizer tkz( ifarray[ i ], _(","));
                wxString strOrdinal = tkz.GetNextToken();
                wxString strType = tkz.GetNextToken();
                wxString strIfGUID = tkz.GetNextToken();
                wxString strDescription = tkz.GetNextToken();

                int pos;
                wxString strName;
                if (wxNOT_FOUND != (pos = strDescription.Find(_("|")))) {
                    strName = strDescription.Left(pos);
                    strName.Trim();
                }

                if (strName.Upper() == m_vscpif.m_strInterfaceName.Upper()) {

                    // Save interface GUID;
                    m_ifguid.getFromString(strIfGUID);

                    return true;
                }

            }

        } 
        else {
            wxMessageBox(_("No interfaces found."));
        }
    } 
    else {
        wxMessageBox(_("Unable to get interface list from VSCP daemon."));
    }

    return false;
}
