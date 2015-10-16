// FILE: canalobj.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmain.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#if wxUSE_GUI!=0
#include <wx/progdlg.h>
#include <wx/imaglist.h>
#endif
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/url.h>
#include <wx/listimpl.cpp>
#include <wx/sstream.h>
#include <wx/hyperlink.h>

#include <canal.h>
#include <vscphelper.h>
#include "vscp_images.h"
#include <canalobj.h>

WX_DEFINE_LIST( CANALOBJ_CHOICE_LIST );
WX_DEFINE_LIST( CANALOBJ_ITEM_LIST );
WX_DEFINE_LIST( CANALOBJ_FLAGBIT_LIST );

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CCanalObj_Choice::CCanalObj_Choice()
{
    ;
}

CCanalObj_Choice::~CCanalObj_Choice()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CCanalObj_OneItem::CCanalObj_OneItem()
{
    m_pos = 0;
    m_bOptional = false;
    m_type = type_unknown;
}

CCanalObj_OneItem::~CCanalObj_OneItem()
{
    // Clear up choice list
    CANALOBJ_CHOICE_LIST::iterator iterValue;
    for ( iterValue = m_listChoice.begin();
    iterValue != m_listChoice.end();
        ++iterValue ) {
        CCanalObj_Choice *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CCanalObj_FlagBit::CCanalObj_FlagBit()
{
    m_pos = 0;
}

CCanalObj_FlagBit::~CCanalObj_FlagBit()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CCanalObj::CCanalObj()
{
    ;
}

CCanalObj::~CCanalObj()
{
    // Clear up item list
    CANALOBJ_ITEM_LIST::iterator iterValueItem;
    for ( iterValueItem = m_listItem.begin();
    iterValueItem != m_listItem.end();
        ++iterValueItem ) {
        CCanalObj_OneItem *pRecordValue = *iterValueItem;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }

    // Clear up flagbit list
    CANALOBJ_FLAGBIT_LIST::iterator iterValueFlag;
    for ( iterValueFlag = m_listFlagBits.begin();
    iterValueFlag != m_listFlagBits.end();
        ++iterValueFlag ) {
        CCanalObj_FlagBit *pRecordValue = *iterValueFlag;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  parseDriverInfo
// 

bool CCanalObj::parseDriverInfo( wxString& xmldata )
{
    bool rv = true;
    wxStringInputStream xmlstream( xmldata );
    wxXmlDocument doc;

    // Empty old MDF information
    //clearStorage();

    if ( !doc.Load( xmlstream ) ) {
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT( "config" ) ) {
        return false;
    }

    wxXmlNode *child1 = doc.GetRoot()->GetChildren();
    while ( child1 ) {
        if ( child1->GetName() == wxT( "description" ) ) {  
            
            m_decription = child1->GetNodeContent();

            wxString str = child1->GetAttribute( _( "type" ), _( "text" ) );
            str.Trim();
            str.Trim( false );
            str.MakeUpper();

            m_TypeDescription = type_text;
            if ( wxNOT_FOUND != str.Find( _( "HTML" ) ) ) {
                m_TypeDescription = type_html;
            }
        }
        else if ( child1->GetName() == wxT( "level" ) ) {
            m_level = vscp_readStringValue( (const char *)child1->GetNodeContent().mbc_str() );
        }
        else if ( child1->GetName() == wxT( "blocking" ) ) {

            m_decription = child1->GetNodeContent();

            wxString str = child1->GetAttribute( _( "type" ), _( "YES" ) );
            str.Trim();
            str.Trim( false );
            str.MakeUpper();

            m_bBlocking = true;
            if ( wxNOT_FOUND != str.Find( _( "NO" ) ) ) {
                m_bBlocking = false;
            }
        }
        else if ( child1->GetName() == wxT( "infourl" ) ) {
            m_infourl = child1->GetNodeContent();
        }
        else if ( child1->GetName() == wxT( "items" ) ) {
            
            wxXmlNode *child2 = child1->GetChildren();
            while ( child2 ) {

                if ( child2->GetName() == wxT( "item" ) ) {
                    
                    wxString str;

                    CCanalObj_OneItem *pOneItem = new CCanalObj_OneItem;
                    wxASSERT( NULL != pOneItem );
                    m_listItem.Append( pOneItem );
                    
                    pOneItem->m_pos = vscp_readStringValue( child2->GetAttribute( _( "pos" ), _( "0" ) ) );
                    pOneItem->m_description = child2->GetAttribute( _( "description" ), _( "" ) );
                    pOneItem->m_infourl = child2->GetAttribute( _( "infourl" ), _( "" ) );
                    
                    str = child2->GetAttribute( _( "optional" ), _( "false" ) );
                    str.MakeUpper();
                    pOneItem->m_bOptional = false;
                    if ( wxNOT_FOUND != str.Find( _( "TRUE" ) ) ) {
                        m_bBlocking = true;
                    }

                    pOneItem->m_type = type_unknown;
                    str = child2->GetAttribute( _( "type" ), _( "string" ) );
                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find( _( "STRING" ) ) ) {
                        pOneItem->m_type = type_string;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "BOOLEAN" ) ) ) {
                        pOneItem->m_type = type_boolean;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "INT32" ) ) ) {
                        pOneItem->m_type = type_int32;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "UINT32" ) ) ) {
                        pOneItem->m_type = type_uint32;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "FLOAT" ) ) ) {
                        pOneItem->m_type = type_float;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "CHOICE" ) ) ) {
                        pOneItem->m_type = type_choice;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "ISOTIME" ) ) ) {
                        pOneItem->m_type = type_isotime;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "ISODATE" ) ) ) {
                        pOneItem->m_type = type_isodate;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "ISODATETIME" ) ) ) {
                        pOneItem->m_type = type_isodatetime;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "GUID" ) ) ) {
                        pOneItem->m_type = type_guid;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "EVENT" ) ) ) {
                        pOneItem->m_type = type_event;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "FILTER" ) ) ) {
                        pOneItem->m_type = type_filter;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "MASK" ) ) ) {
                        pOneItem->m_type = type_filter;
                    }

                    pOneItem->m_strValue = child2->GetAttribute( _( "value" ), _( "" ) );

                    wxXmlNode *child3 = child2->GetChildren();
                    while ( child3 ) {

                        if ( child3->GetName() == wxT( "choice" ) ) {

                            CCanalObj_Choice *pChoice = new CCanalObj_Choice;
                            wxASSERT( NULL != pChoice );
                            pOneItem->m_listChoice.Append( pChoice );

                            pChoice->m_description = child3->GetAttribute( _( "description" ), _( "" ) );
                            pChoice->m_value = child3->GetAttribute( _( "value" ), _( "" ) );

                        } // choice

                        child3 = child3->GetNext();
                    }

                } // item

                child2 = child2->GetNext();
            } // while

        }
        else if ( child1->GetName() == wxT( "flags" ) ) {

            wxXmlNode *child2 = child1->GetChildren();
            while ( child2 ) {

                if ( ( child2->GetName() == wxT( "bit" )  ) || ( child2->GetName() == wxT( "flag" ) ) ) {

                    wxString str;

                    CCanalObj_FlagBit *pFlagBit = new CCanalObj_FlagBit;
                    wxASSERT( NULL != pFlagBit );
                    m_listFlagBits.Append( pFlagBit );

                    pFlagBit->m_pos = vscp_readStringValue( child2->GetAttribute( _( "pos" ), _( "0" ) ) );
                    pFlagBit->m_width = vscp_readStringValue( child2->GetAttribute( _( "width" ), _( "1" ) ) );
                    pFlagBit->m_defaultVal = vscp_readStringValue( child2->GetAttribute( _( "default" ), _( "0" ) ) );
                    pFlagBit->m_description = child2->GetAttribute( _( "description" ), _( "" ) );
                    pFlagBit->m_infourl = child2->GetAttribute( _( "infourl" ), _( "" ) );

                    pFlagBit->m_type = flagtype_bool;
                    str = child2->GetAttribute( _( "type" ), _( "BOOL" ) );
                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find( _( "CHOICE" ) ) ) {
                        pFlagBit->m_type = flagtype_choice;
                    }
                    else if ( wxNOT_FOUND != str.Find( _( "VALUE" ) ) ) {
                        pFlagBit->m_type = flagtype_value;
                    }

                    wxXmlNode *child3 = child2->GetChildren();
                    while ( child3 ) {

                        if ( child3->GetName() == wxT( "choice" ) ) {

                            CCanalObj_Choice *pChoice = new CCanalObj_Choice;
                            wxASSERT( NULL != pChoice );
                            pFlagBit->m_listChoice.Append( pChoice );

                            pChoice->m_description = child3->GetAttribute( _( "description" ), _( "" ) );
                            pChoice->m_value = child3->GetAttribute( _( "value" ), _( "" ) );

                        } // choice

                        child3 = child3->GetNext();
                    }

                }
       
                child2 = child2->GetNext();
            }

        }

        child1 = child1->GetNext();
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  runWizard
// 

bool CCanalObj::runWizard( wxWindow* parent,
                                    wxString& resultConfigString, 
                                    wxString& resultConfigFlags )
{
    // Clear configuration data
    resultConfigString.Empty();
    resultConfigFlags.Empty();

    CanalConfigWizard wizard( parent );
    wizard.CreateControls( this );
    wizard.Run(); 
    return true;
}



//=============================================================================
//                            CANAL CONFIG WIZARD
//=============================================================================

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard type definition
//

IMPLEMENT_DYNAMIC_CLASS( CanalConfigWizard, wxWizard )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard event table definition
//

BEGIN_EVENT_TABLE( CanalConfigWizard, wxWizard )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard constructors
//

CanalConfigWizard::CanalConfigWizard()
{
    Init();
}

CanalConfigWizard::CanalConfigWizard( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Init();
    Create( parent, id, pos );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard creator
//

bool CanalConfigWizard::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS | wxWIZARD_EX_HELPBUTTON );
    wxBitmap wizardBitmap( GetBitmapResource( wxT( "vscp_logo.jpg" ) ) );
    wxWizard::Create( parent,
                      id,
                      CANAL_CONFIG_WIZARD_TITLE,
                      wizardBitmap,
                      pos,
                      wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX );

    //CreateControls();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CanalConfigWizard destructor
//

CanalConfigWizard::~CanalConfigWizard()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void CanalConfigWizard::Init()
{
    m_pgStart = NULL;
    for ( int i = 0; i < MAX_PARAMETERS; i++ ) {
        m_pgConfig[ i ] = NULL;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for CanalConfigWizard
//

void CanalConfigWizard::CreateControls( CCanalObj *pObj )
{
    wxASSERT( NULL != pObj );
    m_pconfigObj = pObj;
    CanalConfigWizard* pWizard = this;

    // Create the start page
    WizardCanalConfigPageStart* itemWizardStartPage = new WizardCanalConfigPageStart;
    itemWizardStartPage->m_pconfigObj = pObj;
    itemWizardStartPage->Create( pWizard );
    pWizard->GetPageAreaSizer()->Add( itemWizardStartPage );
    
    wxWizardPageSimple* lastPage = NULL;

    // Chain the pages
    if ( lastPage ) {
        wxWizardPageSimple::Chain( lastPage, itemWizardStartPage );
    }
    lastPage = itemWizardStartPage;

    // Create the config parameter pages
    for ( unsigned int i = 0; i < m_pconfigObj->m_listItem.GetCount(); i++ ) {

        m_pgConfig[ i ] = new WizardPageCanalConfig;
        m_pgConfig[ i ]->m_strHead = wxString::Format(_("Parameter %d"), i+1 );
        m_pgConfig[ i ]->m_pItem = m_pconfigObj->m_listItem.Item( i )->GetData();
        m_pgConfig[ i ]->Create( pWizard );
        pWizard->GetPageAreaSizer()->Add( m_pgConfig[ i ] );

        // Chain the page
        if ( lastPage ) {
            wxWizardPageSimple::Chain( lastPage, m_pgConfig[ i ] );
        }
        lastPage = m_pgConfig[ i ];
    }

    // * * * Flags * * *

    // Construct the flag parameter pages
    for ( unsigned int i = 0; i < m_pconfigObj->m_listFlagBits.GetCount(); i++ ) {

        m_pgConfigFlags[ i ] = new WizardPageFlagsConfig;
        m_pgConfigFlags[ i ]->m_strHead = wxString::Format( _( "Flags %d" ), i + 1 );
        m_pgConfigFlags[ i ]->m_pItem = m_pconfigObj->m_listFlagBits.Item( i )->GetData();
        m_pgConfigFlags[ i ]->Create( pWizard );
        pWizard->GetPageAreaSizer()->Add( m_pgConfigFlags[ i ] );

        // Chain the page
        if ( lastPage ) {
            wxWizardPageSimple::Chain( lastPage, m_pgConfigFlags[ i ] );
        }
        lastPage = m_pgConfigFlags[ i ];
    }


    /*if ( NULL != m_pgConfig[ m_plistItem->GetCount() ] ) {
        lastPage = m_pgConfig[ m_plistItem->GetCount() - 1 ];
    }
    else {
        lastPage = itemWizardStartPage;
    }*/

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runs the wizard.
//

bool CanalConfigWizard::Run()
{
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

bool CanalConfigWizard::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap CanalConfigWizard::GetBitmapResource( const wxString& name )
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

wxIcon CanalConfigWizard::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}





//*******************************************************************************************************************
//                                              WizardCanalConfigPageStart
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardCanalConfigPageStart type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardCanalConfigPageStart, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage event table definition
//

BEGIN_EVENT_TABLE( WizardCanalConfigPageStart, wxWizardPageSimple )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPage constructors
//

WizardCanalConfigPageStart::WizardCanalConfigPageStart()
{
    Init();
}

WizardCanalConfigPageStart::WizardCanalConfigPageStart( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardCanalConfigPageStart creator
//

bool WizardCanalConfigPageStart::Create( wxWizard* parent )
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

WizardCanalConfigPageStart::~WizardCanalConfigPageStart()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardCanalConfigPageStart::Init()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardCanalConfigPageStart
//

void WizardCanalConfigPageStart::CreateControls()
{
    WizardCanalConfigPageStart* itemWizardPage = this;

    wxBoxSizer* itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    itemWizardPage->SetSizer( itemBoxSizer );

    wxStaticText* itemStaticTextTop = new wxStaticText;
    itemStaticTextTop->Create( itemWizardPage,
                             wxID_STATIC,
                             _( "The CANAL configuration wizard will walk you true the configuration needed\nto use and setup any CANAL driver without any need to look up things in a \nmanual. Setup becomes as easy as it can be." ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer->Add( itemStaticTextTop, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticTextMiddle = new wxStaticText;
    itemStaticTextMiddle->Create( itemWizardPage,
                             wxID_STATIC,
                             _( "All CANAL drivers have two configuration values. First the configuration string \nwhich is a list of semicolon separated values and secondly there is an array of \n32 bit-flags which each have logical values. Individual flags can also be put in \ngroups that span several bits and then can hold other information pieces." ),
                             wxDefaultPosition,
                             wxDefaultSize,
                             0 );
    itemBoxSizer->Add( itemStaticTextMiddle, 0, wxALIGN_LEFT | wxALL, 5 );

    wxASSERT( NULL != m_pconfigObj );
    if ( m_pconfigObj->m_infourl.Length() ) {

        wxHyperlinkCtrl* intenHyperLink = new wxHyperlinkCtrl;
        intenHyperLink->Create( itemWizardPage,
                                wxID_STATIC,
                                _( "Click for more information about the driver" ),
                                m_pconfigObj->m_infourl );
        itemBoxSizer->Add( intenHyperLink, 0, wxALIGN_LEFT | wxALL, 5 );

        itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }

    // Description
    wxStaticText* itemStaticTextDescription = new wxStaticText;
    itemStaticTextDescription->Create( itemWizardPage,
                                       wxID_STATIC,
                                       m_pconfigObj->m_decription,
                                       wxDefaultPosition,
                                       /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                       0 );
    itemStaticTextDescription->Wrap( DEFAULT_STOCK_TEXT_WIDTH );
    itemBoxSizer->Add( itemStaticTextDescription, 0, wxALIGN_LEFT | wxALL, 5 );

    // Level
    wxString strBlocking;
    if ( m_pconfigObj->m_bBlocking ) {
        strBlocking = "blocking";
    }
    else {
        strBlocking = "non blocking";
    }
    wxString strInfo = wxString::Format( _( "This is a %s level %d driver." ), (const char *)strBlocking.mbc_str(), m_pconfigObj->m_level );
    wxStaticText* itemStaticTextInfo = new wxStaticText;
    itemStaticTextInfo->Create( itemWizardPage,
                                       wxID_STATIC,
                                       strInfo,
                                       wxDefaultPosition,
                                       /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                       0 );
    itemStaticTextInfo->Wrap( DEFAULT_STOCK_TEXT_WIDTH );
    itemBoxSizer->Add( itemStaticTextInfo, 0, wxALIGN_LEFT | wxALL, 5 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardCanalConfigPageStart::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardCanalConfigPageStart::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardCanalConfigPageStart::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}





//*******************************************************************************************************************
//                                               WizardPageCanalConfig
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageCanalConfig, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig event table definition
//

BEGIN_EVENT_TABLE( WizardPageCanalConfig, wxWizardPageSimple )
EVT_WIZARD_PAGE_CHANGING( -1, WizardPageCanalConfig::OnWizardPageChanging )
END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig constructors
//

WizardPageCanalConfig::WizardPageCanalConfig()
{
    Init();
}

WizardPageCanalConfig::WizardPageCanalConfig( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig creator
//

bool WizardPageCanalConfig::Create( wxWizard* parent )
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
// WizardPageCanalConfig destructor
//

WizardPageCanalConfig::~WizardPageCanalConfig()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageCanalConfig::Init()
{
    m_listBox = NULL;
    m_textField = NULL;
    m_boolChoice = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageCanalConfig
//

void WizardPageCanalConfig::CreateControls()
{
    WizardPageCanalConfig* itemWizardPage = this;

    wxBoxSizer* itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    itemWizardPage->SetSizer( itemBoxSizer );

    wxStaticText* itemStaticTextHeader = new wxStaticText;
    itemStaticTextHeader->Create( itemWizardPage,
                                    wxID_STATIC,
                                    m_strHead,
                                    wxDefaultPosition,
                                    /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                    0 );
    itemStaticTextHeader->SetFont( wxFont( 16, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
    itemBoxSizer->Add( itemStaticTextHeader, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticTextDescription = new wxStaticText;
    itemStaticTextDescription->Create( itemWizardPage,
                                            wxID_STATIC,
                                            m_pItem->m_description,
                                            wxDefaultPosition,
                                            /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                            0 );
    itemBoxSizer->Add( itemStaticTextDescription, 0, wxALIGN_LEFT | wxALL, 5 );

    if ( m_pItem->m_infourl.Length() ) {

        wxHyperlinkCtrl* intenHyperLink = new wxHyperlinkCtrl;
        intenHyperLink->Create( itemWizardPage,
                                wxID_STATIC,
                                _( "Click for more information" ),
                                m_pItem->m_infourl );
        itemBoxSizer->Add( intenHyperLink, 0, wxALIGN_LEFT | wxALL, 5 );

        itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }

    itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    if ( type_choice == m_pItem->m_type ) {
        
        wxArrayString wxstrings;
        for ( unsigned int i = 0; i < m_pItem->m_listChoice.GetCount(); i++ ) {
            wxstrings.Add( m_pItem->m_listChoice[ i ]->m_description );
        }

        m_listBox = new wxListBox;
        
        m_listBox->Create( itemWizardPage,
                             m_windowsID++,
                             wxDefaultPosition,
                             wxSize( 370, -1 ),
                             wxstrings );
       
        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_listBox->SetToolTip( _( "Set value for parameter" ) );
        }
        
        m_listBox->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_listBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }
    else if ( type_boolean == m_pItem->m_type ) {
        
        m_boolChoice = new wxChoice;
        
        m_boolChoice->Create( itemWizardPage,
                                m_windowsID++,
                                wxDefaultPosition,
                                wxSize( 370, -1 )  );
        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_boolChoice->SetToolTip( _( "Set to enable" ) );
        }

        m_boolChoice->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_boolChoice, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }
    else {

        m_textField = new wxTextCtrl;

        m_textField->Create( itemWizardPage,
                                m_windowsID++,
                                wxEmptyString,
                                wxDefaultPosition,
                                wxSize( 370, -1 ) );

        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_textField->SetToolTip( _( "Set value for parameter" ) );
        }

        m_textField->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_textField, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }

    itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageCanalConfig::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageCanalConfig::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageCanalConfig::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void WizardPageCanalConfig::OnWizardPageChanging( wxWizardEvent& event )
{
   
}





//*******************************************************************************************************************
//                                               WizardPageFlagsConfig
//*******************************************************************************************************************





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageCanalConfig type definition
//

IMPLEMENT_DYNAMIC_CLASS( WizardPageFlagsConfig, wxWizardPageSimple )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageFlagsConfig event table definition
//

BEGIN_EVENT_TABLE( WizardPageFlagsConfig, wxWizardPageSimple )
EVT_WIZARD_PAGE_CHANGING( -1, WizardPageFlagsConfig::OnWizardPageChanging )
END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageFlagsConfig constructors
//

WizardPageFlagsConfig::WizardPageFlagsConfig()
{
    Init();
}

WizardPageFlagsConfig::WizardPageFlagsConfig( wxWizard* parent )
{
    Init();
    Create( parent );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WizardPageFlagsConfig creator
//

bool WizardPageFlagsConfig::Create( wxWizard* parent )
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
// WizardPageFlagsConfig destructor
//

WizardPageFlagsConfig::~WizardPageFlagsConfig()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void WizardPageFlagsConfig::Init()
{
    m_listBox = NULL;
    m_textField = NULL;
    m_boolChoice = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for WizardPageCanalConfig
//

void WizardPageFlagsConfig::CreateControls()
{
    WizardPageFlagsConfig* itemWizardPage = this;

    wxBoxSizer* itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    itemWizardPage->SetSizer( itemBoxSizer );

    wxStaticText* itemStaticTextHeader = new wxStaticText;
    itemStaticTextHeader->Create( itemWizardPage,
                                  wxID_STATIC,
                                  m_strHead,
                                  wxDefaultPosition,
                                  /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                  0 );
    itemStaticTextHeader->SetFont( wxFont( 16, wxSWISS, wxNORMAL, wxBOLD, false, wxT( "Tahoma" ) ) );
    itemBoxSizer->Add( itemStaticTextHeader, 0, wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticTextDescription = new wxStaticText;
    itemStaticTextDescription->Create( itemWizardPage,
                                       wxID_STATIC,
                                       m_pItem->m_description,
                                       wxDefaultPosition,
                                       /*wxDefaultSize*/wxSize( DEFAULT_STOCK_TEXT_WIDTH, -1 ),
                                       0 );
    itemStaticTextDescription->Wrap( DEFAULT_STOCK_TEXT_WIDTH );
    itemBoxSizer->Add( itemStaticTextDescription, 0, wxALIGN_LEFT | wxALL, 5 );

    if ( m_pItem->m_infourl.Length() ) {

        wxHyperlinkCtrl* intenHyperLink = new wxHyperlinkCtrl;
        intenHyperLink->Create( itemWizardPage,
                                wxID_STATIC,
                                _( "Click for more information" ),
                                m_pItem->m_infourl );
        itemBoxSizer->Add( intenHyperLink, 0, wxALIGN_LEFT | wxALL, 5 );

        itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }

    if ( flagtype_choice == m_pItem->m_type ) {

        wxArrayString wxstrings;
        for ( unsigned int i = 0; i < m_pItem->m_listChoice.GetCount(); i++ ) {
            wxstrings.Add( m_pItem->m_listChoice[ i ]->m_description );
        }

        m_listBox = new wxListBox;

        m_listBox->Create( itemWizardPage,
                           m_windowsID++,
                           wxDefaultPosition,
                           wxSize( 370, -1 ),
                           wxstrings );

        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_listBox->SetToolTip( _( "Set value for flag" ) );
        }

        m_listBox->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_listBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }
    else if ( type_boolean == m_pItem->m_type ) {

        m_boolChoice = new wxChoice;

        m_boolChoice->Create( itemWizardPage,
                              m_windowsID++,
                              wxDefaultPosition,
                              wxSize( 370, -1 ) );
        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_boolChoice->SetToolTip( _( "Set to enable" ) );
        }

        m_boolChoice->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_boolChoice, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }
    else {  // number

        m_textField = new wxTextCtrl;

        m_textField->Create( itemWizardPage,
                             m_windowsID++,
                             wxEmptyString,
                             wxDefaultPosition,
                             wxSize( 370, -1 ) );

        if ( WizardPageCanalConfig::ShowToolTips() ) {
            m_textField->SetToolTip( _( "Set value for parameter" ) );
        }

        m_textField->SetBackgroundColour( wxColour( 255, 255, 210 ) );
        itemBoxSizer->Add( m_textField, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    }

    itemBoxSizer->Add( 5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool WizardPageFlagsConfig::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap WizardPageFlagsConfig::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon WizardPageFlagsConfig::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnWizardPageChanging
//

void WizardPageFlagsConfig::OnWizardPageChanging( wxWizardEvent& event )
{

}
