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

#include <canal.h>
#include <vscphelper.h>
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

CCanalObj_Item::CCanalObj_Item()
{
    ;
}

CCanalObj_Item::~CCanalObj_Item()
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

bool CCanalObj_Item::parseDriverInfo( wxString& path )
{
    bool rv = true;
    wxXmlDocument doc;

    // Empty old MDF information
    //clearStorage();

    if ( !doc.Load( path ) ) {
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT( "config" ) ) {
        return false;
    }

    wxXmlNode *child1 = doc.GetRoot()->GetChildren();
    while ( child1 ) {
        if ( child1->GetName() == wxT( "description" ) ) {  
            
            m_strDecription = child1->GetNodeContent();

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

            m_strDecription = child1->GetNodeContent();

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
            m_strInfoUrl = child1->GetNodeContent();
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

                            pChoice->m_description = child2->GetAttribute( _( "description" ), _( "" ) );
                            pChoice->m_value = child2->GetAttribute( _( "value" ), _( "" ) );

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

                if ( ( child1->GetName() == wxT( "bit" )  ) || ( child1->GetName() == wxT( "flag" ) ) ) {

                    wxString str;

                    CCanalObj_FlagBit *pFlagBit = new CCanalObj_FlagBit;
                    wxASSERT( NULL != pFlagBit );
                    m_listFlagBits.Append( pFlagBit );

                    pFlagBit->m_pos = vscp_readStringValue( child1->GetAttribute( _( "pos" ), _( "0" ) ) );
                    pFlagBit->m_width = vscp_readStringValue( child1->GetAttribute( _( "width" ), _( "1" ) ) );
                    pFlagBit->m_defaultVal = vscp_readStringValue( child1->GetAttribute( _( "default" ), _( "0" ) ) );
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

                            pChoice->m_description = child2->GetAttribute( _( "description" ), _( "" ) );
                            pChoice->m_value = child2->GetAttribute( _( "value" ), _( "" ) );

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

bool CCanalObj_Item::runWizard( wxWindow *pwnd, wxString& resultConfigString, wxString& resultConfigFlags )
{
    return true;
}