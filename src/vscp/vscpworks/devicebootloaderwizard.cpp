/////////////////////////////////////////////////////////////////////////////
// Name:        devicebootloaderwizard.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
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
#pragma implementation "devicebootloaderwizard.h"
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

#include "vscpworks.h"
#include "devicebootloaderwizard.h"
#include "devicebootloaderwizard_images.h"
#include "dlgnewvscpsession.h"
#include "bootdevice_vscp.h"
#include "bootdevice_pic1.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard type definition
//

IMPLEMENT_DYNAMIC_CLASS( DeviceBootloaderwizard, wxWizard )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard event table definition
//

BEGIN_EVENT_TABLE( DeviceBootloaderwizard, wxWizard )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard constructors
//

DeviceBootloaderwizard::DeviceBootloaderwizard()
{
    Init();
}

DeviceBootloaderwizard::DeviceBootloaderwizard( wxWindow* parent, 
                                                    wxWindowID id, 
                                                    const wxPoint& pos )
{
    Init();
    Create( parent, id, pos );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard creator
//

bool DeviceBootloaderwizard::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS | wxWIZARD_EX_HELPBUTTON );
    wxBitmap wizardBitmap( GetBitmapResource( wxT( "vscp_logo.jpg" ) ) );
    wxWizard::Create( parent,
                        id,
                        BOOT_LOADER_WIZARD_TITLE,
                        wizardBitmap,
                        pos,
                        wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX );

    CreateControls();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard destructor
//

DeviceBootloaderwizard::~DeviceBootloaderwizard()
{
    if ( NULL != m_pBootCtrl ) {
        delete m_pBootCtrl;
    }

    // Close the interface.
    if ( USE_DLL_INTERFACE == m_iftype ) {
        m_dll.doCmdClose();
    }
    else if ( USE_TCPIP_INTERFACE == m_iftype ) {
        m_tcpip.doCmdClose();
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DeviceBootloaderwizard::Init()
{
    m_pgSelecInterface = NULL;
    m_pgSelecDeviceId = NULL;
    m_pgSelecAlgorithm = NULL;
    m_pgLoadFile = NULL;

    m_bDeviceFound = false;             // No node found
    m_bInterfaceSelected = false;       // No interface selected
    m_bMDFLoaded = false;               // No MDF loaded
    m_bHexFileLoaded = false;           // No firmware file loaded yet

    // Default boot device is none
    m_pBootCtrl = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DeviceBootloaderwizard
//

void DeviceBootloaderwizard::CreateControls()
{
    DeviceBootloaderwizard* itemWizard1 = this;

    WizardPageStart* itemWizardPageSimple2 = new WizardPageStart;
    itemWizardPageSimple2->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( itemWizardPageSimple2 );

    m_pgSelecInterface = new WizardPageSelecInterface;
    m_pgSelecInterface->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( m_pgSelecInterface );

    m_pgSelecDeviceId = new WizardPageSetGUID;
    m_pgSelecDeviceId->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( m_pgSelecDeviceId );

    m_pgSelecAlgorithm = new WizardPageSelectBootloader;
    m_pgSelecAlgorithm->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( m_pgSelecAlgorithm );

    m_pgLoadFile = new WizardPageSelectFirmware;
    m_pgLoadFile->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( m_pgLoadFile );

    WizardPageBootload* itemWizardPageSimple40 = new WizardPageBootload;
    itemWizardPageSimple40->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( itemWizardPageSimple40 );

    WizardPageProgramDevice* itemWizardPageSimple47 = new WizardPageProgramDevice;
    itemWizardPageSimple47->Create( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add( itemWizardPageSimple47 );

    wxWizardPageSimple* lastPage = NULL;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, itemWizardPageSimple2 );
    lastPage = itemWizardPageSimple2;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, m_pgSelecInterface );
    lastPage = m_pgSelecInterface;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, m_pgSelecDeviceId );
    lastPage = m_pgSelecDeviceId;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, m_pgSelecAlgorithm );
    lastPage = m_pgSelecAlgorithm;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, m_pgLoadFile );
    lastPage = m_pgLoadFile;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, itemWizardPageSimple40 );
    lastPage = itemWizardPageSimple40;
    if ( lastPage )
        wxWizardPageSimple::Chain( lastPage, itemWizardPageSimple47 );
    lastPage = itemWizardPageSimple47;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runs the wizard.
//

bool DeviceBootloaderwizard::Run()
{
    //m_bootCtrlPic1.loadIntelHexFile( _("C:\\development\\m2m\\firmware\\pic\\kelvin\\smart\\project\\kelvin_smart2_release_258_reloc.hex") );

    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node ) {
        wxWizardPage* startPage = wxDynamicCast( node->GetData(), wxWizardPage );
        if ( startPage ) return RunWizard( startPage );
        node = node->GetNext();
    }
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool DeviceBootloaderwizard::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap DeviceBootloaderwizard::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    if ( name == wxT( "vscp_logo.jpg" ) ) {
        wxBitmap bitmap( vscp_logo_xpm );
        return bitmap;
    }

    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DeviceBootloaderwizard::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}





//*******************************************************************************************************************
//                                              WizardPageStart
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageStart type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageStart, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage event table definition
//

BEGIN_EVENT_TABLE( WizardPageStart, wxWizardPageSimple )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage constructors
//

WizardPageStart::WizardPageStart()
{
    Init();
}

WizardPageStart::WizardPageStart( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageStart creator
//

bool WizardPageStart::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage destructor
//

WizardPageStart::~WizardPageStart()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageStart::Init()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageStart
//

void WizardPageStart::CreateControls()
{
    WizardPageStart* itemWizardPageSimple2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple2->SetSizer( itemBoxSizer3 );

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemWizardPageSimple2,
                             wxID_STATIC,
                             _( "VSCP Bootloader wizard will now walk you through \nthe steps needed to update the firmware of your \nremote device" ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer3->Add( itemStaticText4, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemWizardPageSimple2,
                             wxID_STATIC,
                             _( "The node you need to update can be located on a VSCP bus \nconnected to a server on a remote location or be connected\nto an interface on your local computer. " ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer3->Add( itemStaticText5, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemWizardPageSimple2, wxID_STATIC,
                             _( "Please don't turn of the power off the remote node \nuntil the firmware update process is completed." ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer3->Add( itemStaticText6, 0, wxALIGN_LEFT | wxALL, 5 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageStart::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageStart::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageStart::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}





//*******************************************************************************************************************
//                                          WizardPageSelecInterface
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageSelecInterface, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface event table definition
//

BEGIN_EVENT_TABLE( WizardPageSelecInterface, wxWizardPageSimple )

EVT_WIZARD_PAGE_CHANGING( -1, WizardPageSelecInterface::OnWizardPageChanging )
EVT_BUTTON( ID_BUTTON20, WizardPageSelecInterface::OnButtonSelectInterfaceClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface constructors
//

WizardPageSelecInterface::WizardPageSelecInterface()
{
    Init();
}

WizardPageSelecInterface::WizardPageSelecInterface( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface creator
//

bool WizardPageSelecInterface::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelecInterface destructor
//

WizardPageSelecInterface::~WizardPageSelecInterface()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageSelecInterface::Init()
{
    m_labelInterfaceSelected = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage1
//

void WizardPageSelecInterface::CreateControls()
{
    WizardPageSelecInterface* itemWizardPageSimple7 = this;

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple7->SetSizer( itemBoxSizer8 );

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemWizardPageSimple7,
                             wxID_STATIC,
                             _( "Select the interface you want to use " ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer8->Add( itemStaticText9, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create( itemWizardPageSimple7,
                              wxID_STATIC,
                              _( "If you want to use a remote VSCP server \nnode you must  select interface and node on that \nserver as well. " ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer8->Add( itemStaticText10, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer8->Add( 5, 80, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_labelInterfaceSelected = new wxStaticText;
    m_labelInterfaceSelected->Create( itemWizardPageSimple7,
                                      wxID_STATIC,
                                      _( "no interface selected" ),
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      0 );
    m_labelInterfaceSelected->SetFont(
        wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Arial Rounded MT Bold" ) ) );
    itemBoxSizer8->Add( m_labelInterfaceSelected, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxButton* itemButton13 = new wxButton;
    itemButton13->Create( itemWizardPageSimple7, ID_BUTTON20, _( "Select interface" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add( itemButton13, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageSelecInterface::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageSelecInterface::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageSelecInterface::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonSelectInterfaceClick
//

void WizardPageSelecInterface::OnButtonSelectInterfaceClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );
    dlg.m_bShowUnconnectedMode = false; // Don't show "unconnected mode"

    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    if ( wxID_OK == dlg.ShowModal() ) { 

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            pblw->m_bInterfaceSelected = true;

            if ( NULL != ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) ) {

                pblw->m_iftype = ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_type;

                if ( ( INTERFACE_CANAL == pblw->m_iftype ) ) {

                    // Save interface parameters
                    pblw->m_canalif.m_strPath =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pcanalif->m_strPath;
                    pblw->m_canalif.m_strConfig =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pcanalif->m_strConfig;
                    pblw->m_canalif.m_strDescription =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pcanalif->m_strDescription;
                    pblw->m_canalif.m_flags =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pcanalif->m_flags;

                    m_labelInterfaceSelected->SetLabel( _( "CANAL - " ) + 
                                                        pblw->m_canalif.m_strDescription );

                    // Init node id combo

                    // Set size of combo
                    wxRect rc = pblw->m_pgSelecDeviceId->m_comboNodeID->GetRect();
                    rc.SetWidth( 60 );
                    pblw->m_pgSelecDeviceId->m_comboNodeID->SetSize( rc );

                    // Empty combo
                    pblw->m_pgSelecDeviceId->m_comboNodeID->Clear();

                    // Write all id values
                    wxArrayString strArray;
                    for ( int i = 1; i < 255; i++ ) {
                        strArray.Add( wxString::Format( _( "0x%02x" ), i ) );
                    }

                    // Add to combo
                    pblw->m_pgSelecDeviceId->m_comboNodeID->Append( strArray );

                    // Select one id
                    pblw->m_pgSelecDeviceId->m_comboNodeID->SetValue( _( "0x01" ) );

                }
                else if ( ( INTERFACE_VSCP == pblw->m_iftype ) ) {

                    wxString str;

                    // Save interface parameters
                    pblw->m_vscpif.m_bLevel2 =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_bLevel2;
                    pblw->m_vscpif.m_strHost =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_strHost;
                    pblw->m_vscpif.m_strUser =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_strUser;
                    pblw->m_vscpif.m_strPassword =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_strPassword;
                    pblw->m_vscpif.m_strInterfaceName =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_strInterfaceName;
                    pblw->m_vscpif.m_strDescription =
                        ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_strDescription;
                    memcpy( pblw->m_vscpif.m_GUID,
                            ( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_GUID, 16 );
                    memcpy( &pblw->m_vscpif.m_vscpfilter,
                            &( ( both_interface * )dlg.m_ctrlListInterfaces->GetClientData( selidx ) )->m_pvscpif->m_vscpfilter, 
                            sizeof( vscpEventFilter ) );
                    // We use the GUID class in further work
                    pblw->m_guid.getFromArray( pblw->m_vscpif.m_GUID  );

                    if ( 0 == pblw->m_vscpif.m_strInterfaceName.Length() ) {
                        wxMessageBox( _( "The connection must have an interface on the VSCP daemon selected" ),
                                      _( "Open new VSCP session" ),
                                      wxICON_STOP );
                        event.Skip();
                        return;
                    }

                    m_labelInterfaceSelected->SetLabel( _( "TCP/IP - " ) + 
                                                        pblw->m_vscpif.m_strDescription );

                    // Combo is filled when next is clicked

                }

            } // VSCP connection

        }
        else {

            pblw->m_bInterfaceSelected = false;

            wxMessageBox( _( "You have to select an interface to connect to!" ),
                          _( "Open new VSCP session" ),
                          wxICON_STOP );

        }

        // Clean up listbox
        dlg.cleanupListbox();

    } // dialog

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void WizardPageSelecInterface::OnWizardPageChanging( wxWizardEvent& event )
{
    wxBusyCursor wait;

    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    // An interface must have been selected to be allowed to continue
    if ( event.GetDirection() ) {

        if ( !( ( DeviceBootloaderwizard * )GetParent() )->m_bInterfaceSelected ) {
            wxMessageBox( _( "An interface must be selected before you can continue!" ) );
            event.Veto();
        }

        // * * * Open the interface * * *
       
        // Get the device nickname/GUID
        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {

            if ( -1 != pblw->m_dll.doCmdOpen( pblw->m_canalif.m_strPath,
                                                pblw->m_canalif.m_strConfig,
                                                pblw->m_canalif.m_flags ) ) {
            }
            else {
                wxMessageBox(_("Unable to open CANAL interface."));
                event.Veto();
            }

        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {

            // Open the interface
            if ( VSCP_ERROR_SUCCESS == 
                pblw->m_tcpip.doCmdOpen( pblw->m_vscpif.m_strHost.ToStdString(),
                                            pblw->m_vscpif.m_strUser.ToStdString(),
                                            pblw->m_vscpif.m_strPassword.ToStdString() ) ) {
            }
            else {
                wxMessageBox( _( "Unable to open TCP/IP interface." ) );

                event.Veto();
                return;
            }

            // Select interface (if one should be selected)
            if ( VSCP_ERROR_SUCCESS ==
                pblw->m_tcpip.fetchIterfaceGUID( pblw->m_vscpif.m_strInterfaceName,
                                                    pblw->m_ifguid ) ) {
                    pblw->m_ifguid.writeGUID( pblw->m_vscpif.m_GUID );
                    pblw->m_guid = pblw->m_ifguid;
            }
            else {
                wxMessageBox( _( "Unable to fetch interfaces from the remote server" ),
                                  _( "Open new VSCP session" ),
                                  wxICON_STOP );
                    
                // Close the connection
                pblw->m_tcpip.doCmdClose();

                event.Veto();
                return;
            }

            // Empty combo
            pblw->m_pgSelecDeviceId->m_comboNodeID->Clear();

            // Fill the combo
            wxString str;
            wxArrayString strings;
            for ( int i = 1; i < 255; i++ ) {
                pblw->m_guid.setLSB( i );
                pblw->m_guid.toString( str );
                strings.Add( str );
            }
            pblw->m_pgSelecDeviceId->m_comboNodeID->Append( strings );

            pblw->m_guid.setLSB( 1 );
            pblw->m_guid.toString( str );
            pblw->m_pgSelecDeviceId->m_comboNodeID->SetValue( str );       

        }

    }

}





//*******************************************************************************************************************
//                                               WizardPageSetGUID
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageSetGUID, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID event table definition
//

BEGIN_EVENT_TABLE( WizardPageSetGUID, wxWizardPageSimple )

EVT_WIZARD_PAGE_CHANGING( -1, WizardPageSetGUID::OnWizardPageChanging )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID constructors
//

WizardPageSetGUID::WizardPageSetGUID()
{
    Init();
}

WizardPageSetGUID::WizardPageSetGUID( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID creator
//

bool WizardPageSetGUID::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSetGUID destructor
//

WizardPageSetGUID::~WizardPageSetGUID()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageSetGUID::Init()
{
    m_comboNodeID = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageSetGUID
//

void WizardPageSetGUID::CreateControls()
{
    WizardPageSetGUID* itemWizardPageSimple14 = this;

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple14->SetSizer( itemBoxSizer15 );

    wxStaticText* itemStaticText16 = new wxStaticText;
    itemStaticText16->Create( itemWizardPageSimple14,
                              wxID_STATIC,
                              _( "Select device to load firmware to" ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer15->Add( itemStaticText16, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText17 = new wxStaticText;
    itemStaticText17->Create( itemWizardPageSimple14,
                              wxID_STATIC,
                              _( "Enter the nickname or the full GUID for the device you want to work with. \n\nA node that is newly loaded with a bootloader is usually at 0xfe, in most other cases a node \nstay at the same id (use scan if unsure of which id a node is at)." ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer15->Add( itemStaticText17, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer15->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxArrayString m_comboNodeIDStrings;
    m_comboNodeID = new wxComboBox;
    m_comboNodeID->Create( itemWizardPageSimple14,
                           ID_COMBOBOX_NODEID,
                           wxEmptyString,
                           wxDefaultPosition,
                           wxSize( 370, -1 ),
                           m_comboNodeIDStrings,
                           wxCB_DROPDOWN );
    if ( WizardPageSetGUID::ShowToolTips() )
        m_comboNodeID->SetToolTip( _( "Set nickname or GUID for node here" ) );
    m_comboNodeID->SetBackgroundColour( wxColour( 255, 255, 210 ) );
    itemBoxSizer15->Add( m_comboNodeID, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer15->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageSetGUID::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageSetGUID::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageSetGUID::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void WizardPageSetGUID::OnWizardPageChanging( wxWizardEvent& event )
{
    wxBusyCursor wait;

    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    // An node must be there to be allowed to continue
    if ( event.GetDirection() ) {  // Forward
        
        // Deault bootloader algorithm
        pblw->m_pgSelecAlgorithm->selectBootLoader( BOOTLOADER_VSCP );

        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {

            // Get Interface id
            uint8_t nodeid = vscp_readStringValue( m_comboNodeID->GetValue() );
            pblw->m_guid.setLSB( nodeid ); // Save for  the future

            if ( ( 0 == nodeid ) || ( nodeid > 254 ) ) {
                wxMessageBox( _( "Invalid Node ID! Must be between 1-254" ) );
                return;
            }

            unsigned char val;
            wxString strTitle;

            if ( CANAL_ERROR_SUCCESS == 
                pblw->m_dll.readLevel1Register( nodeid, 0, 0xd0, &val ) ) {
                pblw->m_bDeviceFound = true;
                strTitle = BOOT_LOADER_WIZARD_TITLE;
                strTitle += _( " - Device found!" );
                pblw->SetTitle( strTitle );
                pblw->m_pgSelecAlgorithm->fetchAlgorithmFromMdf();
            }
            else {
                strTitle = BOOT_LOADER_WIZARD_TITLE;
                strTitle += _( " - Unknown device" );
                wxMessageBox( _( "Device was not found! Check nodeid.\nThis is not a problem if the node is in bootloader mode already." ) );
                pblw->SetTitle( strTitle );
            }

        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {

            // Get the destination GUID
            pblw->m_guid.getFromString( m_comboNodeID->GetValue() );

            unsigned char val;
            wxString strTitle;

            if ( VSCP_ERROR_SUCCESS ==
                 pblw->m_tcpip.readLevel2Register( 0xd0,
                                                    0,      // page
                                                    &val,
                                                    pblw->m_ifguid,
                                                    &pblw->m_guid ) ) {
                pblw->m_bDeviceFound = true;
                strTitle = BOOT_LOADER_WIZARD_TITLE;
                strTitle += _( " - Device found!" );
                pblw->SetTitle( strTitle  );
                pblw->m_pgSelecAlgorithm->fetchAlgorithmFromMdf();
            }
            else {
                wxMessageBox( _( "Device was not found! Check interface GUID + nodeid.\nThis may be no problem if the node is in bootloader mode already." ) );
                strTitle = BOOT_LOADER_WIZARD_TITLE;
                strTitle += _( " - Unknown device" );
                pblw->SetTitle( strTitle );
            }

        }

        if ( !( ( DeviceBootloaderwizard * )GetParent() )->m_bInterfaceSelected ) {
            wxMessageBox( _( "An interface must be selected before you can continue!" ) );
            event.Veto();
        }
        
    }
    else {  // Backwards

        pblw->SetTitle( BOOT_LOADER_WIZARD_TITLE );

        // Close the interface.
        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
            pblw->m_dll.doCmdClose();
        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {
            pblw->m_tcpip.doCmdClose();
        }

    }

}





//*******************************************************************************************************************
//                                          WizardPageSelectBootloader
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectBootloader type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageSelectBootloader, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectBootloader event table definition
//

BEGIN_EVENT_TABLE( WizardPageSelectBootloader, wxWizardPageSimple )

EVT_WIZARD_PAGE_CHANGING( -1, WizardPageSelectBootloader::OnWizardPageChanging )
EVT_CHOICE( ID_CHOICE2, WizardPageSelectBootloader::OnBootLoaderAlgorithmSelected )
EVT_BUTTON( ID_BUTTON_ALGORITHM_FROM_MDF, WizardPageSelectBootloader::OnButtonAlgorithmFromMdfClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectBootloader constructors
//

WizardPageSelectBootloader::WizardPageSelectBootloader()
{
    Init();
}

WizardPageSelectBootloader::WizardPageSelectBootloader( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectBootloader creator
//

bool WizardPageSelectBootloader::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectBootloader destructor
//

WizardPageSelectBootloader::~WizardPageSelectBootloader()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageSelectBootloader::Init()
{
    m_nBootAlgorithm = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageSelectBootloader
//

void WizardPageSelectBootloader::CreateControls()
{
    WizardPageSelectBootloader* itemWizardPageSimple21 = this;

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple21->SetSizer( itemBoxSizer22 );

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemWizardPageSimple21,
                              wxID_STATIC,
                              _( "Select the bootloader algorithm to use " ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer22->Add( itemStaticText23, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPageSimple21,
                              wxID_STATIC,
                              _( "If you load bootloader info from the MDF file the algorithm will \nbe set for you." ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer22->Add( itemStaticText24, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer22->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemWizardPageSimple21,
                              wxID_STATIC,
                              _( "Boot algorithm" ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer22->Add( itemStaticText26, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxArrayString m_nBootAlgorithmStrings;
    m_nBootAlgorithmStrings.Add( _( "VSCP standard algorithm" ) );
    m_nBootAlgorithmStrings.Add( _( "Microchip pic algorithm 1" ) );
    m_nBootAlgorithmStrings.Add( _( "AVR algorithm 1 (not active)" ) );
    m_nBootAlgorithmStrings.Add( _( "ARM algorithm 1 (not active)" ) );
    m_nBootAlgorithm = new wxChoice;
    m_nBootAlgorithm->Create( itemWizardPageSimple21, 
                                ID_CHOICE2, 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                m_nBootAlgorithmStrings, 
                                0 );
    m_nBootAlgorithm->SetStringSelection( _( "VSCP standard algorithm" ) );
    itemBoxSizer22->Add( m_nBootAlgorithm, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer22->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxButton* itemButton29 = new wxButton;
    itemButton29->Create( itemWizardPageSimple21,
                          ID_BUTTON_ALGORITHM_FROM_MDF,
                          _( "Select algorithm from MDF..." ),
                          wxDefaultPosition,
                          wxDefaultSize,
                          0 );
    itemBoxSizer22->Add( itemButton29, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageSelectBootloader::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageSelectBootloader::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageSelectBootloader::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnBootLoaderAlgorithmSelected
//

void WizardPageSelectBootloader::OnBootLoaderAlgorithmSelected( wxCommandEvent& event )
{
    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();
    selectBootLoader( m_nBootAlgorithm->GetSelection() );

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// selectBootLoader
//

void WizardPageSelectBootloader::selectBootLoader( int nBootloader )
{
    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    // Remove previous 
    if ( NULL != pblw->m_pBootCtrl ) {
        delete pblw->m_pBootCtrl;
        pblw->m_pBootCtrl = NULL;
    }

    switch ( nBootloader ) {

        case BOOTLOADER_VSCP: // VSCP Standard algorithm
            if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
                pblw->m_pBootCtrl =
                    new CBootDevice_VSCP( &pblw->m_dll, pblw->m_guid.getLSB(), pblw->m_bDeviceFound );
            }
            else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {
                pblw->m_pBootCtrl =
                    new CBootDevice_VSCP( &pblw->m_tcpip, pblw->m_guid, pblw->m_ifguid, pblw->m_bDeviceFound );
            }
            else {
                pblw->m_pBootCtrl = NULL;
            }
            break;

        case BOOTLOADER_PIC1: // Microchip PIC 1 algorithm
            if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
                pblw->m_pBootCtrl =
                    new CBootDevice_PIC1( &pblw->m_dll, pblw->m_guid.getLSB(), pblw->m_bDeviceFound );
            }
            else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {
                pblw->m_pBootCtrl =
                    new CBootDevice_PIC1( &pblw->m_tcpip, pblw->m_guid, pblw->m_ifguid, pblw->m_bDeviceFound );
            }
            else {
                pblw->m_pBootCtrl = NULL;
            }
            break;

        default:
            pblw->m_pBootCtrl = NULL;
            break;

    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonAlgorithmFromMdfClick
//

void WizardPageSelectBootloader::OnButtonAlgorithmFromMdfClick( wxCommandEvent& event )
{
    fetchAlgorithmFromMdf();
    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// doAlgorithmFromMdfClick
//

void WizardPageSelectBootloader::fetchAlgorithmFromMdf( void )
{
    wxString strID;
    wxString mdfurl;
    bool rv = true;

    wxBusyCursor wait;
    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    // Get selected id
    strID = pblw->m_pgSelecDeviceId->m_comboNodeID->GetValue();

    // Get the device nickname/GUID
    if ( USE_DLL_INTERFACE == pblw->m_iftype ) {

        if ( !pblw->m_dll.getMDFUrlFromLevel1Device( pblw->m_guid.getLSB(), mdfurl ) ) {
            wxMessageBox( _( "Failed to fetch MDF url! \nIs the device active and available?" ) );
            return;
        }

        if ( !pblw->m_mdf.load( mdfurl ) ) {
            wxMessageBox( _( "Failed to load MDF!" ) );
            return;
        }

        // MDF has been fetched
        pblw->m_bMDFLoaded = true;

        // MDF has been fetched - Set algorithm
        m_nBootAlgorithm->SetSelection( pblw->m_mdf.m_bootInfo.m_nAlgorithm );
        selectBootLoader( pblw->m_mdf.m_bootInfo.m_nAlgorithm );

    }
    else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {

        if ( !pblw->m_tcpip.getMDFUrlFromLevel2Device( pblw->m_ifguid, pblw->m_guid, mdfurl ) ) {
            wxMessageBox( _( "Failed to fetch MDF url! \nIs the device active and available?" ) );
            return;
        }

        if ( !pblw->m_mdf.load( mdfurl ) ) {
            wxMessageBox( _( "Failed to load MDF!" ) );
            return;
        }

        // MDF has been fetched -
        pblw->m_bMDFLoaded = true;

        // MDF has been fetched - Set algorithm
        m_nBootAlgorithm->SetSelection( pblw->m_mdf.m_bootInfo.m_nAlgorithm );
        selectBootLoader( pblw->m_mdf.m_bootInfo.m_nAlgorithm );

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void WizardPageSelectBootloader::OnWizardPageChanging( wxWizardEvent& event )
{
    wxBusyCursor wait;

    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    if ( event.GetDirection() ) {  // Forward

        if ( NULL == ( ( DeviceBootloaderwizard * )GetParent() )->m_pBootCtrl ) {
            wxMessageBox( _( "You must select a bootloader algorithm before you can continue!" ) );
            event.Veto();
        }

        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
            ;
        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {
            ;
        }

    }
    else {  // Backwards

        pblw->m_bDeviceFound = false;

        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
            ;
        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {
            ;
        }

    }

    event.Skip();

}




//*******************************************************************************************************************
//                                            WizardPageSelectFirmware 
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageSelectFirmware, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware event table definition
//

BEGIN_EVENT_TABLE( WizardPageSelectFirmware, wxWizardPageSimple )

EVT_WIZARD_PAGE_CHANGING( -1, WizardPageSelectFirmware::OnWizardpagePageChanging )
EVT_BUTTON( ID_BUTTON_LOAD_FILE, WizardPageSelectFirmware::OnButtonChooseFileClick )
EVT_BUTTON( ID_BUTTON_LOAD_FILE_FROM_MDF, WizardPageSelectFirmware::OnButtonLoadFileFromMdfClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware constructors
//

WizardPageSelectFirmware::WizardPageSelectFirmware()
{
    Init();
}

WizardPageSelectFirmware::WizardPageSelectFirmware( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware creator
//

bool WizardPageSelectFirmware::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageSelectFirmware destructor
//

WizardPageSelectFirmware::~WizardPageSelectFirmware()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageSelectFirmware::Init()
{
    m_hexFileInfo = NULL;
    m_selectedFile = NULL;
    m_ctrlBtnLoadFile = NULL;
    m_ctrlBtnLoadFileFromMDF = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageSelectFirmware
//

void WizardPageSelectFirmware::CreateControls()
{
    WizardPageSelectFirmware* itemWizardPageSimple30 = this;

    wxBoxSizer* itemBoxSizer31 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple30->SetSizer( itemBoxSizer31 );

    wxStaticText* itemStaticText32 = new wxStaticText;
    itemStaticText32->Create( itemWizardPageSimple30,
                              wxID_STATIC,
                              _( "Select firmware hex file to use " ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer31->Add( itemStaticText32, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemWizardPageSimple30, wxID_STATIC, _( "File content" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText33->SetFont( wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT( "Tahoma" ) ) );
    itemBoxSizer31->Add( itemStaticText33, 0, wxALIGN_LEFT | wxALL, 5 );

    m_hexFileInfo = new wxHtmlWindow;
    m_hexFileInfo->Create( itemWizardPageSimple30,
                           ID_HTMLWINDOW4,
                           wxDefaultPosition,
                           wxSize( 300, 150 ),
                           wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    itemBoxSizer31->Add( m_hexFileInfo, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer31->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_selectedFile = new wxStaticText;
    m_selectedFile->Create( itemWizardPageSimple30,
                            wxID_STATIC,
                            _( "no file selected yet" ),
                            wxDefaultPosition,
                            wxSize( 250, -1 ),
                            0 );
    m_selectedFile->SetFont(
        wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Arial Rounded MT Bold" ) ) );
    itemBoxSizer31->Add( m_selectedFile, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer31->Add( itemBoxSizer37, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    m_ctrlBtnLoadFile = new wxButton;
    m_ctrlBtnLoadFile->Create( itemWizardPageSimple30,
                               ID_BUTTON_LOAD_FILE,
                               _( "Load local file..." ),
                               wxDefaultPosition,
                               wxDefaultSize,
                               0 );
    itemBoxSizer37->Add( m_ctrlBtnLoadFile, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_ctrlBtnLoadFileFromMDF = new wxButton;
    m_ctrlBtnLoadFileFromMDF->Create( itemWizardPageSimple30,
                                      ID_BUTTON_LOAD_FILE_FROM_MDF,
                                      _( "Load remote file from MDF..." ),
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      0 );
    itemBoxSizer37->Add( m_ctrlBtnLoadFileFromMDF, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageSelectFirmware::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageSelectFirmware::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageSelectFirmware::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonChooseFileClick
//

void WizardPageSelectFirmware::OnButtonChooseFileClick( wxCommandEvent& event )
{
    m_selectedFile->SetLabel( _( "---" ) );

    wxFileDialog *pdlg = new wxFileDialog( this,
                                           _( "Intel HEX file to load" ),
                                           _( "" ),
                                           _( "" ),
                                           _( "*.hex" ) );
    if ( NULL == pdlg ) return;

    if ( pdlg->ShowModal() == wxID_OK ) {

        if ( !( ( DeviceBootloaderwizard * )
            GetParent() )->m_pBootCtrl->loadBinaryFile( pdlg->GetPath().GetData(), HEXFILE_TYPE_INTEL_HEX8 ) ) {

            wxMessageBox( _T( "Failed to load input file.!" ),
                          _T( "ERROR" ),
                          wxICON_WARNING | wxOK, this );
        }
        else {

            // Allow wizard continue
            ( ( DeviceBootloaderwizard * )GetParent() )->m_bHexFileLoaded = true;

            wxString str = pdlg->GetFilename();
            m_selectedFile->SetLabel( pdlg->GetFilename() );
            ( ( DeviceBootloaderwizard * )GetParent() )->m_pBootCtrl->showInfo( m_hexFileInfo );
        }

    }

    pdlg->Destroy();

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonLoadFileFromMdfClick
//

void WizardPageSelectFirmware::OnButtonLoadFileFromMdfClick( wxCommandEvent& event )
{
    wxString strID;
    cguid guid;
    wxString mdfurl;

    wxBusyCursor wait;
    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    if ( NULL == pblw ) return;

    bool rv = true;

    // Get selected id
    strID = pblw->m_pgSelecDeviceId->m_comboNodeID->GetValue();
    guid.getFromString( strID );

    // Check if the mdf has been loaded and load it if not
    if ( !pblw->m_bMDFLoaded ) {

        // Must load MDF

        // Get the device nickname/GUID
        if ( USE_DLL_INTERFACE == pblw->m_iftype ) {
            
            if ( pblw->m_dll.getMDFUrlFromLevel1Device( pblw->m_guid.getLSB(), mdfurl ) ) {

                // Mark MDF has been fetched
                pblw->m_bMDFLoaded = true;

            }
            else {
                wxMessageBox( _( "Failed to fetch MDF! \nIs the device active and available?" ) );
                return;
            }

            // Download the MDF file
            if ( !pblw->m_mdf.downLoadMDF(pblw->m_mdf.m_strURL, mdfurl ) ) {

                wxMessageBox( _( "Failed to download mDF file! [" ) +
                                  pblw->m_mdf.m_strURL +
                                  _( "]" ) );
                return;
            }

            // Parse the file
            if ( pblw->m_mdf.parseMDF( mdfurl ) ) {
                wxMessageBox( _( "Failed to parse file!" ) );
                return;
            }

            if ( !pblw->m_pBootCtrl->loadBinaryFile( pblw->m_mdf.m_firmware.m_strPath,
                HEXFILE_TYPE_INTEL_HEX8 ) ) {

                wxMessageBox( _T( "Failed to load input file.!" ),
                              _T( "ERROR" ),
                              wxICON_WARNING | wxOK,
                              this );
                return;
            }
         
            // Allow wizard continue
            pblw->m_bHexFileLoaded = true;

            m_selectedFile->SetLabel( mdfurl );
            pblw->m_pBootCtrl->showInfo( m_hexFileInfo );
 
        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {

            if ( !pblw->m_tcpip.getMDFUrlFromLevel2Device( pblw->m_ifguid,
                                                            guid,
                                                            mdfurl ) ) {
                wxMessageBox( _( "Failed to fetch MDF! \nIs the device active and available?" ) );
                return;
            }
                    
            // MDF has been fetched 
            pblw->m_bMDFLoaded = true;

            // Download the MDF file
            if ( !pblw->m_mdf.downLoadMDF( pblw->m_mdf.m_strURL, mdfurl ) ) {
                wxMessageBox( _( "Failed to download mDF file! [" ) +
                                  pblw->m_mdf.m_strURL +
                                  _( "]" ) );
                return;
            }

            // Parse the file
            if ( !pblw->m_mdf.parseMDF( mdfurl ) ) {
                wxMessageBox( _( "Failed to parse file!" ) );
                return;
            }

            if ( !pblw->m_pBootCtrl->loadBinaryFile( pblw->m_mdf.m_firmware.m_strPath,
                                                                    HEXFILE_TYPE_INTEL_HEX8 ) ) {

                wxMessageBox( _T( "Failed to load input file.!" ),
                                          _T( "ERROR" ),
                                          wxICON_WARNING | wxOK,
                                          this );
                return;
            }

            // Allow wizard continue
            pblw->m_bHexFileLoaded = true;

            m_selectedFile->SetLabel( mdfurl );
            pblw->m_pBootCtrl->showInfo( m_hexFileInfo );

        }

    }

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardpagePageChanging
//

void WizardPageSelectFirmware::OnWizardpagePageChanging( wxWizardEvent& event )
{
    wxBusyCursor wait;

    // A hex file must have been loaded to be allowed to continue
    if ( event.GetDirection() ) {
        if ( !( ( DeviceBootloaderwizard * )GetParent() )->m_bHexFileLoaded ) {
            wxMessageBox( _( "Firmware code must be loaded before you can continue!" ) );
            event.Veto();
        }
    }

    //event.Skip(); 
}











//*******************************************************************************************************************
//                                           WizardPageBootload
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageBootload type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageBootload, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageBootload event table definition
//

BEGIN_EVENT_TABLE( WizardPageBootload, wxWizardPageSimple )

EVT_WIZARD_PAGE_CHANGING( -1, WizardPageBootload::OnWizardpagePreBootloadPageChanging )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageBootload constructors
//

WizardPageBootload::WizardPageBootload()
{
    Init();
}

WizardPageBootload::WizardPageBootload( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageBootload creator
//

bool WizardPageBootload::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageBootload destructor
//

WizardPageBootload::~WizardPageBootload()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageBootload::Init()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageBootload
//

void WizardPageBootload::CreateControls()
{
    WizardPageBootload* itemWizardPageSimple40 = this;

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple40->SetSizer( itemBoxSizer41 );

    wxStaticText* itemStaticText42 = new wxStaticText;
    itemStaticText42->Create( itemWizardPageSimple40,
                              wxID_STATIC,
                              _( "Ready for the bootloader process" ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer41->Add( itemStaticText42, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText43 = new wxStaticText;
    itemStaticText43->Create( itemWizardPageSimple40,
                              wxID_STATIC,
                              _( "When you click on the next button you will go to the bootloader page\nthat will start loading the firmware file you have selected into the \ndevice of your choice. It is important not to abandon this process \nbefore it has finished.\n\nIf you don't want to continue with the firmware load process \nclick the cancel button." ),
                              wxDefaultPosition,
                              wxDefaultSize,
                              0 );
    itemBoxSizer41->Add( itemStaticText43, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer41->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer41->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxStaticText* itemStaticText46 = new wxStaticText;
    itemStaticText46->Create( itemWizardPageSimple40, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer41->Add( itemStaticText46, 0, wxALIGN_LEFT | wxALL, 5 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageBootload::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageBootload::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageBootload::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardpagePreBootloadPageChanging
//

void WizardPageBootload::OnWizardpagePreBootloadPageChanging( wxWizardEvent& event )
{
    event.Skip();
}





//*******************************************************************************************************************
//                                          WizardPageProgramDevice
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageProgramDevice, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice event table definition
//

BEGIN_EVENT_TABLE( WizardPageProgramDevice, wxWizardPageSimple )

EVT_BUTTON( ID_BUTTON21, WizardPageProgramDevice::OnButtonProgramClick )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice constructors
//

WizardPageProgramDevice::WizardPageProgramDevice()
{
    Init();
}

WizardPageProgramDevice::WizardPageProgramDevice( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice creator
//

bool WizardPageProgramDevice::Create( wxWizard* parent )
{
    wxBitmap wizardBitmap( wxNullBitmap );
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if ( GetSizer() ) {
        GetSizer()->Fit( this );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageProgramDevice destructor
//

WizardPageProgramDevice::~WizardPageProgramDevice()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageProgramDevice::Init()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageProgramDevice
//

void WizardPageProgramDevice::CreateControls()
{
    WizardPageProgramDevice* itemWizardPageSimple47 = this;

    wxBoxSizer* itemBoxSizer48 = new wxBoxSizer( wxVERTICAL );
    itemWizardPageSimple47->SetSizer( itemBoxSizer48 );

    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemWizardPageSimple47, 
                                wxID_STATIC, 
                                _( "Firmware update" ), 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                0 );
    itemBoxSizer48->Add( itemStaticText49, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText50 = new wxStaticText;
    itemStaticText50->Create( itemWizardPageSimple47,
                                wxID_STATIC,
                                _( "Press the program device button to load firmware to the selected device." ),
                                wxDefaultPosition,
                                wxDefaultSize,
                                0 );
    itemBoxSizer48->Add( itemStaticText50, 0, wxALIGN_LEFT | wxALL, 5 );

    itemBoxSizer48->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer48->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer48->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxButton* itemButton54 = new wxButton;
    itemButton54->Create( itemWizardPageSimple47,
                            ID_BUTTON21,
                            _( "Program selected device" ),
                            wxDefaultPosition,
                            wxDefaultSize,
                            0 );
    itemBoxSizer48->Add( itemButton54, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    itemBoxSizer48->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    wxStaticText* itemStaticText56 = new wxStaticText;
    itemStaticText56->Create( itemWizardPageSimple47, 
                                wxID_STATIC, 
                                wxEmptyString, 
                                wxDefaultPosition, 
                                wxDefaultSize, 
                                0 );
    itemBoxSizer48->Add( itemStaticText56, 0, wxALIGN_LEFT | wxALL, 5 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageProgramDevice::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageProgramDevice::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageProgramDevice::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnButtonProgramClick
//

void WizardPageProgramDevice::OnButtonProgramClick( wxCommandEvent& event )
{
    wxString strID;
    wxBusyCursor busy;

    DeviceBootloaderwizard *pblw = ( DeviceBootloaderwizard * )GetParent();

    // Get the device nickname/GUID
    if ( USE_DLL_INTERFACE == pblw->m_iftype ) {

        if ( pblw->m_bDeviceFound ) {
            if ( !pblw->m_pBootCtrl->setDeviceInBootMode() ) {
                wxMessageBox( _( "Failed to set device in boot mode! \nWill still try to load firmware." ) );
            }
        }

        if ( !pblw->m_pBootCtrl->doFirmwareLoad() ) {
            wxMessageBox( _( "Failed to load firmware code into device!" ) );
        }

    }
    else if ( USE_TCPIP_INTERFACE == pblw->m_iftype ) {

        pblw->m_tcpip.setResponseTimeout( 5 );  // Extend timeout time.
        
        if ( pblw->m_bDeviceFound ) {
            if ( !pblw->m_pBootCtrl->setDeviceInBootMode() ) {
                wxMessageBox( _( "Failed to set device in boot mode! \nWill still try to load firmware." ) );
            }
        }

        if ( !pblw->m_pBootCtrl->doFirmwareLoad() ) {
            wxMessageBox( _( "Failed to load firmware code into device!" ) );
        }

    }

    event.Skip();
}







