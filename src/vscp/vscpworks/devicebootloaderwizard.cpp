/////////////////////////////////////////////////////////////////////////////
// Name:        devicebootloaderwizard.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2007-2014 
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
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "devicebootloaderwizard.h"
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

#include "vscpworks.h"
#include "devicebootloaderwizard.h"
#include "dlgnewvscpsession.h"
#include "bootdevice_vscp.h"
#include "bootdevice_pic1.h"

////@begin XPM images
////@end XPM images


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard type definition
//

IMPLEMENT_DYNAMIC_CLASS( DeviceBootloaderwizard, wxWizard )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard event table definition
//

BEGIN_EVENT_TABLE( DeviceBootloaderwizard, wxWizard )

////@begin DeviceBootloaderwizard event table entries
////@end DeviceBootloaderwizard event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard constructors
//

DeviceBootloaderwizard::DeviceBootloaderwizard()
{
    Init();
}

DeviceBootloaderwizard::DeviceBootloaderwizard( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Init();
    Create(parent, id, pos);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard creator
//

bool DeviceBootloaderwizard::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    ////@begin DeviceBootloaderwizard creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS|wxWIZARD_EX_HELPBUTTON);
  wxBitmap wizardBitmap(GetBitmapResource(wxT("../../../docs/vscp/logo/vscp_logo.jpg")));
  wxWizard::Create( parent, id, _("VSCP  Bootloader Wizard"), wizardBitmap, pos, wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

  CreateControls();
    ////@end DeviceBootloaderwizard creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceBootloaderwizard destructor
//

DeviceBootloaderwizard::~DeviceBootloaderwizard()
{
    ////@begin DeviceBootloaderwizard destruction
    ////@end DeviceBootloaderwizard destruction

    if ( NULL != m_pBootCtrl ) {
        delete m_pBootCtrl;
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void DeviceBootloaderwizard::Init()
{
    ////@begin DeviceBootloaderwizard member initialisation
  m_pgSelecInterface = NULL;
  m_pgSelecDeviceId = NULL;
  m_pgSelecAlgorithm = NULL;
  m_pgLoadFile = NULL;
    ////@end DeviceBootloaderwizard member initialisation
    
    m_bInterfaceSelected = false; // No interface selected
    m_bMDFLoaded = false;         // No MDF loaded
    m_bHexFileLoaded = false;     // No firmware file loaded yet

    // Default boot device is VSCP
    m_pBootCtrl = new CBootDevice_VSCP( &m_csw, m_guid ); 

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for DeviceBootloaderwizard
//

void DeviceBootloaderwizard::CreateControls()
{    
    ////@begin DeviceBootloaderwizard content construction
  DeviceBootloaderwizard* itemWizard1 = this;

  WizardPage* itemWizardPageSimple2 = new WizardPage;
  itemWizardPageSimple2->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple2);

  m_pgSelecInterface = new WizardPage1;
  m_pgSelecInterface->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(m_pgSelecInterface);

  m_pgSelecDeviceId = new WizardPage6;
  m_pgSelecDeviceId->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(m_pgSelecDeviceId);

  m_pgSelecAlgorithm = new WizardPage3;
  m_pgSelecAlgorithm->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(m_pgSelecAlgorithm);

  m_pgLoadFile = new WizardPage2;
  m_pgLoadFile->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(m_pgLoadFile);

  WizardPage5* itemWizardPageSimple40 = new WizardPage5;
  itemWizardPageSimple40->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple40);

  WizardPage7* itemWizardPageSimple47 = new WizardPage7;
  itemWizardPageSimple47->Create( itemWizard1 );
  itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple47);

  wxWizardPageSimple* lastPage = NULL;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple2);
  lastPage = itemWizardPageSimple2;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, m_pgSelecInterface);
  lastPage = m_pgSelecInterface;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, m_pgSelecDeviceId);
  lastPage = m_pgSelecDeviceId;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, m_pgSelecAlgorithm);
  lastPage = m_pgSelecAlgorithm;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, m_pgLoadFile);
  lastPage = m_pgLoadFile;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple40);
  lastPage = itemWizardPageSimple40;
  if (lastPage)
    wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple47);
  lastPage = itemWizardPageSimple47;
    ////@end DeviceBootloaderwizard content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runs the wizard.
//

bool DeviceBootloaderwizard::Run()
{

    //m_bootCtrlPic1.loadIntelHexFile( _("C:\\development\\m2m\\firmware\\pic\\kelvin\\smart\\project\\kelvin_smart2_release_258_reloc.hex") );

    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while (node)
    {
        wxWizardPage* startPage = wxDynamicCast(node->GetData(), wxWizardPage);
        if (startPage) return RunWizard(startPage);
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
    ////@begin DeviceBootloaderwizard bitmap retrieval
  wxUnusedVar(name);
  if (name == wxT("../../../docs/vscp/logo/vscp_logo.jpg"))
  {
    wxBitmap bitmap(vscp_logo_xpm);
    return bitmap;
  }
  return wxNullBitmap;
    ////@end DeviceBootloaderwizard bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon DeviceBootloaderwizard::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin DeviceBootloaderwizard icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end DeviceBootloaderwizard icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage event table definition
//

BEGIN_EVENT_TABLE( WizardPage, wxWizardPageSimple )

////@begin WizardPage event table entries
////@end WizardPage event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage constructors
//

WizardPage::WizardPage()
{
    Init();
}

WizardPage::WizardPage( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage creator
//

bool WizardPage::Create( wxWizard* parent )
{
    ////@begin WizardPage creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage destructor
//

WizardPage::~WizardPage()
{
    ////@begin WizardPage destruction
    ////@end WizardPage destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage::Init()
{
    ////@begin WizardPage member initialisation
    ////@end WizardPage member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage
//

void WizardPage::CreateControls()
{    
    ////@begin WizardPage content construction
  WizardPage* itemWizardPageSimple2 = this;

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple2->SetSizer(itemBoxSizer3);

  wxStaticText* itemStaticText4 = new wxStaticText;
  itemStaticText4->Create( itemWizardPageSimple2, wxID_STATIC, _("VSCP Bootloader wizard will now walk you through \nthe steps needed to update the firmware of your \nremote device"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText;
  itemStaticText5->Create( itemWizardPageSimple2, wxID_STATIC, _("The node you need to update can be located on a VSCP bus \nconnected to a server on a remote location or be connected\nto an interface on your local computer. "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText6 = new wxStaticText;
  itemStaticText6->Create( itemWizardPageSimple2, wxID_STATIC, _("Please don't turn of the power off the remote node \nuntil the firmware update process is completed."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 5);

    ////@end WizardPage content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage1 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage1, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage1 event table definition
//

BEGIN_EVENT_TABLE( WizardPage1, wxWizardPageSimple )

////@begin WizardPage1 event table entries
  EVT_WIZARD_PAGE_CHANGING( -1, WizardPage1::OnWizardpageSelectInterfacePageChanging )
  EVT_BUTTON( ID_BUTTON20, WizardPage1::OnButtonSelectInterfaceClick )
////@end WizardPage1 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage1 constructors
//

WizardPage1::WizardPage1()
{
    Init();
}

WizardPage1::WizardPage1( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage1 creator
//

bool WizardPage1::Create( wxWizard* parent )
{
    ////@begin WizardPage1 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage1 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage1 destructor
//

WizardPage1::~WizardPage1()
{
    ////@begin WizardPage1 destruction
    ////@end WizardPage1 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage1::Init()
{
    ////@begin WizardPage1 member initialisation
  m_labelInterfaceSelected = NULL;
    ////@end WizardPage1 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage1
//

void WizardPage1::CreateControls()
{    
    ////@begin WizardPage1 content construction
  WizardPage1* itemWizardPageSimple7 = this;

  wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple7->SetSizer(itemBoxSizer8);

  wxStaticText* itemStaticText9 = new wxStaticText;
  itemStaticText9->Create( itemWizardPageSimple7, wxID_STATIC, _("Select the interface you want to use "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText10 = new wxStaticText;
  itemStaticText10->Create( itemWizardPageSimple7, wxID_STATIC, _("If you want to use a remote VSCP server connected \nnode you must  select interface and node on that \nserver as well. "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer8->Add(5, 80, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_labelInterfaceSelected = new wxStaticText;
  m_labelInterfaceSelected->Create( itemWizardPageSimple7, wxID_STATIC, _("no interface selected"), wxDefaultPosition, wxDefaultSize, 0 );
  m_labelInterfaceSelected->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial Rounded MT Bold")));
  itemBoxSizer8->Add(m_labelInterfaceSelected, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton13 = new wxButton;
  itemButton13->Create( itemWizardPageSimple7, ID_BUTTON20, _("Select interface"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer8->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    ////@end WizardPage1 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage1::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage1::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage1 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage1 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage1::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage1 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage1 icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage2 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage2, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage2 event table definition
//

BEGIN_EVENT_TABLE( WizardPage2, wxWizardPageSimple )

////@begin WizardPage2 event table entries
  EVT_WIZARD_PAGE_CHANGING( -1, WizardPage2::OnWizardpage3PageChanging )
  EVT_BUTTON( ID_BUTTON_LOAD_FILE, WizardPage2::OnButtonChooseFileClick )
  EVT_BUTTON( ID_BUTTON_LOAD_FILE_FROM_MDF, WizardPage2::OnButtonLoadFileFromMdfClick )
////@end WizardPage2 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage2 constructors
//

WizardPage2::WizardPage2()
{
    Init();
}

WizardPage2::WizardPage2( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage2 creator
//

bool WizardPage2::Create( wxWizard* parent )
{
    ////@begin WizardPage2 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage2 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage2 destructor
//

WizardPage2::~WizardPage2()
{
    ////@begin WizardPage2 destruction
    ////@end WizardPage2 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage2::Init()
{
    ////@begin WizardPage2 member initialisation
  m_hexFileInfo = NULL;
  m_selectedFile = NULL;
  m_ctrlBtnLoadFile = NULL;
  m_ctrlBtnLoadFileFromMDF = NULL;
    ////@end WizardPage2 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage2
//

void WizardPage2::CreateControls()
{    
    ////@begin WizardPage2 content construction
  WizardPage2* itemWizardPageSimple30 = this;

  wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple30->SetSizer(itemBoxSizer31);

  wxStaticText* itemStaticText32 = new wxStaticText;
  itemStaticText32->Create( itemWizardPageSimple30, wxID_STATIC, _("Select firmware hex file to use "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer31->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText33 = new wxStaticText;
  itemStaticText33->Create( itemWizardPageSimple30, wxID_STATIC, _("File content"), wxDefaultPosition, wxDefaultSize, 0 );
  itemStaticText33->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
  itemBoxSizer31->Add(itemStaticText33, 0, wxALIGN_LEFT|wxALL, 5);

  m_hexFileInfo = new wxHtmlWindow;
  m_hexFileInfo->Create( itemWizardPageSimple30, ID_HTMLWINDOW4, wxDefaultPosition, wxSize(300, 150), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
  itemBoxSizer31->Add(m_hexFileInfo, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer31->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_selectedFile = new wxStaticText;
  m_selectedFile->Create( itemWizardPageSimple30, wxID_STATIC, _("no file selected yet"), wxDefaultPosition, wxSize(250, -1), 0 );
  m_selectedFile->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial Rounded MT Bold")));
  itemBoxSizer31->Add(m_selectedFile, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer31->Add(itemBoxSizer37, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  m_ctrlBtnLoadFile = new wxButton;
  m_ctrlBtnLoadFile->Create( itemWizardPageSimple30, ID_BUTTON_LOAD_FILE, _("Load local file..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer37->Add(m_ctrlBtnLoadFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  m_ctrlBtnLoadFileFromMDF = new wxButton;
  m_ctrlBtnLoadFileFromMDF->Create( itemWizardPageSimple30, ID_BUTTON_LOAD_FILE_FROM_MDF, _("Load remote file from MDF..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer37->Add(m_ctrlBtnLoadFileFromMDF, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    ////@end WizardPage2 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage2::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage2::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage2 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage2 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage2::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage2 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage2 icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage3 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage3, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage3 event table definition
//

BEGIN_EVENT_TABLE( WizardPage3, wxWizardPageSimple )

////@begin WizardPage3 event table entries
  EVT_WIZARD_PAGE_CHANGING( -1, WizardPage3::OnWizardpage2PageChanging )
  EVT_CHOICE( ID_CHOICE2, WizardPage3::OnBootLoaderAlgorithmSelected )
  EVT_BUTTON( ID_BUTTON_ALGORITHM_FROM_MDF, WizardPage3::OnButtonAlgorithmFromMdfClick )
////@end WizardPage3 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage3 constructors
//

WizardPage3::WizardPage3()
{
    Init();
}

WizardPage3::WizardPage3( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage3 creator
//

bool WizardPage3::Create( wxWizard* parent )
{
    ////@begin WizardPage3 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage3 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage3 destructor
//

WizardPage3::~WizardPage3()
{
    ////@begin WizardPage3 destruction
    ////@end WizardPage3 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage3::Init()
{
    ////@begin WizardPage3 member initialisation
  m_nBootAlgorithm = NULL;
    ////@end WizardPage3 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage3
//

void WizardPage3::CreateControls()
{    
    ////@begin WizardPage3 content construction
  WizardPage3* itemWizardPageSimple21 = this;

  wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple21->SetSizer(itemBoxSizer22);

  wxStaticText* itemStaticText23 = new wxStaticText;
  itemStaticText23->Create( itemWizardPageSimple21, wxID_STATIC, _("Select the bootloader algorithm to use "), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText24 = new wxStaticText;
  itemStaticText24->Create( itemWizardPageSimple21, wxID_STATIC, _("If you load bootloader info from the MDF file the algorithm will \nbe set for you."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer22->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText26 = new wxStaticText;
  itemStaticText26->Create( itemWizardPageSimple21, wxID_STATIC, _("Boot algorithm"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemStaticText26, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString m_nBootAlgorithmStrings;
  m_nBootAlgorithmStrings.Add(_("VSCP standard algorithm"));
  m_nBootAlgorithmStrings.Add(_("Microchip pic algorith 1"));
  m_nBootAlgorithm = new wxChoice;
  m_nBootAlgorithm->Create( itemWizardPageSimple21, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, m_nBootAlgorithmStrings, 0 );
  m_nBootAlgorithm->SetStringSelection(_("VSCP standard algorithm"));
  itemBoxSizer22->Add(m_nBootAlgorithm, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer22->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton29 = new wxButton;
  itemButton29->Create( itemWizardPageSimple21, ID_BUTTON_ALGORITHM_FROM_MDF, _("Select algorithm from MDF..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer22->Add(itemButton29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    ////@end WizardPage3 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage3::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage3::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage3 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage3 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage3::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage3 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage3 icon retrieval
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage5 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage5, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage5 event table definition
//

BEGIN_EVENT_TABLE( WizardPage5, wxWizardPageSimple )

////@begin WizardPage5 event table entries
  EVT_WIZARD_PAGE_CHANGING( -1, WizardPage5::OnWizardpagePreBootloadPageChanging )
////@end WizardPage5 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage5 constructors
//

WizardPage5::WizardPage5()
{
    Init();
}

WizardPage5::WizardPage5( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage5 creator
//

bool WizardPage5::Create( wxWizard* parent )
{
    ////@begin WizardPage5 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage5 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage5 destructor
//

WizardPage5::~WizardPage5()
{
    ////@begin WizardPage5 destruction
    ////@end WizardPage5 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage5::Init()
{
    ////@begin WizardPage5 member initialisation
    ////@end WizardPage5 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage5
//

void WizardPage5::CreateControls()
{    
    ////@begin WizardPage5 content construction
  WizardPage5* itemWizardPageSimple40 = this;

  wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple40->SetSizer(itemBoxSizer41);

  wxStaticText* itemStaticText42 = new wxStaticText;
  itemStaticText42->Create( itemWizardPageSimple40, wxID_STATIC, _("Ready for the bootloader process"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer41->Add(itemStaticText42, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText43 = new wxStaticText;
  itemStaticText43->Create( itemWizardPageSimple40, wxID_STATIC, _("When you click on the next button you will go to the bootloader page\nthat will start loading the firmware file you have selected into the \ndevice of your choice. It is important not to abandon this process \nbefore it has finished.\n\nIf you don't want to continue with the firmware load process \nclick the cancel button."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer41->Add(itemStaticText43, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer41->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer41->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText46 = new wxStaticText;
  itemStaticText46->Create( itemWizardPageSimple40, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer41->Add(itemStaticText46, 0, wxALIGN_LEFT|wxALL, 5);

    ////@end WizardPage5 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage5::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage5::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage5 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage5 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage5::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage5 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage5 icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage6 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage6, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage6 event table definition
//

BEGIN_EVENT_TABLE( WizardPage6, wxWizardPageSimple )

////@begin WizardPage6 event table entries
////@end WizardPage6 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage6 constructors
//

WizardPage6::WizardPage6()
{
    Init();
}

WizardPage6::WizardPage6( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage6 creator
//

bool WizardPage6::Create( wxWizard* parent )
{
    ////@begin WizardPage6 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage6 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage6 destructor
//

WizardPage6::~WizardPage6()
{
    ////@begin WizardPage6 destruction
    ////@end WizardPage6 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage6::Init()
{
    ////@begin WizardPage6 member initialisation
  m_comboNodeID = NULL;
    ////@end WizardPage6 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage6
//

void WizardPage6::CreateControls()
{    
    ////@begin WizardPage6 content construction
  WizardPage6* itemWizardPageSimple14 = this;

  wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple14->SetSizer(itemBoxSizer15);

  wxStaticText* itemStaticText16 = new wxStaticText;
  itemStaticText16->Create( itemWizardPageSimple14, wxID_STATIC, _("Select device to bootload"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText17 = new wxStaticText;
  itemStaticText17->Create( itemWizardPageSimple14, wxID_STATIC, _("Enter the nickname or the full GUID for the device you want to work with"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxArrayString m_comboNodeIDStrings;
  m_comboNodeID = new wxComboBox;
  m_comboNodeID->Create( itemWizardPageSimple14, ID_COMBOBOX_NODEID, wxEmptyString, wxDefaultPosition, wxSize(370, -1), m_comboNodeIDStrings, wxCB_DROPDOWN );
  if (WizardPage6::ShowToolTips())
    m_comboNodeID->SetToolTip(_("Set nickname or GUID for node here"));
  m_comboNodeID->SetBackgroundColour(wxColour(255, 255, 210));
  itemBoxSizer15->Add(m_comboNodeID, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    ////@end WizardPage6 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage6::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage6::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage6 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage6 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage6::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage6 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage6 icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage7 type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPage7, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage7 event table definition
//

BEGIN_EVENT_TABLE( WizardPage7, wxWizardPageSimple )

////@begin WizardPage7 event table entries
  EVT_BUTTON( ID_BUTTON21, WizardPage7::OnButtonProgramClick )
////@end WizardPage7 event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage7 constructors
//

WizardPage7::WizardPage7()
{
    Init();
}

WizardPage7::WizardPage7( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage7 creator
//

bool WizardPage7::Create( wxWizard* parent )
{
    ////@begin WizardPage7 creation
  wxBitmap wizardBitmap(wxNullBitmap);
  wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

  CreateControls();
  if (GetSizer())
    GetSizer()->Fit(this);
    ////@end WizardPage7 creation
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage7 destructor
//

WizardPage7::~WizardPage7()
{
    ////@begin WizardPage7 destruction
    ////@end WizardPage7 destruction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPage7::Init()
{
    ////@begin WizardPage7 member initialisation
    ////@end WizardPage7 member initialisation
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPage7
//

void WizardPage7::CreateControls()
{    
    ////@begin WizardPage7 content construction
  WizardPage7* itemWizardPageSimple47 = this;

  wxBoxSizer* itemBoxSizer48 = new wxBoxSizer(wxVERTICAL);
  itemWizardPageSimple47->SetSizer(itemBoxSizer48);

  wxStaticText* itemStaticText49 = new wxStaticText;
  itemStaticText49->Create( itemWizardPageSimple47, wxID_STATIC, _("Firmware update"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer48->Add(itemStaticText49, 0, wxALIGN_LEFT|wxALL, 5);

  wxStaticText* itemStaticText50 = new wxStaticText;
  itemStaticText50->Create( itemWizardPageSimple47, wxID_STATIC, _("Press the program device button to load firmware to the selected device."), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer48->Add(itemStaticText50, 0, wxALIGN_LEFT|wxALL, 5);

  itemBoxSizer48->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer48->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer48->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton54 = new wxButton;
  itemButton54->Create( itemWizardPageSimple47, ID_BUTTON21, _("Program selected device"), wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer48->Add(itemButton54, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  itemBoxSizer48->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxStaticText* itemStaticText56 = new wxStaticText;
  itemStaticText56->Create( itemWizardPageSimple47, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
  itemBoxSizer48->Add(itemStaticText56, 0, wxALIGN_LEFT|wxALL, 5);

    ////@end WizardPage7 content construction
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPage7::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPage7::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin WizardPage7 bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
    ////@end WizardPage7 bitmap retrieval
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPage7::GetIconResource( const wxString& name )
{
    // Icon retrieval
    ////@begin WizardPage7 icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
    ////@end WizardPage7 icon retrieval
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON20
//

void WizardPage1::OnButtonSelectInterfaceClick( wxCommandEvent& event )
{
    int selidx = -1;
    dlgNewVSCPSession dlg( this );

    if ( wxID_OK == dlg.ShowModal() ) {

        if ( wxNOT_FOUND != ( selidx = dlg.m_ctrlListInterfaces->GetSelection() ) ) {

            if ( 0 != selidx ) {

                ((DeviceBootloaderwizard *)GetParent())->m_bInterfaceSelected = true;

                both_interface *pBoth = 
                    (both_interface *)dlg.m_ctrlListInterfaces->GetClientData( selidx );

                if ( NULL != pBoth ) {  

                    if ( INTERFACE_CANAL == pBoth->m_type ) {

                        m_labelInterfaceSelected->SetLabel( 
                            _("CANAL - ") + 
                            pBoth->m_pcanalif->m_strDescription );

                        // Init node id combo

                        // Set size of combo
                        wxRect rc = ((DeviceBootloaderwizard *)GetParent())->
                            m_pgSelecDeviceId->m_comboNodeID->GetRect();
                        rc.SetWidth( 60 );
                        ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->SetSize( rc );

                        // Write all id values
                        for ( int i=1; i<256; i++ ) {
                            ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->Append( wxString::Format(_("0x%02x"), i));
                        }

                        // Select one id
                        ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->SetValue(_("0x01"));

                        // Set the selected interface
                        ((DeviceBootloaderwizard *)GetParent())->m_csw.setInterface( pBoth->m_pcanalif->m_strDescription,
                            pBoth->m_pcanalif->m_strPath,
                            pBoth->m_pcanalif->m_strConfig,
                            pBoth->m_pcanalif->m_flags, 0, 0 );

                    }
                    else if ( ( INTERFACE_VSCP == pBoth->m_type ) && 
                        ( NULL != pBoth->m_pcanalif ) ) {

                            wxString str;
                            unsigned char GUID[16];
                            memcpy( GUID, pBoth->m_pvscpif->m_GUID, 16 );

                            m_labelInterfaceSelected->SetLabel( 
                                _("TCP/IP - ") + 
                                pBoth->m_pvscpif->m_strDescription );

                            // Fill the combo
                            for ( int i=1; i<256; i++ ) {
                                GUID[0] = i;
                                vscp_writeGuidArrayToString( GUID, str );
                                ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->Append( str );
                            }

                            GUID[0] = 0x01;
                            vscp_writeGuidArrayToString( GUID, str );
                            ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->SetValue( str );

                            // Set the selected interface
                            ((DeviceBootloaderwizard *)GetParent())->m_csw.setInterface( pBoth->m_pvscpif->m_strHost,
                                pBoth->m_pvscpif->m_port,
                                pBoth->m_pvscpif->m_strUser,
                                pBoth->m_pvscpif->m_strPassword );

                    }

                    /*
                    if ( INTERFACE_VSCP == subframe->m_CtrlObject.m_interfaceType ) {

                    // If server username is given and no password is entered we ask for it.
                    if ( subframe->m_CtrlObject.m_ifVSCP.m_strPassword.IsEmpty() && 
                    !subframe->m_CtrlObject.m_ifVSCP.m_strUser.IsEmpty() ) {
                    subframe->m_CtrlObject.m_ifVSCP.m_strPassword = 
                    ::wxGetTextFromUser( _("Please enter passeword"), 
                    _("Connection Test") );
                    }

                    }
                    */


                } // VSCP connection

            } // 0 == selindex
            else {
                /*
                subframe->m_BtnActivateInterface->Show( false );
                subframe->SetTitle(_("VSCP Session - Unconnected Mode"));
                subframe->Show( true );
                */
            }
        }
        else {
            
            ((DeviceBootloaderwizard *)GetParent())->m_bInterfaceSelected = false;

            wxMessageBox(_("You have to select an interface to connect to!"),
                _("Open new VSCP session"), 
                wxICON_STOP );
        }

        // Clean up listbox
        dlg.cleanupListbox();

    } // dialog

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CHOOSE_FILE
//

void WizardPage2::OnButtonChooseFileClick( wxCommandEvent& event )
{
    m_selectedFile->SetLabel( _("---") );

    wxFileDialog *pdlg = new wxFileDialog( this,
                                            _("Intel HEX file to load"),
                                            _(""),
                                            _(""),
                                            _("*.hex") );
    if ( NULL == pdlg ) return;

    if ( pdlg->ShowModal() == wxID_OK ) {

        if ( !((DeviceBootloaderwizard *)
            GetParent())->m_pBootCtrl->loadBinaryFile( pdlg->GetPath().GetData(), HEXFILE_TYPE_INTEL_HEX8 ) ) {

                wxMessageBox( _T("Failed to load input file.!"), 
                    _T("ERROR"),
                    wxICON_WARNING | wxOK, this );
        }
        else {
            
            // Allow wizard continue
            ((DeviceBootloaderwizard *)GetParent())->m_bHexFileLoaded = true;

            wxString str = pdlg->GetFilename();
            m_selectedFile->SetLabel( pdlg->GetFilename() );
            ((DeviceBootloaderwizard *)GetParent())->
                m_pBootCtrl->showInfo( m_hexFileInfo );
        }

    }

    pdlg->Destroy(); 
    //delete pdlg;

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE1
//

void WizardPage1::OnWizardpageSelectInterfacePageChanging( wxWizardEvent& event )
{
    // An interface must have been selected to be allowed to continue
    if ( event.GetDirection() ) {
        if ( !((DeviceBootloaderwizard *)GetParent())->m_bInterfaceSelected ) {
            wxMessageBox(_("An interface must be selected before you can continue!"));
            event.Veto();
        }
    }
    //event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE2
//

void WizardPage2::OnWizardpage3PageChanging( wxWizardEvent& event )
{
    // A hex file must have been loaded to be allowed to continue
    if ( event.GetDirection() ) {
        if ( !((DeviceBootloaderwizard *)GetParent())->m_bHexFileLoaded ) {
            wxMessageBox(_("Firmware code must be loaded before you can continue!"));
            event.Veto();
        }
    }
    //event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE3
//

void WizardPage3::OnWizardpage2PageChanging( wxWizardEvent& event )
{

    event.Skip(); 
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE2
//

void WizardPage3::OnBootLoaderAlgorithmSelected( wxCommandEvent& event )
{
    // Remove previous 
    if ( NULL != ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl ) {
        delete ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl;
        ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl = NULL;
    }

    switch ( m_nBootAlgorithm->GetSelection() ) {
    
        case 0: // VSCP Standard algorithm
            ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl = 
                new CBootDevice_VSCP( 
                    &(((DeviceBootloaderwizard *)GetParent())->m_csw),
                    ((DeviceBootloaderwizard *)GetParent())->m_guid );
            break;

        case 1: // Microchip PIC 1 algorithm
            ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl = 
                new CBootDevice_PIC1( 
                    &(((DeviceBootloaderwizard *)GetParent())->m_csw),
                    ((DeviceBootloaderwizard *)GetParent())->m_guid );
            break;

        case 2:
            ((DeviceBootloaderwizard *)GetParent())->m_pBootCtrl = NULL;
            break;
    
    }
    
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ALGORITHM_FROM_MDF
//

void WizardPage3::OnButtonAlgorithmFromMdfClick( wxCommandEvent& event )
{
    wxString strID;
    uint8_t id[ 16 ];
    wxString url;
    bool rv = true;

    // Get selected id
    strID = ((DeviceBootloaderwizard *)GetParent())->m_pgSelecDeviceId->m_comboNodeID->GetValue();

    // Get the device nickname/GUID
    if ( USE_DLL_INTERFACE == ((DeviceBootloaderwizard *)GetParent())->m_csw.getDeviceType() ) {
        *id = vscp_readStringValue( strID );
    }
    else if ( USE_TCPIP_INTERFACE == ((DeviceBootloaderwizard *)GetParent())->m_csw.getDeviceType() ) {
        vscp_getGuidFromStringToArray ( id, strID );
    }

    // Open the interface
    if ( ((DeviceBootloaderwizard *)GetParent())->m_csw.doCmdOpen() ) {

        rv = wxGetApp().loadMDF( this,
                                    &( ((DeviceBootloaderwizard *)GetParent() )->m_csw ), 
                                    &((DeviceBootloaderwizard *)GetParent())->m_mdf, 
                                    url, 
                                    id );

        if ( rv ) {
            // MDF has been fetched -
            ((DeviceBootloaderwizard *)GetParent() )->m_bMDFLoaded = true;

            // MDF has been fetched - Set algorithm
            m_nBootAlgorithm->SetSelection( ((DeviceBootloaderwizard *)GetParent())->m_mdf.m_bootInfo.m_nAlgorithm );
            OnBootLoaderAlgorithmSelected( event );
        }
        else {
            wxMessageBox(_("Failed to fetch MDF! \nIs the device active and available?"));
        }

        // Close the interface
        ((DeviceBootloaderwizard *)GetParent())->m_csw.doCmdClose();

    }
    else {
        wxMessageBox(_("Failed to open communication interface!"));
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILE_FROM_MDF
//

void WizardPage2::OnButtonLoadFileFromMdfClick( wxCommandEvent& event )
{
    wxString strID;
    uint8_t id[16];
    wxString url;
    DeviceBootloaderwizard *pblw = (DeviceBootloaderwizard *)GetParent();

    if ( NULL == pblw ) return;

    bool rv = true;

    // Get selected id
    strID = pblw->m_pgSelecDeviceId->m_comboNodeID->GetValue();

    // Check if the mdf has been loaded and load it if not
    if ( !pblw->m_bMDFLoaded ) {

        // Must load MDF

        // Get the device nickname/GUID
        if ( USE_DLL_INTERFACE == pblw->m_csw.getDeviceType() ) {
            *id = vscp_readStringValue( strID );
        }
        else if ( USE_TCPIP_INTERFACE == pblw->m_csw.getDeviceType() ) {
            vscp_getGuidFromStringToArray ( id, strID );
        }

        // Open the interface
        if ( pblw->m_csw.doCmdOpen() ) {

            bool rv = wxGetApp().loadMDF( this,
                                            &pblw->m_csw, 
                                            &pblw->m_mdf, 
                                            url, 
                                            id );

            if ( rv ) {
                // MDF has been fetched -
                pblw->m_bMDFLoaded = true;

            }
            else {
                rv = false;
                wxMessageBox(_("Failed to fetch MDF! \nIs the device active and available?"));
            }

            // Close the interface
            pblw->m_csw.doCmdClose();

            // Download the MDF file
            rv = pblw->m_mdf.downLoadMDF( pblw->m_mdf.m_strURL, url );
            if ( !rv ) {
                wxMessageBox(_("Failed to download mDF file! [") + 
                                pblw->m_mdf.m_strURL +
                                _("]") );
            }
            else {
                
                // Parse the file
                if ( pblw->m_mdf.parseMDF( url ) ) {
                
                    if ( !pblw->m_pBootCtrl->loadBinaryFile( 
                                                    pblw->m_mdf.m_firmware.m_strPath, 
                                                    HEXFILE_TYPE_INTEL_HEX8 ) ) {

                            wxMessageBox( _T("Failed to load input file.!"), 
                                            _T("ERROR"),
                                            wxICON_WARNING | wxOK, 
                                            this );
                    }
                    else {
                        
                        // Allow wizard continue
                        pblw->m_bHexFileLoaded = true;

                        m_selectedFile->SetLabel( url );
                        pblw->m_pBootCtrl->showInfo( m_hexFileInfo );
                    }
                }
                else {
                    rv = false;
                    wxMessageBox(_("Failed to parse file!"));
                }
            }
        }
        else {
            rv = false;
            wxMessageBox(_("Failed to open communication interface!"));
        }
    }

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE5
//

void WizardPage5::OnWizardpagePreBootloadPageChanging( wxWizardEvent& event )
{
  event.Skip(); 
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON21
//

void WizardPage7::OnButtonProgramClick( wxCommandEvent& event )
{
    wxString strID;
    wxBusyCursor busy;
    
    DeviceBootloaderwizard *pblw = (DeviceBootloaderwizard *)GetParent();

    // Get the device nickname/GUID
    if ( USE_DLL_INTERFACE == pblw->m_csw.getDeviceType() ) {
        pblw->m_pBootCtrl->m_guid.m_id[ 0 ] = 
            vscp_readStringValue( pblw->m_pgSelecDeviceId->m_comboNodeID->GetValue() );
    }
    else if ( USE_TCPIP_INTERFACE == pblw->m_csw.getDeviceType() ) {
        vscp_getGuidFromStringToArray ( pblw->m_pBootCtrl->m_guid.m_id, 
                                    pblw->m_pgSelecDeviceId->m_comboNodeID->GetValue() );
    }

    // Open the interface
    if ( pblw->m_csw.doCmdOpen() ) {
   
        if ( !pblw->m_pBootCtrl->setDeviceInBootMode() ) {
            wxMessageBox(_("Failed to set device in boot mode! \nWill still try to load firmware."));
        }
            
        if ( !pblw->m_pBootCtrl->doFirmwareLoad() ) {
            wxMessageBox(_("Failed to load firmware code into device!"));
        }

        // Close the interface
        pblw->m_csw.doCmdClose();
       
    }
    else {
        wxMessageBox(_("Failed to open communication interface!"));    
    }

    event.Skip(); 
}

