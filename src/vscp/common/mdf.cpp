// FILE: mdf.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
#include <vscp.h>
#include <vscphelper.h>
#include <mdf.h>

WX_DEFINE_LIST( MDF_VALUE_LIST );
WX_DEFINE_LIST( MDF_ABSTRACTION_LIST );
WX_DEFINE_LIST( MDF_BIT_LIST );
WX_DEFINE_LIST( MDF_REGISTER_LIST );
WX_DEFINE_LIST( MDF_ACTIONPARAMETER_LIST );
WX_DEFINE_LIST( MDF_ACTION_LIST );
WX_DEFINE_LIST( MDF_EVENTDATA_LIST );
WX_DEFINE_LIST( MDF_EVENT_LIST );
WX_DEFINE_LIST( MDF_ITEM_LIST );
WX_DEFINE_LIST( MDF_ADDRESS_LIST );
WX_DEFINE_LIST( MDF_MANUFACTURER_LIST );

// notes
// =====
// All object that can be in different languages default to
// "en". This works by assigning the selected locale ( m_strLocale )
// if found and the default ("en") if the selected language is not found.

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF_ValueListValue::CMDF_ValueListValue()
{
    ;
}

CMDF_ValueListValue::~CMDF_ValueListValue()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF_Abstraction::CMDF_Abstraction()
{

}

CMDF_Abstraction::~CMDF_Abstraction()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
// 

void CMDF_Abstraction::clearStorage( void )
{
    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin(); 
            iterValue != m_list_value.end(); 
            ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionValueType
// 

wxString CMDF_Abstraction::getAbstractionValueType( void )
{
    switch ( m_nType ) {

    case type_string: 
        return( wxString(_("String")) );

    case type_boolval:
        return( wxString(_("Boolean")) );

    case type_bitfield:
        return( wxString(_("Bitfield")) );

    case type_int8_t:
        return( wxString(_("Signed 8-bit integer")) );

    case type_uint8_t:
        return( wxString(_("Unsigned 8-bit integer")) );

    case type_int16_t:
        return( wxString(_("Signed 16-bit integer")) );

    case type_uint16_t:
        return( wxString(_("Unsigned 16-bit integer")) );

    case type_int32_t:
        return( wxString(_("Signed 32-bit integer")) );


    case type_uint32_t:
        return( wxString(_("Unsigned 32-bit integer")) );

    case type_int64_t:
        return( wxString(_("Signed 64-bit integer")) );

    case type_uint64_t:
        return( wxString(_("Unsigned 64-bit integer")) );

    case type_float:
        return( wxString(_("float")) );

    case type_double:
        return( wxString(_("double")) );

    case type_date:
        return( wxString(_("Date")) );

    case type_time:
        return( wxString(_("Time")) );

    case type_guid:
        return( wxString(_("GUID")) );

    case type_unknown:
    default:
        return( wxString(_("Unknown Type")) );
    }
}


///////////////////////////////////////////////////////////////////////////////
//  getAbstractionTypeByteCount
//

uint16_t CMDF_Abstraction::getAbstractionTypeByteCount( void )
{
    uint16_t width = 0;
    
    switch (m_nType) {

        case type_string:
            width = m_nWidth;
            break;

        case type_boolval:
            width = 1;
            break;

        case type_bitfield:
            width = m_nWidth;
            break;

        case type_int8_t:
        case type_uint8_t:
            width = 1;
            break;


        case type_int16_t:
        case type_uint16_t:
            width = 2;
            break;

        case type_int32_t:
        case type_uint32_t:
            width = 4;
            break;

        case type_int64_t:
        case type_uint64_t:
            width = 8;
            break;

        case type_float:
            width = 4;
            break;

        case type_double:
            width = 8;
            break;

        case type_date:
            width = 6;
            break;

        case type_time:
            width = 6;
            break;

        case type_guid:
            width = 16;
            break;

        case type_unknown:
            width = 0;
        default:
            break;

    } // switch  
    
    return width;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF_Bit::CMDF_Bit()
{
    ;
}

CMDF_Bit::~CMDF_Bit()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
// 

void CMDF_Bit::clearStorage( void )
{
    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin(); 
            iterValue != m_list_value.end(); 
            ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF_Register::CMDF_Register()
{
    ;
}

CMDF_Register::~CMDF_Register()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
// 

void CMDF_Register::clearStorage( void )
{
    // Clearup bit list
    MDF_BIT_LIST::iterator iterBit;
    for ( iterBit = m_list_bit.begin(); 
        iterBit != m_list_bit.end(); 
        ++iterBit) {
            CMDF_Bit *pRecordBit = *iterBit;
            if ( NULL != pRecordBit ) {
                delete pRecordBit;
            }
    }

    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin(); 
        iterValue != m_list_value.end(); 
        ++iterValue ) {
            CMDF_ValueListValue *pRecordValue = *iterValue;
            if ( NULL != pRecordValue ) {
                delete pRecordValue;
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  setDefault
//

uint8_t CMDF_Register::setDefault( void )
{
    if ( wxNOT_FOUND == m_strDefault.Find( _("UNDEF") ) ) {
        m_value = vscp_readStringValue( m_strDefault );
    }
    else {
        m_value = 0;
    }

    return m_value;
}

///////////////////////////////////////////////////////////////////////////////
//  Assignment
//  

CMDF_Register &CMDF_Register::operator=( const CMDF_Register& other )
{
    m_strName = other.m_strName;
    m_strDescription = other.m_strDescription;
    m_strHelpType = other.m_strHelpType;
    m_strHelp = other.m_strHelp;

    m_nPage = other.m_nPage;
    m_nOffset = other.m_nOffset;
    m_nWidth = other.m_nWidth;

    m_type = other.m_type;
    m_size = other.m_size;

    m_nMin = other.m_nMin;
    m_nMax = other.m_nMax;

    m_strDefault = other.m_strDefault;

    m_nAccess = other.m_nAccess;

    m_rowInGrid = other.m_rowInGrid;
    m_value = other.m_value;

    m_fgcolor = other.m_fgcolor;
    m_bgcolor = other.m_bgcolor;

    // Clearup bit list
    MDF_BIT_LIST::iterator iterBit;
    for ( iterBit = m_list_bit.begin();
          iterBit != m_list_bit.end();
          ++iterBit ) {
        CMDF_Bit *pRecordBit = *iterBit;
        if ( NULL != pRecordBit ) {
            delete pRecordBit;
        }
    }

    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin();
          iterValue != m_list_value.end();
          ++iterValue ) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {
            delete pRecordValue;
        }
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//  

CMDF_ActionParameter::CMDF_ActionParameter()
{
    ;
}

CMDF_ActionParameter::~CMDF_ActionParameter()
{
    clearStorage();
}


///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
// 

void CMDF_ActionParameter::clearStorage( void )
{
    /*
    // Clearup bit list
    MDF_BIT_LIST::iterator iterBit;
    for ( iterBit = m_list_bit.begin(); 
    iterBit != m_list_bit.end(); 
    ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if ( NULL != pRecordBit ) {
    delete pRecordBit;
    }
    }
    */
    m_list_bit.DeleteContents( true );

    /*
    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin(); 
    iterValue != m_list_value.end(); 
    ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if ( NULL != pRecordValue ) {
    delete pRecordValue;
    }
    }	
    */
    m_list_value.DeleteContents( true );
}


///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Action::CMDF_Action()
{
    ;
}

CMDF_Action::~CMDF_Action()
{
    clearStorage();
}


///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
// 

void CMDF_Action::clearStorage( void )
{
    /*
    // Cleanup action parameter list
    MDF_ACTIONPARAMETER_LIST::iterator iterActionParam;
    for ( iterActionParam = m_list_ActionParameter.begin(); 
    iterActionParam != m_list_ActionParameter.end(); 
    ++iterActionParam ) {
    CMDF_ActionParameter *pRecordActionParam = *iterActionParam;
    if ( NULL != pRecordActionParam ) {
    delete pRecordActionParam;
    }
    }

    // Delete all nodes
    m_list_ActionParameter.Clear();
    */	
    m_list_ActionParameter.DeleteContents( true );
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_DecisionMatrix::CMDF_DecisionMatrix()
{
    m_nLevel = 1;
    m_nStartPage = 0;
    m_nStartOffset = 0;
    m_nRowCount = 0;
    m_nRowSize = 8;
    m_bIndexed = false;
}

CMDF_DecisionMatrix::~CMDF_DecisionMatrix()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

void CMDF_DecisionMatrix::clearStorage() 
{
    m_nLevel = 1; 
    m_nStartPage = 0;
    m_nStartOffset = 0; 
    m_nRowCount = 0;
    m_nRowSize = 8; 

    m_list_action.DeleteContents( true );
    m_list_action.Clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_EventData::CMDF_EventData()
{
    ;
}

CMDF_EventData::~CMDF_EventData()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_EventData::clearStorage() 
{
    // Clearup bit list
    MDF_BIT_LIST::iterator iterBit;
    for ( iterBit = m_list_bit.begin(); 
        iterBit != m_list_bit.end(); 
        ++iterBit) {
            CMDF_Bit *pRecordBit = *iterBit;
            if ( NULL != pRecordBit ) {
                delete pRecordBit;
            }
    }

    // Clearup value list
    MDF_VALUE_LIST::iterator iterValue;
    for ( iterValue = m_list_value.begin(); 
        iterValue != m_list_value.end(); 
        ++iterValue) {
            CMDF_ValueListValue *pRecordValue = *iterValue;
            if ( NULL != pRecordValue ) {
                delete pRecordValue;
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Event::CMDF_Event()
{
    ;
}

CMDF_Event::~CMDF_Event()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_Event::clearStorage() 
{
    // Cleanup event data
    MDF_EVENTDATA_LIST::iterator iterEventData;
    for ( iterEventData = m_list_eventdata.begin(); 
        iterEventData != m_list_eventdata.end(); 
        ++iterEventData) {
            CMDF_EventData *pRecordEventData = *iterEventData;
            if ( NULL != pRecordEventData ) {
                delete pRecordEventData;
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Item::CMDF_Item()
{

}

CMDF_Item::~CMDF_Item()
{

}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_BootLoaderInfo::CMDF_BootLoaderInfo()
{

}

CMDF_BootLoaderInfo::~CMDF_BootLoaderInfo()
{

}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_BootLoaderInfo::clearStorage( void ) 
{	
    m_nAlgorithm = 0;
    m_nBlockSize = 0;
    m_nBlockCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Address::CMDF_Address()
{

}

CMDF_Address::~CMDF_Address()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_Address::clearStorage( void ) 
{	
    m_strStreet.Empty();    
    m_strTown.Empty();
    m_strCity.Empty();
    m_strPostCode.Empty();
    m_strState.Empty();
    m_strRegion.Empty();
    m_strCountry.Empty();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Manufacturer::CMDF_Manufacturer()
{
    ;
}

CMDF_Manufacturer::~CMDF_Manufacturer()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_Manufacturer::clearStorage( void ) 
{
    // Cleanup Address list 
    MDF_ADDRESS_LIST::iterator iterAddress;
    for (iterAddress = m_list_Address.begin(); 
        iterAddress != m_list_Address.end(); 
        ++iterAddress) {
            CMDF_Address *pRecordAddress = *iterAddress;
            if ( NULL != pRecordAddress ) {
                delete pRecordAddress;
            }
    }

    // Cleanup Phone list
    MDF_ITEM_LIST::iterator iterPhone;
    for (iterPhone = m_list_Phone.begin(); 
        iterPhone != m_list_Phone.end(); 
        ++iterPhone) {
            CMDF_Item *pRecordPhone = *iterPhone;
            if ( NULL != pRecordPhone ) {
                delete pRecordPhone;
            }
    }

    // Cleanup Fax list
    MDF_ITEM_LIST::iterator iterFax;
    for (iterFax = m_list_Fax.begin(); 
        iterFax != m_list_Fax.end(); 
        ++iterFax) {
            CMDF_Item *pRecordFax = *iterFax;
            if ( NULL != pRecordFax ) {
                delete pRecordFax;
            }
    }

    // Cleanup mail list
    MDF_ITEM_LIST::iterator iterEmail;
    for (iterEmail = m_list_Email.begin(); 
        iterEmail != m_list_Email.end(); 
        ++iterEmail) {
            CMDF_Item *pRecordEmail = *iterEmail;
            if ( NULL != pRecordEmail ) {
                delete pRecordEmail;
            }
    }

    // Cleanup web list
    MDF_ITEM_LIST::iterator iterweb;
    for (iterweb = m_list_Web.begin(); 
        iterweb != m_list_Web.end(); 
        ++iterweb) {
            CMDF_Item *pRecordWeb = *iterweb;
            if ( NULL != pRecordWeb ) {
                delete pRecordWeb;
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF_Firmware::CMDF_Firmware()
{
    clearStorage();
}

CMDF_Firmware::~CMDF_Firmware()
{

}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF_Firmware::clearStorage( void ) 
{
    m_strPath.Empty();
    m_size = 0;
    m_description.Empty();
    m_version_major = 0;
    m_version_minor = 0;
    m_version_subminor = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
// 

CMDF::CMDF()
{
    m_strLocale = _("en");
}

CMDF::~CMDF()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void CMDF::clearStorage( void ) 
{
    // Clean up manufacturer list
    MDF_MANUFACTURER_LIST::iterator iterManufacturer;
    for ( iterManufacturer = m_list_manufacturer.begin(); 
        iterManufacturer != m_list_manufacturer.end(); 
        ++iterManufacturer) {
            CMDF_Manufacturer *pRecordManufacturer = *iterManufacturer;
            if ( NULL != pRecordManufacturer ) {
                delete pRecordManufacturer;
            }
    }

    // Cleanup node event list
    MDF_EVENT_LIST::iterator iterEvent;
    for (iterEvent = m_list_event.begin(); 
        iterEvent != m_list_event.end(); 
        ++iterEvent) {
            CMDF_Event *pRecordEvent = *iterEvent;
            if ( NULL != pRecordEvent ) {
                delete pRecordEvent;
            }
    }

    // Clean up register list
    MDF_REGISTER_LIST::iterator iterRegister;
    for (iterRegister = m_list_register.begin(); 
        iterRegister != m_list_register.end(); 
        ++iterRegister) {
            CMDF_Register *pRecordRegister = *iterRegister;
            if ( NULL != pRecordRegister ) {
                delete pRecordRegister;
            }
    }

    // Clean up abstraction list
    MDF_ABSTRACTION_LIST::iterator iterAbstraction;
    for (iterAbstraction = m_list_abstraction.begin(); 
        iterAbstraction != m_list_abstraction.end(); 
        ++iterAbstraction) {
            CMDF_Abstraction *pRecordAbstraction = *iterAbstraction;
            if ( NULL != pRecordAbstraction ) {
                delete pRecordAbstraction;
            }
    }

    // Clear alarm bit list
    MDF_BIT_LIST::iterator iterAlarmBits;
    for (iterAlarmBits = m_list_alarmbits.begin(); 
        iterAlarmBits != m_list_alarmbits.end(); 
        ++iterAlarmBits) {
            CMDF_Bit *pRecordAlarmBits = *iterAlarmBits;
            if ( NULL != pRecordAlarmBits ) {
                delete pRecordAlarmBits;
            }
    }

    m_strModule_Name.Empty();
    m_strModule_Model.Empty();
    m_strModule_Version.Empty();
    m_strModule_Description.Empty();
    m_strModule_InfoURL.Empty();
    m_changeDate.Empty();
    m_Module_buffersize = 0;
    m_strURL.Empty();
    m_list_manufacturer.Clear();
    m_dmInfo.clearStorage();
    m_bootInfo.clearStorage();
    m_list_event.Clear();
    m_list_register.Clear();
    m_list_abstraction.Clear();
    m_list_alarmbits.Clear();

    m_firmware.clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  downLoadMDF
//  

bool CMDF::downLoadMDF( wxString& remoteFile, wxString &tempFileName )
{
    char buf[ 64000 ];
    size_t cnt;
    wxFile tempFile;

    tempFileName = wxFileName::CreateTempFileName( _("mdf"), &tempFile );
    if ( 0 == tempFileName.Length() ) {
        return false;
    }

    m_tempFileName = tempFileName;

    // Create and open http stream
    wxURL url( remoteFile );	
    if ( wxURL_NOERR != url.GetError() ) {
        return false;
    }

    wxInputStream *in_stream;
    if ( NULL == ( in_stream = url.GetInputStream() ) ) {
        return false;	
    }

    do  {

        // Read data from server
        in_stream->Read( buf, sizeof( buf ) );

        // get valid size for buffer
        cnt = in_stream->LastRead();

        // Write buffer to file
        tempFile.Write( buf, cnt );

    } while ( cnt == sizeof( buf ) );

    // we are done with the stream object
    if ( NULL != in_stream ) delete in_stream;

    // Close the file
    tempFile.Close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  load
//


bool CMDF::load( wxString& remoteFile, bool bLocalFile, bool bSilent  )
{
    wxString localFile = remoteFile;

    if ( wxNOT_FOUND == remoteFile.Find( _("http://") ) ) {
        wxString str;
        str = _("http://");
        str += remoteFile;
        remoteFile = str;
    }

    // Get URL from user if not given
    if ( !bSilent && !bLocalFile && !remoteFile.Length() ) {
#if wxUSE_GUI!=0
        remoteFile = ::wxGetTextFromUser( _("Please enter URI to MDF file on server ") );
#else
        return false;
#endif

    }
    // Get filename from user if not given
    else if ( !bSilent && bLocalFile && !remoteFile.Length() ) {

#if wxUSE_GUI!=0
        // Load MDF from local file
        wxFileDialog dlg( NULL,
                            _("Choose file to load MDF from "),
                            wxStandardPaths::Get().GetUserDataDir(),
                            _(""),
                            _("Module Description Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*") );
        if ( wxID_OK == dlg.ShowModal() ) {
            localFile = dlg.GetPath();
        }
        else {
            return false;
        }
#else
        return false;
#endif
    
    }

    // Must have a path at this point
    if ( 0 == remoteFile.Length() ) {
    
        if ( !bSilent && bLocalFile ) {
#if wxUSE_GUI!=0
            ::wxMessageBox( _("A filename must be entered."), _("VSCP Works"), wxICON_ERROR );
#endif
        }
        else if ( !bSilent ) {
#if wxUSE_GUI!=0
            ::wxMessageBox( _("A URI must be entered."), _("VSCP Works"), wxICON_ERROR );
#endif
        }

        return false;

    }

    if ( !bLocalFile ) {
        
        if ( !downLoadMDF( remoteFile, localFile ) ) {
#if wxUSE_GUI!=0
            if ( !bSilent ) ::wxMessageBox( _("Unable to download MDF."), 
                                                _("VSCP Works"), 
                                                wxICON_ERROR );
#endif
            return false;
        }

    }

    return parseMDF( localFile );
}

///////////////////////////////////////////////////////////////////////////////
//  mdfDescriptionFormat
//

void CMDF::mdfDescriptionFormat( wxString& str )
{
    bool bEscape = false;
    bool bSpace = true;     // Remove leading space 
    wxString strWork;
    unsigned int i;

    for ( i = 0; i < str.Length(); i++ ) {

        if ( bSpace && ( ' ' != str[ i ] ) ) bSpace = false;

        if ( bEscape ) {

            switch ( (wxChar) str[ i ] ) {

                case '\\':
                    strWork += str[ i ];
                    break;

                case 'n':
                    strWork += '\n';
                    bSpace = true;      // Don't allow a space to start line
                    break;

            }

            bEscape = false;

        }
        else {
            switch ( (wxChar) str[ i ] ) {

                case '\n':
                case '\r':
                case '\t':
                    break;

                case '\\':
                    bEscape = true;
                    break;

                    // We allow only one space
                case ' ':
                    if ( !bSpace ) {
                        strWork += str[ i ];
                        bSpace = true;
                    }
                    break;

                default:
                    strWork += str[ i ];
            }
        }
    }

    str = strWork;

    int cnt = 0;

    strWork.Empty();

    // Break apart line that are longer then 80 chars
    for ( i = 0; i < str.Length(); i++ ) {

        if ( '\n' == str[ i ] ) {
            cnt = 0;
        }

        cnt++;

        if ( cnt > 80 ) {

            if ( ' ' == str[ i ] ) {

                // Add line break
                strWork += '\n';

                cnt = 0;
            }
            else {
                strWork += str[ i ];    
            }

        }
        else {
            strWork += str[ i ];
        }
    }

    str = strWork;

}


///////////////////////////////////////////////////////////////////////////////
//  parseMDF
//  

bool CMDF::parseMDF( wxString& path )
{
    bool rv = true;
    wxXmlDocument doc;

    // Empty old MDF information
    clearStorage();

    if ( !doc.Load( path ) ) {
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT("vscp") ) {
        return false;
    }

    wxXmlNode *child1 = doc.GetRoot()->GetChildren();
    while ( child1 ) {

        // One module
        if ( child1->GetName() == wxT("module") ) {

            wxXmlNode *child2 = child1->GetChildren();
            while ( child2 ) {

                if ( child2->GetName() == wxT("name") ) {
                    m_strModule_Name = child2->GetNodeContent();
                }
                else if ( child2->GetName() == wxT("model") ) {
                    m_strModule_Model = child2->GetNodeContent();
                }
                else if ( child2->GetName() == wxT("version") ) {
                    m_strModule_Version = child2->GetNodeContent();
                }
                else if ( child2->GetName() == wxT("description") ) {
                    wxString str;
                   
                    str = child2->GetAttribute( _( "lang" ), _("en") );
                  
                    if ( str == m_strLocale ) {
                        m_strModule_Description = child2->GetNodeContent();
                    }
                    else if ( ( _("en") == str ) && m_strModule_Description.IsEmpty() ) {
                        m_strModule_Description = child2->GetNodeContent();
                    }
                }        
                else if ( child2->GetName() == wxT("infourl") ) {
                    m_strModule_InfoURL = child2->GetNodeContent();
                } 
                else if ( child2->GetName() == wxT("changed") ) {
                    m_changeDate = child2->GetNodeContent();
                }
                else if ( child2->GetName() == wxT("buffersize") ) {
                    m_Module_buffersize = vscp_readStringValue( child2->GetNodeContent() );
                }
                else if ( child2->GetName() == wxT("manufacturer") ) {

                    CMDF_Manufacturer *pmanufacturer = new CMDF_Manufacturer;
                    wxASSERT( NULL != pmanufacturer );
                    m_list_manufacturer.Append( pmanufacturer );

                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("name") ) {
                            pmanufacturer->m_strName = child3->GetNodeContent();
                        }
                        else if ( child3->GetName() == wxT("address") ) {

                            CMDF_Address *pAddress = new CMDF_Address;
                            wxASSERT( NULL != pAddress );
                            pmanufacturer->m_list_Address.Append( pAddress );

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("street") ) {
                                    pAddress->m_strStreet = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("town") ) {
                                    pAddress->m_strTown = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("city") ) {
                                    pAddress->m_strCity = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("postcode") ) {
                                    pAddress->m_strPostCode = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("state") ) {
                                    pAddress->m_strState = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("region") ) {
                                    pAddress->m_strRegion = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("country") ) {
                                    pAddress->m_strCountry = child4->GetNodeContent();
                                }

                                child4 = child4->GetNext();

                            } // child4

                        } // address
                        else if ( child3->GetName() == wxT("telephone") ) {

                            CMDF_Item *pPhone = new CMDF_Item;
                            wxASSERT( NULL != pPhone );
                            pmanufacturer->m_list_Phone.Append( pPhone );

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("number") ) {
                                    pPhone->m_strItem = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
                               
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pPhone->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pPhone->m_strDescription.IsEmpty() ) {
                                        pPhone->m_strDescription = child4->GetNodeContent();
                                    }
                                } 

                                child4 = child4->GetNext();

                            } // child 4

                        } // telephone
                        else if ( child3->GetName() == wxT("fax") ) {

                            CMDF_Item *pFax = new CMDF_Item;
                            wxASSERT( NULL != pFax );
                            pmanufacturer->m_list_Fax.Append( pFax );

                            wxXmlNode *child4 = child3->GetChildren();
                            while ( child4 ) {

                                if ( child4->GetName() == wxT("number") ) {
                                    pFax->m_strItem = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
                                  
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                
                                    if ( str == m_strLocale ) {
                                        pFax->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pFax->m_strDescription.IsEmpty() ) {
                                        pFax->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                
                                child4 = child4->GetNext();
                                
                            } // Child 4
                        } // fax
                        else if ( child3->GetName() == wxT("email") ) {

                            CMDF_Item *pEmail = new CMDF_Item;
                            wxASSERT( NULL != pEmail );
                            pmanufacturer->m_list_Email.Append( pEmail );

                            wxXmlNode *child4 = child3->GetChildren();
                            while ( child4 ) {

                                if ( child4->GetName() == wxT("address") ) {
                                    pEmail->m_strItem = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                  
                                    if ( str == m_strLocale ) {
                                        pEmail->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pEmail->m_strDescription.IsEmpty() ) {
                                        pEmail->m_strDescription = child4->GetNodeContent();
                                    }
                                }

                                child4 = child4->GetNext();

                            } // child 4

                        } // email
                        else if ( child3->GetName() == wxT("web") ) {

                            CMDF_Item *pWeb = new CMDF_Item;
                            wxASSERT( NULL != pWeb );
                            pmanufacturer->m_list_Web.Append( pWeb );

                            wxXmlNode *child4 = child3->GetChildren();
                            while ( child4 ) {

                                if ( child4->GetName() == wxT("address") ) {
                                    pWeb->m_strItem = child4->GetNodeContent();
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                             
                                    if ( str == m_strLocale ) {
                                        pWeb->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pWeb->m_strDescription.IsEmpty() ) {
                                        pWeb->m_strDescription = child4->GetNodeContent();
                                    }
                                }

                                child4 = child4->GetNext();

                            } // child 4 

                        } // web

                        child3 = child3->GetNext();

                    } // while child3

                } // manufacturer
                else if ( child2->GetName() == wxT("firmware") ) {


                    m_firmware.m_strPath = child2->GetAttribute( _( "path" ), _("") );                              
                    m_firmware.m_size  = vscp_readStringValue( child2->GetAttribute( _( "size" ), _("0") ) );               
                    wxString format = child2->GetAttribute( _( "format" ), _("intelhex8") );
                    m_firmware.m_version_major  = vscp_readStringValue( child2->GetAttribute( _( "version_major" ), _("0") ) );
                    m_firmware.m_version_minor  = vscp_readStringValue( child2->GetAttribute( _( "version_minor" ), _("0") ) );
                    m_firmware.m_version_subminor  = vscp_readStringValue( child2->GetAttribute( _( "version_subminor" ), _("0") ) );

                    wxXmlNode *child3 = child2->GetChildren();
                    while ( child3 ) {

                        wxString str;                      
                        str = child3->GetAttribute( _( "lang" ), _("en") );
                        
                        if ( str == m_strLocale ) {
                            m_firmware.m_description = child3->GetNodeContent();
                        }
                        else if ( ( _("en") == str ) && m_strModule_Description.IsEmpty() ) {
                            m_firmware.m_description = child3->GetNodeContent();
                        }

                        child3 = child3->GetNext();

                    } // child3

                }  // firmware 




                else if ( child2->GetName() == wxT("abstractions") ) {

                    wxXmlNode *child3 = child2->GetChildren();
                    while ( child3 ) {

                        if ( child3->GetName() == wxT("abstraction") ) {

                            CMDF_Abstraction *pAbstraction = new CMDF_Abstraction;
                            wxASSERT( NULL != pAbstraction );
                            m_list_abstraction.Append( pAbstraction );

                        
                            pAbstraction->m_strID =  child3->GetAttribute( _( "id" ), _("") );                          
                            pAbstraction->m_strDefault =  child3->GetAttribute( _( "default" ), _("") );
                            pAbstraction->m_nPage =  vscp_readStringValue( child3->GetAttribute( _( "page" ), _("0") ) );                           
                            pAbstraction->m_nOffset =  vscp_readStringValue( child3->GetAttribute( _( "offset" ), _("0") ) );                                                      
                            pAbstraction->m_nBitnumber =  vscp_readStringValue( child3->GetAttribute( _( "bit" ), _("0") ) );                          
                            pAbstraction->m_nWidth =  vscp_readStringValue( child3->GetAttribute( _( "width" ), _("0") ) );                           
                            pAbstraction->m_nMax =  vscp_readStringValue( child3->GetAttribute( _( "max" ), _("0") ) );                          
                            pAbstraction->m_nMin =  vscp_readStringValue( child3->GetAttribute( _( "min" ), _("0") ) );						
                            wxString stridx = child3->GetAttribute( _( "indexed" ), _("false") );
                            pAbstraction->m_fgcolor = wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( wxT( "fgcolor" ), wxT( "0x00000000" ) ) ) );
                            pAbstraction->m_bgcolor = wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( wxT( "bgcolor" ), wxT( "0xffffffff" ) ) ) );
                            pAbstraction->m_fgcolor >>= 8;
                            pAbstraction->m_bgcolor >>= 8;
                            
                            stridx.Lower();
                            if ( wxNOT_FOUND != stridx.Find(_("true")) ) {
                                pAbstraction->m_bIndexed = true;
                            }
                            else {
                                pAbstraction->m_bIndexed = false;
                            }
                         
                            wxString strType =  child3->GetAttribute( _( "type" ), _("") );                            

                            if ( strType.IsSameAs(_("string")) ) {
                                pAbstraction->m_nType = type_string;
                            }
                            else if ( strType.IsSameAs(_("bitfield")) ) {
                                pAbstraction->m_nType = type_bitfield;
                            }
                            else if ( strType.IsSameAs(_("bool")) ) {
                                pAbstraction->m_nType = type_boolval;
                            }
                            else if ( strType.IsSameAs(_("char")) ) {
                                pAbstraction->m_nType = type_int8_t;
                            }
                            else if ( strType.IsSameAs(_("int8_t")) ) {
                                pAbstraction->m_nType = type_int8_t;
                            }
                            else if ( strType.IsSameAs(_("byte")) ) {
                                pAbstraction->m_nType = type_uint8_t;
                            }
                            else if ( strType.IsSameAs(_("uint8_t")) ) {
                                pAbstraction->m_nType = type_uint8_t;
                            }
                            else if ( strType.IsSameAs(_("int16_t")) ) {
                                pAbstraction->m_nType = type_int16_t;
                            }
                            else if ( strType.IsSameAs(_("short")) ) {
                                pAbstraction->m_nType = type_int16_t;
                            }
                            else if ( strType.IsSameAs(_("integer")) ) {
                                pAbstraction->m_nType = type_int16_t;
                            }
                            else if ( strType.IsSameAs(_("uint16_t")) ) {
                                pAbstraction->m_nType = type_uint16_t;
                            }
                            else if ( strType.IsSameAs(_("int32_t")) ) {
                                pAbstraction->m_nType = type_int32_t;
                            }
                            else if ( strType.IsSameAs(_("long")) ) {
                                pAbstraction->m_nType = type_int32_t;
                            }
                            else if ( strType.IsSameAs(_("uint32_t")) ) {
                                pAbstraction->m_nType = type_uint32_t;
                            }
                            else if ( strType.IsSameAs(_("int64_t")) ) {
                                pAbstraction->m_nType = type_int64_t;
                            }
                            else if ( strType.IsSameAs(_("uint64_t")) ) {
                                pAbstraction->m_nType = type_uint64_t;
                            }
                            else if ( strType.IsSameAs(_("double")) ) {
                                pAbstraction->m_nType = type_double;
                            }
                            else if ( strType.IsSameAs(_("float")) ) {
                                pAbstraction->m_nType = type_float;
                            }
                            else if ( strType.IsSameAs(_("date")) ) {
                                pAbstraction->m_nType = type_date;
                            }
                            else if ( strType.IsSameAs(_("time")) ) {
                                pAbstraction->m_nType = type_time;
                            }
                            else if ( strType.IsSameAs(_("guid")) ) {
                                pAbstraction->m_nType = type_guid;
                            }
                            else if ( strType.IsSameAs(_("index8_int16_t")) ) {
                                pAbstraction->m_nType = type_index8_int16_t;
                            }
                            else if ( strType.IsSameAs(_("index8_uint16_t")) ) {
                                pAbstraction->m_nType = type_index8_uint16_t;
                            }
                            else if ( strType.IsSameAs(_("index8_int32_t")) ) {
                                pAbstraction->m_nType = type_guid;
                            }
                            else if ( strType.IsSameAs(_("index8_uint32_t")) ) {
                                pAbstraction->m_nType = type_index8_uint32_t;
                            }
                            else if ( strType.IsSameAs(_("index8_int64_t")) ) {
                                pAbstraction->m_nType = type_index8_int64_t;
                            }
                            else if ( strType.IsSameAs(_("index8_uint64_t")) ) {
                                pAbstraction->m_nType = type_index8_uint64_t;
                            }
                            else if ( strType.IsSameAs(_("index8_float")) ) {
                                pAbstraction->m_nType = type_index8_float;
                            }
                            else if ( strType.IsSameAs(_("index8_double")) ) {
                                pAbstraction->m_nType = type_index8_double;
                            }
                            else if ( strType.IsSameAs(_("index8_date")) ) {
                                pAbstraction->m_nType = type_index8_date;
                            }
                            else if ( strType.IsSameAs(_("index8_time")) ) {
                                pAbstraction->m_nType = type_index8_time;
                            }
                            else if ( strType.IsSameAs(_("index8_guid")) ) {
                                pAbstraction->m_nType = type_index8_guid;
                            }
                            else if ( strType.IsSameAs(_("index8_string")) ) {
                                pAbstraction->m_nType = type_index8_string;
                            }
                            else {
                                pAbstraction->m_nType = type_unknown;
                            }

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;                                
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pAbstraction->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAbstraction->m_strName.IsEmpty() ) {
                                        pAbstraction->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;                                 
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pAbstraction->m_strDescription = child4->GetNodeContent();
                                        mdfDescriptionFormat( pAbstraction->m_strDescription );
                                    }
                                    else if ( ( _("en") == str ) && pAbstraction->m_strName.IsEmpty() ) {
                                        pAbstraction->m_strDescription = child4->GetNodeContent();
                                        mdfDescriptionFormat( pAbstraction->m_strDescription );
                                    }
                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                    
                                    if ( str == m_strLocale ) {
                                        pAbstraction->m_strHelp = child4->GetNodeContent();                                       
                                        pAbstraction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                      
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pAbstraction->m_strHelp = child4->GetNodeContent();                                   
                                        pAbstraction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                       
                                    }
                                }                
                                else if ( child4->GetName() == wxT("access") ) {

                                    // Set default access
                                    pAbstraction->m_nAccess = MDF_ACCESS_READ;
                                    wxString strAccessRights = child4->GetNodeContent();

                                    strAccessRights.MakeUpper();
                                    if ( wxNOT_FOUND != strAccessRights.Find(_("R") ) ) {
                                        pAbstraction->m_nAccess |= MDF_ACCESS_READ;
                                    }

                                    if ( wxNOT_FOUND != strAccessRights.Find(_("W")) ) {
                                        pAbstraction->m_nAccess |= MDF_ACCESS_WRITE;
                                    }

                                }
                                else if ( child4->GetName() == wxT("valuelist") ) {

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("item") ) {

                                            CMDF_ValueListValue *pValueItem = new CMDF_ValueListValue;
                                            wxASSERT( NULL != pValueItem );
                                            pAbstraction->m_list_value.Append( pValueItem );                                          
                                            pValueItem->m_strValue = child5->GetAttribute( _( "value" ), _("") );                                           

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;                                                  
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                  
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;                                                
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                 
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();                                                       
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                    
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();                                                       
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                       
                                                    }
                                                }                        

                                                child6 = child6->GetNext();

                                            }

                                        }

                                        child5 = child5->GetNext();

                                    } // while 5

                                }

                                child4 = child4->GetNext();

                            } // while 5
                        }

                        child3 = child3->GetNext();
                    }

                }        
                else if ( child2->GetName() == wxT("registers") ) {

                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("reg") ) {

                            wxString strType;
                            CMDF_Register *pRegister = new CMDF_Register;
                            wxASSERT( NULL != pRegister );
                             
                            pRegister->m_rowInGrid = -1; // Set when reg is written to grid
                            pRegister->m_nPage = vscp_readStringValue( child3->GetAttribute( wxT( "page" ), wxT("0") ) );                           
                            pRegister->m_nOffset = vscp_readStringValue( child3->GetAttribute( wxT( "offset" ), wxT("0") ) );                         
                            pRegister->m_nWidth = vscp_readStringValue( child3->GetAttribute( wxT( "width" ), wxT("8") ) );                           
                            pRegister->m_size = vscp_readStringValue( child3->GetAttribute( wxT( "size" ), wxT("1") ) );   
                            pRegister->m_nMin = vscp_readStringValue( child3->GetAttribute( wxT( "min" ), wxT( "0" ) ) );
                            pRegister->m_nMax = vscp_readStringValue( child3->GetAttribute( wxT( "max" ), wxT("255") ) );                        
                            pRegister->m_strDefault = child3->GetAttribute( wxT( "default" ), wxT("UNDEF") );
                            pRegister->m_fgcolor = wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( wxT( "fgcolor" ), wxT( "0x00000000" ) ) ) );
                            pRegister->m_bgcolor = wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( wxT( "bgcolor" ), wxT( "0xffffffff" ) ) ) );
                            pRegister->m_fgcolor >>= 8;
                            pRegister->m_bgcolor >>= 8;
                            strType = child3->GetAttribute( wxT( "type" ), wxT( "std" ) );
                            strType.Trim();
                            strType.Trim(false);
                            pRegister->m_type = REGISTER_TYPE_STANDARD;
                            if ( _("dmatrix1") == strType ) {
                                if ( pRegister->m_size > 1 ) {
                                    pRegister->m_type = REGISTER_TYPE_DMATRIX1;
                                }
                            }
                            else if ( _("block") == strType ) {
                                if ( pRegister->m_size > 1 ) {
                                    pRegister->m_type = REGISTER_TYPE_BLOCK;
                                }
                            }
                            
                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;                                 
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pRegister->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pRegister->m_strName.IsEmpty() ) {
                                        pRegister->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;                                  
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pRegister->m_strDescription = child4->GetNodeContent();
                                        mdfDescriptionFormat( pRegister->m_strDescription );
                                    }
                                    else if ( ( _("en") == str ) && pRegister->m_strName.IsEmpty() ) {
                                        pRegister->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("access") ) {
                                    pRegister->m_nAccess = MDF_ACCESS_READ;
                                    wxString strAccessRights = child4->GetNodeContent();

                                    strAccessRights.MakeUpper();
                                    if ( wxNOT_FOUND != strAccessRights.Find(_("R") ) ) {
                                        pRegister->m_nAccess |= MDF_ACCESS_READ;
                                    }

                                    if ( wxNOT_FOUND != strAccessRights.Find(_("W")) ) {
                                        pRegister->m_nAccess |= MDF_ACCESS_WRITE;
                                    }

                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pRegister->m_strHelp = child4->GetNodeContent();                                       
                                        pRegister->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                       
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pRegister->m_strHelp = child4->GetNodeContent();                                       
                                        pRegister->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                      
                                    }
                                }                
                                // Not allowed for non standard regs
                                else if ( ( child4->GetName() == wxT( "bitfield" ) ) && 
                                          ( REGISTER_TYPE_STANDARD== pRegister->m_type ) ) {

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("bit") ) {

                                            CMDF_Bit *pBit = new CMDF_Bit;
                                            wxASSERT( NULL != pBit );
                                            pRegister->m_list_bit.Append( pBit );                                           
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "pos" ), _("0") ) );                                         
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetAttribute( _( "width" ), _("0") ) );                                           
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "default" ), _("0") ) );
                                           

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;                                                  
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );   
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();                                                       
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();                                                       
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                     
                                                    }
                                                }                        
                                                else if ( child6->GetName() == wxT("access") ) {

                                                    pBit->m_nAccess = MDF_ACCESS_READ;
                                                    wxString strAccessRights = child6->GetNodeContent();

                                                    strAccessRights.MakeUpper();
                                                    if ( wxNOT_FOUND != strAccessRights.Find(_("R") ) ) {
                                                        pBit->m_nAccess |= MDF_ACCESS_READ;
                                                    }

                                                    if ( wxNOT_FOUND != strAccessRights.Find(_("W")) ) {
                                                        pBit->m_nAccess |= MDF_ACCESS_WRITE;
                                                    }

                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        }

                                        child5 = child5->GetNext();

                                    } // while 5

                                }
                                // Not allowed for non standard regs
                                else if ( ( child4->GetName() == wxT( "valuelist" ) ) &&
                                                ( REGISTER_TYPE_STANDARD == pRegister->m_type ) ) {

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("item") ) {

                                            CMDF_ValueListValue *pValueItem = new CMDF_ValueListValue;
                                            wxASSERT( NULL != pValueItem );
                                            pRegister->m_list_value.Append( pValueItem );
                                            pValueItem->m_strValue = child5->GetAttribute( _( "value" ), _("") );
                                           

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                 
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();                                                         
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                       
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();                                                         
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );                                                        
                                                       
                                                    }
                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        } // item

                                        child5 = child5->GetNext();

                                    } // while 5

                                }

                                child4 = child4->GetNext();

                            } // while 4

                            // If register type is not 'std' then we need to add more registers
                            // at this point accoring to the 'size' attribute
                            if ( REGISTER_TYPE_DMATRIX1 == pRegister->m_type ) {

                                for ( int idx = 0; idx < pRegister->m_size; idx++ ) {
                                    wxString str;
                                    CMDF_Register *pRegisterExt = new CMDF_Register;
                                    wxASSERT( NULL != pRegisterExt );
                                    *pRegisterExt = *pRegister;
                                    pRegisterExt->m_size = 1;
                                    pRegisterExt->m_nOffset += idx;
                                    
                                    switch ( idx % 8 ) {
                                        case 0: // originating address
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Oaddr" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _("Originating address.\nSet to nickname for node that should trigger action here. Oaddr is the originating address.\nMeans we are only interested in messages from the node given here.\n0x00 is segment controller and 0xff is a node without a nickname.\nIf bit 6 of flags is set oaddr will not be checked and events from all nodes will be accepted.");
                                            break;
                                        case 1: // flags
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Flags" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "Flags. Set selection behaviour.\nThe enable bit can be used to disable a decion matrix row while it is edited.\nThe zone and use subzone bits can be activated to have a check on the zone / subzone information\nof an event.That is the zone / subzone of the machine must match the one of the event to trigger\nthe DM row." );
                                            break;
                                        case 2: // class mask
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Class mask (low eight bits)" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "The lowest eight bits of the class mask that defines the events that should trigger the action of\nthis decision matrix row. Bit 8 of the class mask is stored in bit 1 of the flag byte." );
                                            break;
                                        case 3: // class filter
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Class filter (low eight bits)" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "The lowest eight bits of the class filter that defines the events that should trigger the action of\nthis decision matrix row. Bit 8 of the class filter is stored in bit 1 of the flag byte." );
                                            break;
                                        case 4: // type mask
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Type mask" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "Type mask that defines the events that should trigger the action of this decision matrix row." );
                                            break;
                                        case 5: // type filter
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Type filter" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "Type filter that defines the events that should trigger the action of this decision matrix row." );
                                            break;
                                        case 6: // action
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Action" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "This is the action or operation that should be performed if the filtering is satisfied.\nOnly action code 0x00 is predefined and means No-Operation.\nAll other codes are application specific and typical application defined codes could do\nmeasurement, send predefined event etc." );
                                            break;
                                        case 7: // action parameter
                                            pRegisterExt->m_strName = wxString::Format( _( "Decision matrix row %d: Action parameter" ), 1+idx/8 );
                                            pRegisterExt->m_strDescription = _( "A numeric action parameter can be set and its meaning is application specific." );
                                            break;
                                    }

                                    // Append the register
                                    m_list_register.Append( pRegisterExt );

                                }

                                // Detete the placeholder
                                delete pRegister;

                            }
                            else if ( REGISTER_TYPE_BLOCK == pRegister->m_type ) {

                                for ( int idx = 0; idx < pRegister->m_size; idx++ ) {
                                    CMDF_Register *pRegisterExt = new CMDF_Register;
                                    wxASSERT( NULL != pRegisterExt );
                                    *pRegisterExt = *pRegister;
                                    pRegisterExt->m_size = 1;
                                    pRegisterExt->m_nOffset += idx;
                                    pRegisterExt->m_strName = wxString::Format( _( "%s%d" ), pRegisterExt->m_strName.c_str(), idx );
                                    // Append the register
                                    m_list_register.Append( pRegisterExt );
                                }

                                // Detete the placeholder
                                delete pRegister;

                            }
                            else {
                                m_list_register.Append( pRegister );
                            }

                        } // Reg

                        child3 = child3->GetNext();

                    } // while 3

                }
                else if ( child2->GetName() == wxT("dmatrix") ) {

                    //
                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("level") ) {
                            m_dmInfo.m_nLevel = vscp_readStringValue( child3->GetNodeContent() );
                        }
                        else if ( child3->GetName() == wxT("start") ) {                           
                            m_dmInfo.m_nStartPage =  vscp_readStringValue( child3->GetAttribute( _( "page" ), _("0") ) );                           
                            m_dmInfo.m_nStartOffset = vscp_readStringValue( child3->GetAttribute( _( "offset" ), _("0") ) );							                         
                            wxString stridx = child3->GetAttribute( _( "indexed" ), _("false") );
                           
                            
                            stridx.Lower();
                            if ( wxNOT_FOUND != stridx.Find(_("true")) ) {
                                m_dmInfo.m_bIndexed = true;
                            }
                            else {
                                m_dmInfo.m_bIndexed = false;
                            }
                        }
                        else if ( child3->GetName() == wxT("rowcnt") ) {
                            m_dmInfo.m_nRowCount = vscp_readStringValue( child3->GetNodeContent() );
                        }
                        else if ( child3->GetName() == wxT("rowsize") ) {
                            m_dmInfo.m_nRowSize = vscp_readStringValue( child3->GetNodeContent() );
                        }
                        else if ( child3->GetName() == wxT("action") ) {

                            CMDF_Action *pAction = new CMDF_Action;
                            wxASSERT( NULL != pAction );
                            m_dmInfo.m_list_action.Append( pAction );

                            // Action parameters                           
                            pAction->m_nCode = vscp_readStringValue( child3->GetAttribute( _( "code" ), _("0") ) );
                           

                            wxXmlNode *child4 = child3->GetChildren();
                            while ( child4 ) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                  
                                    if ( str == m_strLocale ) {
                                        pAction->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAction->m_strName.IsEmpty() ) {
                                        pAction->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                    
                                    if ( str == m_strLocale ) {
                                        pAction->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAction->m_strName.IsEmpty() ) {
                                        pAction->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;                                  
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                   
                                    if ( str == m_strLocale ) {
                                        pAction->m_strHelp = child4->GetNodeContent();                                      
                                        pAction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                        
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pAction->m_strHelp = child4->GetNodeContent();                                       
                                        pAction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                       
                                    }
                                }
                                else if ( child4->GetName() == wxT("param") ) {

                                    CMDF_ActionParameter *pActionParameter = new CMDF_ActionParameter;
                                    wxASSERT( NULL != pActionParameter );
                                    pAction->m_list_ActionParameter.Append( pActionParameter );	

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while ( child5 ) {

                                        if ( child5->GetName() == wxT("name") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                            
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pActionParameter->m_strName.IsEmpty() ) {
                                                pActionParameter->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("description") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                           
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pActionParameter->m_strName.IsEmpty() ) {
                                                pActionParameter->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("help") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                           
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strHelp = child5->GetNodeContent();                                                
                                                pActionParameter->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                               
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pActionParameter->m_strHelp = child5->GetNodeContent();                                                
                                                pActionParameter->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                               
                                            }
                                        }	
                                        else if ( child5->GetName() == wxT("bit") ) {

                                            CMDF_Bit *pBit = new CMDF_Bit;
                                            wxASSERT( NULL != pBit );
                                            pActionParameter->m_list_bit.Append( pBit );	
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "pos" ), _("0") ) );                                                                     
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetAttribute( _( "width" ), _("0") ) );                                          
                                            pBit->m_nDefault = vscp_readStringValue( child5->GetAttribute( _( "default" ), _("0") ) );                                          
                                            pBit->m_nAccess = vscp_readStringValue( child5->GetAttribute( _( "access" ), _("0") ) );
                                            

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while ( child6 ) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;
                                                   
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
                                                   
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
                                                      
                                                    }
                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        }
                                        else if ( child5->GetName() == wxT("valuelist") ) {	

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while ( child6 ) {

                                                if ( child6->GetName() == wxT("item") ) {

                                                    CMDF_ValueListValue *pValue = new CMDF_ValueListValue;
                                                    wxASSERT( NULL != pValue );
                                                    pActionParameter->m_list_value.Append( pValue );                                                   
                                                    pValue->m_strValue = child6->GetAttribute( _( "value" ), _("0") );
                                                   

                                                    wxXmlNode *child7 = child6->GetChildren();
                                                    while ( child7 ) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;                                                           
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                           
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValue->m_strName.IsEmpty() ) {
                                                                pValue->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;                                                          
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                           
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValue->m_strName.IsEmpty() ) {
                                                                pValue->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child6->GetName() == wxT("help") ) {
                                                            wxString str;                                                          
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                          
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strHelp = child7->GetNodeContent();                                                                 
                                                                pValue->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                                
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pValue->m_strHelp = child7->GetNodeContent();                                                                
                                                                pValue->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                                
                                                            }
                                                        }

                                                        child7 = child7->GetNext();

                                                    } // while 5

                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        }

                                        child5 = child5->GetNext();

                                    } // 5



                                } // if 'param'

                                child4 = child4->GetNext();                        

                            } // while 4

                        } // while 3

                        child3 = child3->GetNext();

                    } // child 3

                }        
                else if ( child2->GetName() == wxT("events") ) {

                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("event") ) {

                            CMDF_Event *pEvent = new CMDF_Event;
                            wxASSERT( NULL != pEvent );
                            m_list_event.Append( pEvent );                           
                            pEvent->m_nClass = vscp_readStringValue( child3->GetAttribute( _( "class" ), _("0") ) );                           
                            pEvent->m_nType = vscp_readStringValue( child3->GetAttribute( _( "type" ), _("0") ) );                            
                            pEvent->m_nPriority = vscp_readStringValue( child3->GetAttribute( _( "priority" ), _("0") ) );                           

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pEvent->m_strName.IsEmpty() ) {
                                        pEvent->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;                                   
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pEvent->m_strName.IsEmpty() ) {
                                        pEvent->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );                                    
                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strHelp = child4->GetNodeContent();                                       
                                        pEvent->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                        
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pEvent->m_strHelp = child4->GetNodeContent();                                       
                                        pEvent->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
                                       
                                    }
                                }                
                                else if ( child4->GetName() == wxT("data") ) {

                                    CMDF_EventData *pEventData = new CMDF_EventData;
                                    wxASSERT( NULL != pEventData );
                                    pEvent->m_list_eventdata.Append( pEventData );                                  
                                    pEventData->m_nOffset = vscp_readStringValue( child4->GetAttribute( _( "offset" ), _("0") ) );
 

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child4->GetName() == wxT("name") ) {
                                            wxString str;                                            
                                            str = child5->GetAttribute ( _( "lang" ), _("en") );
                                             
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pEventData->m_strName.IsEmpty() ) {
                                                pEventData->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child4->GetName() == wxT("description") ) {
                                            wxString str;                                      
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                           
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pEventData->m_strName.IsEmpty() ) {
                                                pEventData->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child4->GetName() == wxT("help") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                             
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strHelp = child5->GetNodeContent();                                               
                                                pEventData->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                                 
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pEventData->m_strHelp = child5->GetNodeContent();                                                
                                                pEventData->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                                
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("valuelist") ) {

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("item") ) {

                                                    CMDF_ValueListValue *pValueItem = new CMDF_ValueListValue;
                                                    wxASSERT( NULL != pValueItem );
                                                    pEventData->m_list_value.Append( pValueItem );                                                  
                                                    pValueItem->m_strValue = child6->GetAttribute( _( "value" ), _("") );
                                                     

                                                    wxXmlNode *child7 = child5->GetChildren();
                                                    while (child7) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;                                                           
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                             
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                                pValueItem->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;                                                           
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                             
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                                pValueItem->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("help") ) {
                                                            wxString str;                                                          
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                            
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strHelp = child7->GetNodeContent();                                                                 
                                                                pValueItem->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                                
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pValueItem->m_strHelp = child7->GetNodeContent();                                                                 
                                                                pValueItem->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                                
                                                            }
                                                        }

                                                        child7 = child7->GetNext();

                                                    }

                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        }
                                        else if ( child5->GetName() == wxT("bitfield") ) {

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("bit") ) {

                                                    CMDF_Bit *pBit = new CMDF_Bit;
                                                    wxASSERT( NULL != pBit );
                                                    pEventData->m_list_bit.Append( pBit );                                                   
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetAttribute( _( "pos" ), _("0") ) );                                                   
                                                    pBit->m_nWidth = vscp_readStringValue( child6->GetAttribute( _( "width" ), _("0") ) );                                                    
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetAttribute( _( "default" ), _("0") ) );
                                                     

                                                    wxXmlNode *child7 = child6->GetChildren();
                                                    while (child7) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;                                                           
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                            
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                                pBit->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;                                                          
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                            
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                                pBit->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("help") ) {
                                                            wxString str;                                                           
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
                                                             
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strHelp = child7->GetNodeContent();                                                                 
                                                                pBit->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                                
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pBit->m_strHelp = child7->GetNodeContent();                                                                
                                                                pBit->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
                                                               
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("access") ) {

                                                            pBit->m_nAccess = MDF_ACCESS_READ;
                                                            wxString strAccessRights = child7->GetNodeContent();

                                                            strAccessRights.MakeUpper();
                                                            if ( wxNOT_FOUND != strAccessRights.Find(_("R") ) ) {
                                                                pBit->m_nAccess |= MDF_ACCESS_READ;
                                                            }

                                                            if ( wxNOT_FOUND != strAccessRights.Find(_("W")) ) {
                                                                pBit->m_nAccess |= MDF_ACCESS_WRITE;
                                                            }

                                                        }

                                                        child7 = child7->GetNext();

                                                    } // while 7

                                                }

                                                child6 = child6->GetNext();

                                            } // while 6

                                        }

                                        child5 = child5->GetNext();

                                    } // while 5

                                }

                                child4 = child4->GetNext();

                            } // while

                        }

                        child3 = child3->GetNext();

                    } // while

                }        
                else if ( child2->GetName() == wxT("alarm") ) {

                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("bitfield") ) {

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("bit") ) {

                                    CMDF_Bit *pBit = new CMDF_Bit;
                                    wxASSERT( NULL != pBit );
                                    m_list_alarmbits.Append( pBit );
                                   
                                    pBit->m_nPos = vscp_readStringValue( child4->GetAttribute( _( "pos" ), _("0") ) );                                   
                                    pBit->m_nWidth = vscp_readStringValue( child4->GetAttribute( _( "width" ), _("0") ) );                                  
                                    pBit->m_nPos = vscp_readStringValue( child4->GetAttribute( _( "default" ), _("0") ) );
                                    

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("name") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                            
                                            if ( str == m_strLocale ) {
                                                pBit->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                pBit->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("description") ) {
                                            wxString str;                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                            
                                            if ( str == m_strLocale ) {
                                                pBit->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                pBit->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("help") ) {
                                            wxString str;                                           
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
                                           
                                            if ( str == m_strLocale ) {
                                                pBit->m_strHelp = child5->GetNodeContent();                                               
                                                pBit->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                                
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pBit->m_strHelp = child5->GetNodeContent();                                              
                                                pBit->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
                                                
                                            }
                                        }                    
                                        else if ( child5->GetName() == wxT("access") ) {

                                            pBit->m_nAccess = MDF_ACCESS_READ;
                                            wxString strAccessRights = child5->GetNodeContent();

                                            strAccessRights.MakeUpper();
                                            if ( wxNOT_FOUND != strAccessRights.Find(_("R") ) ) {
                                                pBit->m_nAccess |= MDF_ACCESS_READ;
                                            }

                                            if ( wxNOT_FOUND != strAccessRights.Find(_("W")) ) {
                                                pBit->m_nAccess |= MDF_ACCESS_WRITE;
                                            }

                                        }

                                        child5 = child5->GetNext();

                                    } // while 7

                                }

                                child4 = child4->GetNext();

                            } // while 6

                        }

                        child3 = child3->GetNext();
                    }

                }
                else if ( child2->GetName() == wxT("boot") ) {

                    wxXmlNode *child3 = child2->GetChildren();
                    while (child3) {

                        if ( child3->GetName() == wxT("algorithm") ) {
                            m_bootInfo.m_nAlgorithm =  vscp_readStringValue( child3->GetNodeContent() );
                        }
                        else if ( child3->GetName() == wxT("blocksize") ) {
                            m_bootInfo.m_nBlockSize = vscp_readStringValue(  child3->GetNodeContent() );
                        }
                        else if ( child3->GetName() == wxT("blockcount") ) {
                            m_bootInfo.m_nBlockSize = vscp_readStringValue(  child3->GetNodeContent() );
                        }

                        child3 = child3->GetNext();
                    }

                }

                child2 = child2->GetNext();

            } // while Child2

        } // module

        child1 = child1->GetNext();

    } // while Child1

    // If a Level I dm is placed at location 126 on the first
    // page it will be automatically set to indexed
    if ( ( 1 == m_dmInfo.m_nLevel ) && 
            ( 126 == m_dmInfo.m_nStartOffset ) &&
            ( 0 == m_dmInfo.m_nStartPage ) ) {
        m_dmInfo.m_bIndexed = true;
    }
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  getNumberOfRegisters
//  

uint32_t CMDF::getNumberOfRegisters( uint32_t page ) 
{ 
    uint32_t nregisters = 0;

    MDF_REGISTER_LIST::iterator iterValue;
    for ( iterValue = m_list_register.begin(); 
            iterValue != m_list_register.end(); 
            ++iterValue) {
        CMDF_Register *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {
            if ( page == pRecordValue->m_nPage ) nregisters++;
        }
    }

    return nregisters;
};

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfPages
//  

uint32_t CMDF::getPages( wxArrayLong& array ) 
{ 
    //bool bFound;

    MDF_REGISTER_LIST::iterator iterValue;
    for ( iterValue = m_list_register.begin(); 
            iterValue != m_list_register.end();
            ++iterValue) {
        CMDF_Register *pRecordValue = *iterValue;
        if ( NULL != pRecordValue ) {

            /*bFound = false;
            for ( uint32_t i=0; i<array.Count(); i++ ) {
                uint16_t ttt = array.Item( i );
                if ( -1 == ttt ) continue;
                if ( pRecordValue->m_nPage == ttt ) {
                    bFound = true;
                    break;
                }
            }*/

            //if (!bFound ) {
            if ( wxNOT_FOUND == array.Index( pRecordValue->m_nPage ) ) {
                array.Add( pRecordValue->m_nPage );
            }

        }
    }

    return array.Count();
};

///////////////////////////////////////////////////////////////////////////////
//  getMDFRegister
//  

CMDF_Register * CMDF::getMDFRegister( uint8_t reg, uint16_t page )
{
    MDF_REGISTER_LIST::iterator iter;
    for ( iter = m_list_register.begin(); iter != m_list_register.end(); ++iter ) {

        CMDF_Register *preg = *iter;
        if ( ( reg == preg->m_nOffset ) && ( page == preg->m_nPage ) ) {
            return preg;
        }
    }

    return NULL;
}


