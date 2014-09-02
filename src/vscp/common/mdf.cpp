// FILE: mdf.cpp
//
// Copyright (C) 2002-2014 Ake Hedman akhe@grodansparadis.com 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
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
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/imaglist.h>
#include <wx/url.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/url.h>
#include <wx/listimpl.cpp>

#include "canal.h"
#include "vscp.h"
#include "vscphelper.h"
#include "mdf.h"

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

    //m_list_action.Clear(); 
    /*
    // Cleanup action list
    MDF_ACTION_LIST::iterator iterAction;
    for ( iterAction = m_list_action.begin(); 
    iterAction != m_list_action.end(); 
    ++iterAction) {
    CMDF_Action *pRecordAction = *iterAction;
    if ( NULL != pRecordAction ) {
    delete pRecordAction;
    }
    }
    */
    m_list_action.DeleteContents( true );

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
    //wxStandardPaths strpath;
    wxFile tempFile;

    tempFileName = wxFileName::CreateTempFileName( _("mdf"), &tempFile );
    if ( 0 == tempFileName.Length() ) {
        return false;
    }

    //wxMessageBox( tmpPath );

    ::wxBeginBusyCursor();

    // Create and open http stream
    wxURL url( remoteFile );	
    if ( wxURL_NOERR != url.GetError() ) {
        ::wxEndBusyCursor();
        return false;
    }

    wxInputStream *in_stream;
    if ( NULL == ( in_stream = url.GetInputStream() ) ) {
        ::wxEndBusyCursor();
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

    ::wxEndBusyCursor();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  load
//


bool CMDF::load( wxString& remoteFile, bool bLocalFile, bool bSilent  )
{
    //wxStandardPaths stdpaths;
	wxString localFile = remoteFile;

	if ( wxNOT_FOUND == remoteFile.Find( _("http://") ) ) {
		wxString str;
		str = _("http://");
		str += remoteFile;
		remoteFile = str;
	}

	// Get URL from user if not given
	if ( !bSilent && !bLocalFile && !remoteFile.Length() ) {

		remoteFile = ::wxGetTextFromUser( _("Please enter URI to MDF file on server ") );

	}
	// Get filename from user if not given
	else if ( !bSilent && bLocalFile && !remoteFile.Length() ) {

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
	
	}

	// Must have a path at this point
	if ( 0 == remoteFile.Length() ) {
	
		if ( !bSilent && bLocalFile ) {
			::wxMessageBox( _("A filename must be entered."), _("VSCP Works"), wxICON_ERROR );
		}
		else if ( !bSilent ) {
			::wxMessageBox( _("A URI must be entered."), _("VSCP Works"), wxICON_ERROR );
		}

		return false;

	}

	if ( !bLocalFile ) {
		
		if ( !downLoadMDF( remoteFile, localFile ) ) {
			if ( !bSilent ) ::wxMessageBox( _("Unable to download MDF."), 
												_("VSCP Works"), 
												wxICON_ERROR );
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
#if wxMAJOR_VERSION > 3                    
                    str = child2->GetAttribute( _( "lang" ), _("en") );
#else 
                    str = child2->GetPropVal( _( "lang" ), _("en") );
#endif                     
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pFax->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pFax->m_strDescription.IsEmpty() ) {
                                        pFax->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                            } // Child 4

                            child4 = child4->GetNext();

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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
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

#if wxMAJOR_VERSION > 3
                    m_firmware.m_strPath = child2->GetAttribute( _( "path" ), _("") );
#else 
                    m_firmware.m_strPath = child2->GetPropVal( _( "path" ), _("") );
#endif                    
#if wxMAJOR_VERSION > 3                    
                    m_firmware.m_size  = vscp_readStringValue( child2->GetAttribute( _( "size" ), _("0") ) );
#else 
                    m_firmware.m_size  = readStringValue( child2->GetPropVal( _( "size" ), _("0") ) );
#endif                    
#if wxMAJOR_VERSION > 3                    
                    wxString format = child2->GetAttribute( _( "format" ), _("intelhex8") );
#else 
                    wxString format = child2->GetPropVal( _( "format" ), _("intelhex8") );
#endif        
#if wxMAJOR_VERSION > 3                    
                    m_firmware.m_version_major  = vscp_readStringValue( child2->GetAttribute( _( "version_major" ), _("0") ) );
#else 
                    m_firmware.m_version_major  = readStringValue( child2->GetPropVal( _( "version_major" ), _("0") ) );
#endif                     
#if wxMAJOR_VERSION > 3                    
                    m_firmware.m_version_minor  = vscp_readStringValue( child2->GetAttribute( _( "version_minor" ), _("0") ) );
#else 
                    m_firmware.m_version_minor  = readStringValue( child2->GetPropVal( _( "version_minor" ), _("0") ) );
#endif                    
#if wxMAJOR_VERSION > 3                    
                    m_firmware.m_version_subminor  = vscp_readStringValue( child2->GetAttribute( _( "version_subminor" ), _("0") ) );
#else 
                    m_firmware.m_version_subminor  = readStringValue( child2->GetPropVal( _( "version_subminor" ), _("0") ) );
#endif                    
                    

                    wxXmlNode *child3 = child2->GetChildren();
                    while ( child3 ) {

                        wxString str;
#if wxMAJOR_VERSION > 3                        
                        str = child3->GetAttribute( _( "lang" ), _("en") );
#else 
                        str = child3->GetPropVal( _( "lang" ), _("en") );
#endif                        
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

#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_strID =  child3->GetAttribute( _( "id" ), _("") );
#else 
                            pAbstraction->m_strID =  child3->GetPropVal( _( "id" ), _("") );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_strDefault =  child3->GetAttribute( _( "default" ), _("") );
#else 
                            pAbstraction->m_strDefault =  child3->GetPropVal( _( "default" ), _("") );
#endif                            
#if wxMAJOR_VERSION > 3
                            pAbstraction->m_nPage =  vscp_readStringValue( child3->GetAttribute( _( "page" ), _("0") ) );
#else 
                            pAbstraction->m_nPage =  vscp_readStringValue( child3->GetPropVal( _( "page" ), _("0") ) );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_nOffset =  vscp_readStringValue( child3->GetAttribute( _( "offset" ), _("0") ) );
#else 
                            pAbstraction->m_nOffset =  vscp_readStringValue( child3->GetPropVal( _( "offset" ), _("0") ) );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_nBitnumber =  vscp_readStringValue( child3->GetAttribute( _( "bit" ), _("0") ) );
#else 
                            pAbstraction->m_nBitnumber =  vscp_readStringValue( child3->GetPropVal( _( "bit" ), _("0") ) );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_nWidth =  vscp_readStringValue( child3->GetAttribute( _( "width" ), _("0") ) );
#else 
                            pAbstraction->m_nWidth =  vscp_readStringValue( child3->GetPropVal( _( "width" ), _("0") ) );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_nMax =  vscp_readStringValue( child3->GetAttribute( _( "max" ), _("0") ) );
#else 
                            pAbstraction->m_nMax =  vscp_readStringValue( child3->GetPropVal( _( "max" ), _("0") ) );
#endif                            
#if wxMAJOR_VERSION > 3                            
                            pAbstraction->m_nMin =  vscp_readStringValue( child3->GetAttribute( _( "min" ), _("0") ) );
#else 
                            pAbstraction->m_nMin =  vscp_readStringValue( child3->GetPropVal( _( "min" ), _("0") ) );
#endif                            

#if wxMAJOR_VERSION > 3							
                            wxString stridx = child3->GetAttribute( _( "indexed" ), _("false") );
#else 
                            wxString stridx = child3->GetPropVal( _( "indexed" ), _("false") );
#endif                            
							stridx.Lower();
							if ( wxNOT_FOUND != stridx.Find(_("true")) ) {
								pAbstraction->m_bIndexed = true;
							}
							else {
								pAbstraction->m_bIndexed = false;
							}

#if wxMAJOR_VERSION > 3                            
                            wxString strType =  child3->GetAttribute( _( "type" ), _("") );
#else 
                            wxString strType =  child3->GetPropVal( _( "type" ), _("") );
#endif                            

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
                                pAbstraction->m_nType = type_int8_t;
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pAbstraction->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAbstraction->m_strName.IsEmpty() ) {
                                        pAbstraction->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pAbstraction->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pAbstraction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pAbstraction->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pAbstraction->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pAbstraction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pAbstraction->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
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

#if wxMAJOR_VERSION > 3                                            
                                            pValueItem->m_strValue = child5->GetAttribute( _( "value" ), _("") );
#else 
                                            pValueItem->m_strValue = child5->GetPropVal( _( "value" ), _("") );
#endif                                            

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                         
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pValueItem->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                         
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pValueItem->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
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

                            CMDF_Register *pRegister = new CMDF_Register;
                            wxASSERT( NULL != pRegister );
                            m_list_register.Append( pRegister );

#if wxMAJOR_VERSION > 3                            
                            pRegister->m_nPage = vscp_readStringValue( child3->GetAttribute( wxT( "page" ), wxT("0") ) );
#else 
                            pRegister->m_nPage = vscp_readStringValue( child3->GetPropVal( wxT( "page" ), wxT("0") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            pRegister->m_nOffset = vscp_readStringValue( child3->GetAttribute( wxT( "offset" ), wxT("0") ) );
#else 
                            pRegister->m_nOffset = vscp_readStringValue( child3->GetPropVal( wxT( "offset" ), wxT("0") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            pRegister->m_nWidth = vscp_readStringValue( child3->GetAttribute( wxT( "width" ), wxT("8") ) );
#else 
                            pRegister->m_nWidth = vscp_readStringValue( child3->GetPropVal( wxT( "width" ), wxT("8") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            pRegister->m_nWidth = vscp_readStringValue( child3->GetAttribute( wxT( "width" ), wxT("8") ) );
#else 
                            pRegister->m_nWidth = vscp_readStringValue( child3->GetPropVal( wxT( "width" ), wxT("8") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            pRegister->m_nMax = vscp_readStringValue( child3->GetAttribute( wxT( "max" ), wxT("255") ) );
#else 
                            pRegister->m_nMax = vscp_readStringValue( child3->GetPropVal( wxT( "max" ), wxT("255") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            pRegister->m_strDefault = child3->GetAttribute( wxT( "default" ), wxT("UNDEF") );
#else 
                            pRegister->m_strDefault = child3->GetPropVal( wxT( "default" ), wxT("UNDEF") );
#endif
                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pRegister->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pRegister->m_strName.IsEmpty() ) {
                                        pRegister->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
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
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pRegister->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pRegister->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pRegister->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pRegister->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pRegister->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pRegister->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
                                    }
                                }                
                                else if ( child4->GetName() == wxT("bitfield") ) {

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("bit") ) {

                                            CMDF_Bit *pBit = new CMDF_Bit;
                                            wxASSERT( NULL != pBit );
                                            pRegister->m_list_bit.Append( pBit );

#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "pos" ), _("0") ) );
#else 
                                            pBit->m_nPos = vscp_readStringValue( child5->GetPropVal( _( "pos" ), _("0") ) );
#endif 
#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetAttribute( _( "width" ), _("0") ) );
#else 
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetPropVal( _( "width" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "default" ), _("0") ) );
#else 
                                            pBit->m_nPos = vscp_readStringValue( child5->GetPropVal( _( "default" ), _("0") ) );
#endif                                            

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );   
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") ); 
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pBit->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pBit->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
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
                                else if ( child4->GetName() == wxT("valuelist") ) {

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("item") ) {

                                            CMDF_ValueListValue *pValueItem = new CMDF_ValueListValue;
                                            wxASSERT( NULL != pValueItem );
                                            pRegister->m_list_value.Append( pValueItem );

#if wxMAJOR_VERSION > 3
                                            pValueItem->m_strValue = child5->GetAttribute( _( "value" ), _("") );
#else 
                                            pValueItem->m_strValue = child5->GetPropVal( _( "value" ), _("") );
#endif                                            

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                        pValueItem->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                          
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pValueItem->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );                                                      
#endif                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pValueItem->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                         
                                                        pValueItem->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );                                                        
#else 
                                                        pValueItem->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") ); 
#endif                                                        
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

                        }

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
#if wxMAJOR_VERSION > 3                            
                            m_dmInfo.m_nStartPage =  vscp_readStringValue( child3->GetAttribute( _( "page" ), _("0") ) );
#else 
                            m_dmInfo.m_nStartPage =  vscp_readStringValue( child3->GetPropVal( _( "page" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                            
                            m_dmInfo.m_nStartOffset = vscp_readStringValue( child3->GetAttribute( _( "offset" ), _("0") ) );							
#else 
                            m_dmInfo.m_nStartOffset = vscp_readStringValue( child3->GetPropVal( _( "offset" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                            
							wxString stridx = child3->GetAttribute( _( "indexed" ), _("false") );
#else 
							wxString stridx = child3->GetPropVal( _( "indexed" ), _("false") );
#endif                            
                            
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
#if wxMAJOR_VERSION > 3                            
                            pAction->m_nCode = vscp_readStringValue( child3->GetAttribute( _( "code" ), _("0") ) );
#else 
                            pAction->m_nCode = readStringValue( child3->GetPropVal( _( "code" ), _("0") ) );
#endif                            

                            wxXmlNode *child4 = child3->GetChildren();
                            while ( child4 ) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pAction->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAction->m_strName.IsEmpty() ) {
                                        pAction->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pAction->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pAction->m_strName.IsEmpty() ) {
                                        pAction->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                    
                                    if ( str == m_strLocale ) {
                                        pAction->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pAction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pAction->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pAction->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pAction->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pAction->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                        
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
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pActionParameter->m_strName.IsEmpty() ) {
                                                pActionParameter->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("description") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pActionParameter->m_strName.IsEmpty() ) {
                                                pActionParameter->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("help") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pActionParameter->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                 
                                                pActionParameter->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pActionParameter->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pActionParameter->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                 
                                                pActionParameter->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pActionParameter->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                
                                            }
                                        }	
                                        else if ( child5->GetName() == wxT("bit") ) {

                                            CMDF_Bit *pBit = new CMDF_Bit;
                                            wxASSERT( NULL != pBit );
                                            pActionParameter->m_list_bit.Append( pBit );	

#if wxMAJOR_VERSION > 3
                                            pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( _( "pos" ), _("0") ) );
#else 
                                            pBit->m_nPos = vscp_readStringValue( child5->GetPropVal( _( "pos" ), _("0") ) );
#endif                                            
#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetAttribute( _( "width" ), _("0") ) );
#else 
                                            pBit->m_nWidth = vscp_readStringValue( child5->GetPropVal( _( "width" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nDefault = vscp_readStringValue( child5->GetAttribute( _( "default" ), _("0") ) );
#else 
                                            pBit->m_nDefault = vscp_readStringValue( child5->GetPropVal( _( "default" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                                            
                                            pBit->m_nAccess = vscp_readStringValue( child5->GetAttribute( _( "access" ), _("0") ) );
#else 
                                            pBit->m_nAccess = vscp_readStringValue( child5->GetPropVal( _( "access" ), _("0") ) );
#endif                                            

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while ( child6 ) {

                                                if ( child6->GetName() == wxT("name") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strName = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("description") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                    else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                        pBit->m_strDescription = child6->GetNodeContent();
                                                    }
                                                }
                                                else if ( child6->GetName() == wxT("help") ) {
                                                    wxString str;
#if wxMAJOR_VERSION > 3                                                    
                                                    str = child6->GetAttribute( _( "lang" ), _("en") );
#else 
                                                    str = child6->GetPropVal( _( "lang" ), _("en") );
#endif                                                    
                                                    if ( str == m_strLocale ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pBit->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
                                                    }
                                                    else if ( ( _("en") == str ) ) {
                                                        pBit->m_strHelp = child6->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                        
                                                        pBit->m_strHelpType = child6->GetAttribute( _( "type" ), _("text") );
#else 
                                                        pBit->m_strHelpType = child6->GetPropVal( _( "type" ), _("text") );
#endif                                                        
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

#if wxMAJOR_VERSION > 3                                                    
                                                    pValue->m_strValue = child6->GetAttribute( _( "value" ), _("0") );
#else 
                                                    pValue->m_strValue = child6->GetPropVal( _( "value" ), _("0") );
#endif                                                    

                                                    wxXmlNode *child7 = child6->GetChildren();
                                                    while ( child7 ) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                            
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValue->m_strName.IsEmpty() ) {
                                                                pValue->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                            
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValue->m_strName.IsEmpty() ) {
                                                                pValue->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child6->GetName() == wxT("help") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                            
                                                            if ( str == m_strLocale ) {
                                                                pValue->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pValue->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pValue->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                 
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pValue->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pValue->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pValue->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                 
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

#if wxMAJOR_VERSION > 3                            
                            pEvent->m_nClass = vscp_readStringValue( child3->GetAttribute( _( "class" ), _("0") ) );
#else 
                            pEvent->m_nClass = vscp_readStringValue( child3->GetPropVal( _( "class" ), _("0") ) );
#endif 
#if wxMAJOR_VERSION > 3                            
                            pEvent->m_nType = vscp_readStringValue( child3->GetAttribute( _( "type" ), _("0") ) );
#else 
                            pEvent->m_nType = vscp_readStringValue( child3->GetPropVal( _( "type" ), _("0") ) );
#endif 
#if wxMAJOR_VERSION > 3                            
                            pEvent->m_nPriority = vscp_readStringValue( child3->GetAttribute( _( "priority" ), _("0") ) );
#else 
                            pEvent->m_nPriority = vscp_readStringValue( child3->GetPropVal( _( "priority" ), _("0") ) );
#endif                             

                            wxXmlNode *child4 = child3->GetChildren();
                            while (child4) {

                                if ( child4->GetName() == wxT("name") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strName = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pEvent->m_strName.IsEmpty() ) {
                                        pEvent->m_strName = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("description") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );
#else 
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strDescription = child4->GetNodeContent();
                                    }
                                    else if ( ( _("en") == str ) && pEvent->m_strName.IsEmpty() ) {
                                        pEvent->m_strDescription = child4->GetNodeContent();
                                    }
                                }
                                else if ( child4->GetName() == wxT("help") ) {
                                    wxString str;
#if wxMAJOR_VERSION > 3                                    
                                    str = child4->GetAttribute( _( "lang" ), _("en") );                                    
#else                                     
                                    str = child4->GetPropVal( _( "lang" ), _("en") );
#endif                                     
                                    if ( str == m_strLocale ) {
                                        pEvent->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pEvent->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pEvent->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                         
                                    }
                                    else if ( ( _("en") == str ) ) {
                                        pEvent->m_strHelp = child4->GetNodeContent();
#if wxMAJOR_VERSION > 3                                        
                                        pEvent->m_strHelpType = child4->GetAttribute( _( "type" ), _("text") );
#else 
                                        pEvent->m_strHelpType = child4->GetPropVal( _( "type" ), _("text") );
#endif                                         
                                    }
                                }                
                                else if ( child4->GetName() == wxT("data") ) {

                                    CMDF_EventData *pEventData = new CMDF_EventData;
                                    wxASSERT( NULL != pEventData );
                                    pEvent->m_list_eventdata.Append( pEventData );

#if wxMAJOR_VERSION > 3                                    
                                    pEventData->m_nOffset = vscp_readStringValue( child4->GetAttribute( _( "offset" ), _("0") ) );
#else 
                                    pEventData->m_nOffset = vscp_readStringValue( child4->GetPropVal( _( "offset" ), _("0") ) );
#endif 

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child4->GetName() == wxT("name") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute ( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                             
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pEventData->m_strName.IsEmpty() ) {
                                                pEventData->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child4->GetName() == wxT("description") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                             
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pEventData->m_strName.IsEmpty() ) {
                                                pEventData->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child4->GetName() == wxT("help") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                             
                                            if ( str == m_strLocale ) {
                                                pEventData->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                
                                                pEventData->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pEventData->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                 
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pEventData->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                
                                                pEventData->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pEventData->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                 
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("valuelist") ) {

                                            wxXmlNode *child6 = child5->GetChildren();
                                            while (child6) {

                                                if ( child6->GetName() == wxT("item") ) {

                                                    CMDF_ValueListValue *pValueItem = new CMDF_ValueListValue;
                                                    wxASSERT( NULL != pValueItem );
                                                    pEventData->m_list_value.Append( pValueItem );

#if wxMAJOR_VERSION > 3                                                   
                                                    pValueItem->m_strValue = child6->GetAttribute( _( "value" ), _("") );
#else 
                                                    pValueItem->m_strValue = child6->GetPropVal( _( "value" ), _("") );
#endif                                                     

                                                    wxXmlNode *child7 = child5->GetChildren();
                                                    while (child7) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                                pValueItem->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pValueItem->m_strName.IsEmpty() ) {
                                                                pValueItem->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("help") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pValueItem->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pValueItem->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pValueItem->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pValueItem->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pValueItem->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pValueItem->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                
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

#if wxMAJOR_VERSION > 3                                                    
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetAttribute( _( "pos" ), _("0") ) );
#else 
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetPropVal( _( "pos" ), _("0") ) );
#endif 
#if wxMAJOR_VERSION > 3                                                    
                                                    pBit->m_nWidth = vscp_readStringValue( child6->GetAttribute( _( "width" ), _("0") ) );
#else 
                                                    pBit->m_nWidth = vscp_readStringValue( child6->GetPropVal( _( "width" ), _("0") ) );
#endif 
#if wxMAJOR_VERSION > 3                                                    
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetAttribute( _( "default" ), _("0") ) );
#else 
                                                    pBit->m_nPos = vscp_readStringValue( child6->GetPropVal( _( "default" ), _("0") ) );
#endif                                                     

                                                    wxXmlNode *child7 = child6->GetChildren();
                                                    while (child7) {

                                                        if ( child7->GetName() == wxT("name") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strName = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                                pBit->m_strName = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("description") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strDescription = child7->GetNodeContent();
                                                            }
                                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                                pBit->m_strDescription = child7->GetNodeContent();
                                                            }
                                                        }
                                                        else if ( child7->GetName() == wxT("help") ) {
                                                            wxString str;
#if wxMAJOR_VERSION > 3                                                            
                                                            str = child7->GetAttribute( _( "lang" ), _("en") );
#else 
                                                            str = child7->GetPropVal( _( "lang" ), _("en") );
#endif                                                             
                                                            if ( str == m_strLocale ) {
                                                                pBit->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pBit->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pBit->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                
                                                            }
                                                            else if ( ( _("en") == str ) ) {
                                                                pBit->m_strHelp = child7->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                                 
                                                                pBit->m_strHelpType = child7->GetAttribute( _( "type" ), _("text") );
#else 
                                                                pBit->m_strHelpType = child7->GetPropVal( _( "type" ), _("text") );
#endif                                                                
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

#if wxMAJOR_VERSION > 3                                    
                                    pBit->m_nPos = vscp_readStringValue( child4->GetAttribute( _( "pos" ), _("0") ) );
#else 
                                    pBit->m_nPos = vscp_readStringValue( child4->GetPropVal( _( "pos" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                                    
                                    pBit->m_nWidth = vscp_readStringValue( child4->GetAttribute( _( "width" ), _("0") ) );
#else 
                                    pBit->m_nWidth = vscp_readStringValue( child4->GetPropVal( _( "width" ), _("0") ) );
#endif
#if wxMAJOR_VERSION > 3                                    
                                    pBit->m_nPos = vscp_readStringValue( child4->GetAttribute( _( "default" ), _("0") ) );
#else 
                                    pBit->m_nPos = vscp_readStringValue( child4->GetPropVal( _( "default" ), _("0") ) );
#endif                                    

                                    wxXmlNode *child5 = child4->GetChildren();
                                    while (child5) {

                                        if ( child5->GetName() == wxT("name") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pBit->m_strName = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                pBit->m_strName = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("description") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pBit->m_strDescription = child5->GetNodeContent();
                                            }
                                            else if ( ( _("en") == str ) && pBit->m_strName.IsEmpty() ) {
                                                pBit->m_strDescription = child5->GetNodeContent();
                                            }
                                        }
                                        else if ( child5->GetName() == wxT("help") ) {
                                            wxString str;
#if wxMAJOR_VERSION > 3                                            
                                            str = child5->GetAttribute( _( "lang" ), _("en") );
#else 
                                            str = child5->GetPropVal( _( "lang" ), _("en") );
#endif                                            
                                            if ( str == m_strLocale ) {
                                                pBit->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                
                                                pBit->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pBit->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                
                                            }
                                            else if ( ( _("en") == str ) ) {
                                                pBit->m_strHelp = child5->GetNodeContent();
#if wxMAJOR_VERSION > 3                                                
                                                pBit->m_strHelpType = child5->GetAttribute( _( "type" ), _("text") );
#else 
                                                pBit->m_strHelpType = child5->GetPropVal( _( "type" ), _("text") );
#endif                                                
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

uint32_t CMDF::getPages( SortedArrayLong& arraylong ) 
{ 
	uint32_t npages = 0;
	bool bFound;

	MDF_REGISTER_LIST::iterator iterValue;
    for ( iterValue = m_list_register.begin(); 
			iterValue != m_list_register.end();
			++iterValue) {
		CMDF_Register *pRecordValue = *iterValue;
		if ( NULL != pRecordValue ) {
			//if ( page == pRecordValue->m_nPage ) npages++;
			bFound = false;
			for ( uint32_t i=0; i<arraylong.Count(); i++ ) {
				if ( pRecordValue->m_nPage== arraylong.Index( i ) ) {
					bFound = true;
					break;
				}
			}

			if (!bFound ) arraylong.Add( pRecordValue->m_nPage );

		}
    }

	//arraylong.Sort();

	return arraylong.Count();
};

///////////////////////////////////////////////////////////////////////////////
//  getMDFRegs
//  

CMDF_Register * CMDF::getMDFRegs( uint8_t reg, uint16_t page )
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
