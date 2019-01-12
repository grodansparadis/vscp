// FILE: mdf.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include <deque>
#include <set>
#include <string>

#include <canal.h>
#include <mdf.h>
#include <stdlib.h>
#include <unistd.h>
#include <vscp.h>
#include <vscphelper.h>

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

CMDF_Abstraction::CMDF_Abstraction() {}

CMDF_Abstraction::~CMDF_Abstraction()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_Abstraction::clearStorage(void)
{
    // Clearup value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for (iterValue = m_list_value.begin(); iterValue != m_list_value.end();
         ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionValueType
//

std::string
CMDF_Abstraction::getAbstractionValueType(void)
{
    switch (m_nType) {

        case type_string:
            return (std::string("String"));

        case type_boolval:
            return (std::string("Boolean"));

        case type_bitfield:
            return (std::string("Bitfield"));

        case type_int8_t:
            return (std::string("Signed 8-bit integer"));

        case type_uint8_t:
            return (std::string("Unsigned 8-bit integer"));

        case type_int16_t:
            return (std::string("Signed 16-bit integer"));

        case type_uint16_t:
            return (std::string("Unsigned 16-bit integer"));

        case type_int32_t:
            return (std::string("Signed 32-bit integer"));

        case type_uint32_t:
            return (std::string("Unsigned 32-bit integer"));

        case type_int64_t:
            return (std::string("Signed 64-bit integer"));

        case type_uint64_t:
            return (std::string("Unsigned 64-bit integer"));

        case type_float:
            return (std::string("float"));

        case type_double:
            return (std::string("double"));

        case type_date:
            return (std::string("Date"));

        case type_time:
            return (std::string("Time"));

        case type_guid:
            return (std::string("GUID"));

        case type_unknown:
        default:
            return (std::string("Unknown Type"));
    }
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionTypeByteCount
//

uint16_t
CMDF_Abstraction::getAbstractionTypeByteCount(void)
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

void
CMDF_Bit::clearStorage(void)
{
    // Clearup value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for (iterValue = m_list_value.begin(); iterValue != m_list_value.end();
         ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
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

void
CMDF_Register::clearStorage(void)
{
    // Clearup bit list
    std::deque<CMDF_Bit *>::iterator iterBit;
    for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
        CMDF_Bit *pRecordBit = *iterBit;
        if (NULL != pRecordBit) {
            delete pRecordBit;
        }
    }

    // Clear up value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for (iterValue = m_list_value.begin(); iterValue != m_list_value.end();
         ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
            delete pRecordValue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  setDefault
//

uint8_t
CMDF_Register::setDefault(void)
{
    if (m_strDefault.npos == m_strDefault.find("UNDEF")) {
        m_value = vscp_readStringValue(m_strDefault);
    } else {
        m_value = 0;
    }

    return m_value;
}

///////////////////////////////////////////////////////////////////////////////
//  Assignment
//

CMDF_Register &
CMDF_Register::operator=(const CMDF_Register &other)
{
    m_strName        = other.m_strName;
    m_strDescription = other.m_strDescription;
    m_strHelpType    = other.m_strHelpType;
    m_strHelp        = other.m_strHelp;

    m_nPage   = other.m_nPage;
    m_nOffset = other.m_nOffset;
    m_nWidth  = other.m_nWidth;

    m_type = other.m_type;
    m_size = other.m_size;

    m_nMin = other.m_nMin;
    m_nMax = other.m_nMax;

    m_strDefault = other.m_strDefault;

    m_nAccess = other.m_nAccess;

    m_rowInGrid = other.m_rowInGrid;
    m_value     = other.m_value;

    m_fgcolor = other.m_fgcolor;
    m_bgcolor = other.m_bgcolor;

    // Clearup bit list
    std::deque<CMDF_Bit *>::iterator iterBit;
    for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
        CMDF_Bit *pRecordBit = *iterBit;
        if (NULL != pRecordBit) {
            delete pRecordBit;
        }
    }

    // Clearup value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for (iterValue = m_list_value.begin(); iterValue != m_list_value.end();
         ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
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

void
CMDF_ActionParameter::clearStorage(void)
{
    /*
    // Clearup bit list
    std::deque<CMDF_Bit *>::iterator iterBit;
    for ( iterBit = m_list_bit.begin();
    iterBit != m_list_bit.end();
    ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if ( NULL != pRecordBit ) {
    delete pRecordBit;
    }
    }
    */
    m_list_bit.clear();

    /*
    // Clearup value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for ( iterValue = m_list_value.begin();
    iterValue != m_list_value.end();
    ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if ( NULL != pRecordValue ) {
    delete pRecordValue;
    }
    }
    */
    m_list_value.clear();
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

void
CMDF_Action::clearStorage(void)
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
    m_list_ActionParameter.clear();
    */
    m_list_ActionParameter.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_DecisionMatrix::CMDF_DecisionMatrix()
{
    m_nLevel       = 1;
    m_nStartPage   = 0;
    m_nStartOffset = 0;
    m_nRowCount    = 0;
    m_nRowSize     = 8;
    m_bIndexed     = false;
}

CMDF_DecisionMatrix::~CMDF_DecisionMatrix()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

void
CMDF_DecisionMatrix::clearStorage()
{
    m_nLevel       = 1;
    m_nStartPage   = 0;
    m_nStartOffset = 0;
    m_nRowCount    = 0;
    m_nRowSize     = 8;

    m_list_action.clear();
    m_list_action.clear();
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

void
CMDF_EventData::clearStorage()
{
    // Clearup bit list
    std::deque<CMDF_Bit *>::iterator iterBit;
    for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
        CMDF_Bit *pRecordBit = *iterBit;
        if (NULL != pRecordBit) {
            delete pRecordBit;
        }
    }

    // Clearup value list
    std::deque<CMDF_ValueListValue *>::iterator iterValue;
    for (iterValue = m_list_value.begin(); iterValue != m_list_value.end();
         ++iterValue) {
        CMDF_ValueListValue *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
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

void
CMDF_Event::clearStorage()
{
    // Cleanup event data
    std::deque<CMDF_EventData *>::iterator iterEventData;
    for (iterEventData = m_list_eventdata.begin();
         iterEventData != m_list_eventdata.end();
         ++iterEventData) {
        CMDF_EventData *pRecordEventData = *iterEventData;
        if (NULL != pRecordEventData) {
            delete pRecordEventData;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Item::CMDF_Item() {}

CMDF_Item::~CMDF_Item() {}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_BootLoaderInfo::CMDF_BootLoaderInfo() {}

CMDF_BootLoaderInfo::~CMDF_BootLoaderInfo() {}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_BootLoaderInfo::clearStorage(void)
{
    m_nAlgorithm  = 0;
    m_nBlockSize  = 0;
    m_nBlockCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Address::CMDF_Address() {}

CMDF_Address::~CMDF_Address()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Address::clearStorage(void)
{
    m_strStreet.clear();
    m_strTown.clear();
    m_strCity.clear();
    m_strPostCode.clear();
    m_strState.clear();
    m_strRegion.clear();
    m_strCountry.clear();
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

void
CMDF_Manufacturer::clearStorage(void)
{
    // Cleanup Address list
    std::deque<CMDF_Address *>::iterator iterAddress;
    for (iterAddress = m_list_Address.begin();
         iterAddress != m_list_Address.end();
         ++iterAddress) {
        CMDF_Address *pRecordAddress = *iterAddress;
        if (NULL != pRecordAddress) {
            delete pRecordAddress;
        }
    }

    // Cleanup Phone list
    std::deque<CMDF_Item *>::iterator iterPhone;
    for (iterPhone = m_list_Phone.begin(); iterPhone != m_list_Phone.end();
         ++iterPhone) {
        CMDF_Item *pRecordPhone = *iterPhone;
        if (NULL != pRecordPhone) {
            delete pRecordPhone;
        }
    }

    // Cleanup Fax list
    std::deque<CMDF_Item *>::iterator iterFax;
    for (iterFax = m_list_Fax.begin(); iterFax != m_list_Fax.end(); ++iterFax) {
        CMDF_Item *pRecordFax = *iterFax;
        if (NULL != pRecordFax) {
            delete pRecordFax;
        }
    }

    // Cleanup mail list
    std::deque<CMDF_Item *>::iterator iterEmail;
    for (iterEmail = m_list_Email.begin(); iterEmail != m_list_Email.end();
         ++iterEmail) {
        CMDF_Item *pRecordEmail = *iterEmail;
        if (NULL != pRecordEmail) {
            delete pRecordEmail;
        }
    }

    // Cleanup web list
    std::deque<CMDF_Item *>::iterator iterweb;
    for (iterweb = m_list_Web.begin(); iterweb != m_list_Web.end(); ++iterweb) {
        CMDF_Item *pRecordWeb = *iterweb;
        if (NULL != pRecordWeb) {
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

CMDF_Firmware::~CMDF_Firmware() {}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Firmware::clearStorage(void)
{
    m_strPath.clear();
    m_size = 0;
    m_description.clear();
    m_version_major    = 0;
    m_version_minor    = 0;
    m_version_subminor = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF::CMDF()
{
    m_strLocale = "en";
}

CMDF::~CMDF()
{
    clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF::clearStorage(void)
{
    // Clean up manufacturer list
    std::deque<CMDF_Manufacturer *>::iterator iterManufacturer;
    for (iterManufacturer = m_list_manufacturer.begin();
         iterManufacturer != m_list_manufacturer.end();
         ++iterManufacturer) {
        CMDF_Manufacturer *pRecordManufacturer = *iterManufacturer;
        if (NULL != pRecordManufacturer) {
            delete pRecordManufacturer;
        }
    }

    // Cleanup node event list
    std::deque<CMDF_Event *>::iterator iterEvent;
    for (iterEvent = m_list_event.begin(); iterEvent != m_list_event.end();
         ++iterEvent) {
        CMDF_Event *pRecordEvent = *iterEvent;
        if (NULL != pRecordEvent) {
            delete pRecordEvent;
        }
    }

    // Clean up register list
    std::deque<CMDF_Register *>::iterator iterRegister;
    for (iterRegister = m_list_register.begin();
         iterRegister != m_list_register.end();
         ++iterRegister) {
        CMDF_Register *pRecordRegister = *iterRegister;
        if (NULL != pRecordRegister) {
            delete pRecordRegister;
        }
    }

    // Clean up abstraction list
    std::deque<CMDF_Abstraction *>::iterator iterAbstraction;
    for (iterAbstraction = m_list_abstraction.begin();
         iterAbstraction != m_list_abstraction.end();
         ++iterAbstraction) {
        CMDF_Abstraction *pRecordAbstraction = *iterAbstraction;
        if (NULL != pRecordAbstraction) {
            delete pRecordAbstraction;
        }
    }

    // Clear alarm bit list
    std::deque<CMDF_Bit *>::iterator iterAlarmBits;
    for (iterAlarmBits = m_list_alarmbits.begin();
         iterAlarmBits != m_list_alarmbits.end();
         ++iterAlarmBits) {
        CMDF_Bit *pRecordAlarmBits = *iterAlarmBits;
        if (NULL != pRecordAlarmBits) {
            delete pRecordAlarmBits;
        }
    }

    m_strModule_Name.clear();
    m_strModule_Model.clear();
    m_strModule_Version.clear();
    m_strModule_Description.clear();
    m_strModule_InfoURL.clear();
    m_changeDate.clear();
    m_Module_buffersize = 0;
    m_strURL.clear();
    m_list_manufacturer.clear();
    m_dmInfo.clearStorage();
    m_bootInfo.clearStorage();
    m_list_event.clear();
    m_list_register.clear();
    m_list_abstraction.clear();
    m_list_alarmbits.clear();

    m_firmware.clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  downLoadMDF
//

bool
CMDF::downLoadMDF(std::string &remoteFile, std::string &tempFileName)
{
    /* TODO
    char buf[ 64000 ];
    size_t cnt;
    xxFile tempFile;

    tempFileName = wxFileName::CreateTempFileName( "mdf", &tempFile );
    if ( 0 == tempFileName.length() ) {
        return false;
    }

    m_tempFileName = tempFileName;

    // Create and open http stream
    xxURL url( remoteFile );
    if ( xxURL_NOERR != url.GetError() ) {
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
*/
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  load
//

bool
CMDF::load(std::string &remoteFile, bool bLocalFile)
{
    std::string localFile = remoteFile;

    if (remoteFile.npos == remoteFile.find("http://")) {
        std::string str;
        str = "http://";
        str += remoteFile;
        remoteFile = str;
    }

    // Get URL from user if not given
    if (!bLocalFile && !remoteFile.length()) {
        return false;
    }
    // Get filename from user if not given
    else if (bLocalFile && !remoteFile.length()) {
        return false;
    }

    // Must have a path at this point
    if (0 == remoteFile.length()) {
        return false;
    }

    if (!bLocalFile) {

        if (!downLoadMDF(remoteFile, localFile)) {
            return false;
        }
    }

    return parseMDF(localFile);
}

///////////////////////////////////////////////////////////////////////////////
//  mdfDescriptionFormat
//

void
CMDF::mdfDescriptionFormat(std::string &str)
{
    bool bEscape = false;
    bool bSpace  = true; // Remove leading space
    std::string strWork;
    unsigned int i;

    for (i = 0; i < str.length(); i++) {

        if (bSpace && (' ' != str[i])) bSpace = false;

        if (bEscape) {

            switch (str[i]) {

                case '\\':
                    strWork += str[i];
                    break;

                case 'n':
                    strWork += '\n';
                    bSpace = true; // Don't allow a space to start line
                    break;
            }

            bEscape = false;

        } else {
            switch (str[i]) {

                case '\n':
                case '\r':
                case '\t':
                    break;

                case '\\':
                    bEscape = true;
                    break;

                    // We allow only one space
                case ' ':
                    if (!bSpace) {
                        strWork += str[i];
                        bSpace = true;
                    }
                    break;

                default:
                    strWork += str[i];
            }
        }
    }

    str = strWork;

    int cnt = 0;

    strWork.clear();

    // Break apart line that are longer then 80 chars
    for (i = 0; i < str.length(); i++) {

        if ('\n' == str[i]) {
            cnt = 0;
        }

        cnt++;

        if (cnt > 80) {

            if (' ' == str[i]) {

                // Add line break
                strWork += '\n';

                cnt = 0;
            } else {
                strWork += str[i];
            }

        } else {
            strWork += str[i];
        }
    }

    str = strWork;
}

///////////////////////////////////////////////////////////////////////////////
//  parseMDF
//

bool
CMDF::parseMDF(std::string &path)
{
    /* TODO
       bool rv = true;
       wxXmlDocument doc;

       // Empty old MDF information
       clearStorage();

       if ( !doc.Load( path ) ) {
           return false;
       }

       // start processing the XML file
       if ( doc.GetRoot()->GetName() != ("vscp") ) {
           return false;
       }

       wxXmlNode *child1 = doc.GetRoot()->GetChildren();
       while ( child1 ) {

           // One module
           if ( child1->GetName() == ("module") ) {

               wxXmlNode *child2 = child1->GetChildren();
               while ( child2 ) {

                   if ( child2->GetName() == "name" ) {
                       m_strModule_Name =
       child2->GetNodeContent().ToStdString();
                   }
                   else if ( child2->GetName() == "model" ) {
                       m_strModule_Model =
       child2->GetNodeContent().ToStdString();
                   }
                   else if ( child2->GetName() == "version" ) {
                       m_strModule_Version =
       child2->GetNodeContent().ToStdString();
                   }
                   else if ( child2->GetName() == "description" ) {
                       std::string str;

                       str = child2->GetAttribute( "lang", "en" ).ToStdString();

                       if ( str == m_strLocale ) {
                           m_strModule_Description =
       child2->GetNodeContent().ToStdString();
                       }
                       else if ( ( "en" == str ) &&
       m_strModule_Description.empty() ) { m_strModule_Description =
       child2->GetNodeContent().ToStdString();
                       }
                   }
                   else if ( child2->GetName() == "infourl" ) {
                       m_strModule_InfoURL =
       child2->GetNodeContent().ToStdString();
                   }
                   else if ( child2->GetName() == "changed" ) {
                       m_changeDate = child2->GetNodeContent().ToStdString();
                   }
                   else if ( child2->GetName() == "buffersize" ) {
                       m_Module_buffersize = vscp_readStringValue(
       child2->GetNodeContent().ToStdString() );
                   }
                   else if ( child2->GetName() == "manufacturer" ) {

                       CMDF_Manufacturer *pmanufacturer = new CMDF_Manufacturer;
                       wxASSERT( NULL != pmanufacturer );
                       m_list_manufacturer.push_back( pmanufacturer );

                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == "name" ) {
                               pmanufacturer->m_strName =
       child3->GetNodeContent().ToStdString();
                           }
                           else if ( child3->GetName() == "address" ) {

                               CMDF_Address *pAddress = new CMDF_Address;
                               wxASSERT( NULL != pAddress );
                               pmanufacturer->m_list_Address.push_back( pAddress
       );

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == "street" ) {
                                       pAddress->m_strStreet =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "town" ) {
                                       pAddress->m_strTown =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "city" ) {
                                       pAddress->m_strCity =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "postcode" ) {
                                       pAddress->m_strPostCode =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "state" ) {
                                       pAddress->m_strState =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "region" ) {
                                       pAddress->m_strRegion =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "country" ) {
                                       pAddress->m_strCountry =
       child4->GetNodeContent().ToStdString();
                                   }

                                   child4 = child4->GetNext();

                               } // child4

                           } // address
                           else if ( child3->GetName() == "telephone" ) {

                               CMDF_Item *pPhone = new CMDF_Item;
                               wxASSERT( NULL != pPhone );
                               pmanufacturer->m_list_Phone.push_back( pPhone );

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == "number" ) {
                                       pPhone->m_strItem =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "description"
       ) { std::string str;

                                       str = child4->GetAttribute( "lang", "en"
       ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pPhone->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( "en" == str ) &&
       pPhone->m_strDescription.empty() ) { pPhone->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }

                                   child4 = child4->GetNext();

                               } // child 4

                           } // telephone
                           else if ( child3->GetName() == "fax" ) {

                               CMDF_Item *pFax = new CMDF_Item;
                               wxASSERT( NULL != pFax );
                               pmanufacturer->m_list_Fax.push_back( pFax );

                               wxXmlNode *child4 = child3->GetChildren();
                               while ( child4 ) {

                                   if ( child4->GetName() == "number" ) {
                                       pFax->m_strItem =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "description"
       ) { std::string str;

                                       str = child4->GetAttribute( "lang", "en"
       ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pFax->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( "en" == str ) &&
       pFax->m_strDescription.empty() ) { pFax->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }

                                   child4 = child4->GetNext();

                               } // Child 4
                           } // fax
                           else if ( child3->GetName() == "email" ) {

                               CMDF_Item *pEmail = new CMDF_Item;
                               wxASSERT( NULL != pEmail );
                               pmanufacturer->m_list_Email.push_back( pEmail );

                               wxXmlNode *child4 = child3->GetChildren();
                               while ( child4 ) {

                                   if ( child4->GetName() == "address" ) {
                                       pEmail->m_strItem =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "description"
       ) { std::string str;

                                       str = child4->GetAttribute( "lang", "en"
       ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pEmail->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( "en" == str ) &&
       pEmail->m_strDescription.empty() ) { pEmail->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }

                                   child4 = child4->GetNext();

                               } // child 4

                           } // email
                           else if ( child3->GetName() == "web" ) {

                               CMDF_Item *pWeb = new CMDF_Item;
                               wxASSERT( NULL != pWeb );
                               pmanufacturer->m_list_Web.push_back( pWeb );

                               wxXmlNode *child4 = child3->GetChildren();
                               while ( child4 ) {

                                   if ( child4->GetName() == "address" ) {
                                       pWeb->m_strItem =
       child4->GetNodeContent().ToStdString();
                                   }
                                   else if ( child4->GetName() == "description"
       ) { std::string str;

                                       str = child4->GetAttribute( "lang", "en"
       ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pWeb->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( "en" == str ) &&
       pWeb->m_strDescription.empty() ) { pWeb->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }

                                   child4 = child4->GetNext();

                               } // child 4

                           } // web

                           child3 = child3->GetNext();

                       } // while child3

                   } // manufacturer
                   else if ( child2->GetName() == "firmware" ) {


                       m_firmware.m_strPath = child2->GetAttribute( "path", ""
       ).ToStdString(); m_firmware.m_size  = vscp_readStringValue(
       child2->GetAttribute( "size", "0" ).ToStdString() ); std::string format =
       child2->GetAttribute( "format", "intelhex8" ).ToStdString();
                       m_firmware.m_version_major  = vscp_readStringValue(
       child2->GetAttribute( "version_major", "0" ).ToStdString() );
                       m_firmware.m_version_minor  = vscp_readStringValue(
       child2->GetAttribute( "version_minor", "0" ).ToStdString() );
                       m_firmware.m_version_subminor  = vscp_readStringValue(
       child2->GetAttribute( "version_subminor", "0" ).ToStdString() );

                       wxXmlNode *child3 = child2->GetChildren();
                       while ( child3 ) {

                           std::string str;
                           str = child3->GetAttribute( "lang", "en"
       ).ToStdString();

                           if ( str == m_strLocale ) {
                               m_firmware.m_description =
       child3->GetNodeContent().ToStdString();
                           }
                           else if ( ( "en" == str ) &&
       m_strModule_Description.empty() ) { m_firmware.m_description =
       child3->GetNodeContent().ToStdString();
                           }

                           child3 = child3->GetNext();

                       } // child3

                   }  // firmware




                   else if ( child2->GetName() == "abstractions" ) {

                       wxXmlNode *child3 = child2->GetChildren();
                       while ( child3 ) {

                           if ( child3->GetName() == "abstraction" ) {

                               CMDF_Abstraction *pAbstraction = new
       CMDF_Abstraction; wxASSERT( NULL != pAbstraction );
                               m_list_abstraction.push_back( pAbstraction );


                               pAbstraction->m_strID =  child3->GetAttribute(
       "id", "" ).ToStdString(); pAbstraction->m_strDefault =
       child3->GetAttribute( "default", "" ).ToStdString();
                               pAbstraction->m_nPage =  vscp_readStringValue(
       child3->GetAttribute( "page", "0" ).ToStdString() );
                               pAbstraction->m_nOffset =  vscp_readStringValue(
       child3->GetAttribute( "offset", "0" ).ToStdString() );
                               pAbstraction->m_nBitnumber =
       vscp_readStringValue( child3->GetAttribute( "bit" , "0" ).ToStdString()
       ); pAbstraction->m_nWidth =  vscp_readStringValue( child3->GetAttribute(
       "width", "0" ).ToStdString() ); pAbstraction->m_nMax =
       vscp_readStringValue( child3->GetAttribute( "max" ,"0" ).ToStdString() );
                               pAbstraction->m_nMin =  vscp_readStringValue(
       child3->GetAttribute( "min" , "0" ).ToStdString() ); std::string stridx =
       child3->GetAttribute( "indexed", "false" ).ToStdString();
                               pAbstraction->m_fgcolor = wxUINT32_SWAP_ALWAYS(
       vscp_readStringValue( child3->GetAttribute( "fgcolor", "0x00000000"
       ).ToStdString() ) ); pAbstraction->m_bgcolor = wxUINT32_SWAP_ALWAYS(
       vscp_readStringValue( child3->GetAttribute( "bgcolor", "0xffffffff"
       ).ToStdString() ) ); pAbstraction->m_fgcolor >>= 8;
                               pAbstraction->m_bgcolor >>= 8;

                               vscp_makeLower( stridx );
                               if ( stridx.npos != stridx.find("true") ) {
                                   pAbstraction->m_bIndexed = true;
                               }
                               else {
                                   pAbstraction->m_bIndexed = false;
                               }

                               std::string strType =  child3->GetAttribute(
       "type", "" ).ToStdString();

                               if ( strType == "string" ) {
                                   pAbstraction->m_nType = type_string;
                               }
                               else if ( strType == "bitfield" ) {
                                   pAbstraction->m_nType = type_bitfield;
                               }
                               else if ( strType == "bool" ) {
                                   pAbstraction->m_nType = type_boolval;
                               }
                               else if ( strType == "char" ) {
                                   pAbstraction->m_nType = type_int8_t;
                               }
                               else if ( strType == "int8_t" ) {
                                   pAbstraction->m_nType = type_int8_t;
                               }
                               else if ( strType == "byte" ) {
                                   pAbstraction->m_nType = type_uint8_t;
                               }
                               else if ( strType == "uint8_t" ) {
                                   pAbstraction->m_nType = type_uint8_t;
                               }
                               else if ( strType == "int16_t" ) {
                                   pAbstraction->m_nType = type_int16_t;
                               }
                               else if ( strType == "short" ) {
                                   pAbstraction->m_nType = type_int16_t;
                               }
                               else if ( strType == "integer" ) {
                                   pAbstraction->m_nType = type_int16_t;
                               }
                               else if ( strType == "uint16_t" ) {
                                   pAbstraction->m_nType = type_uint16_t;
                               }
                               else if ( strType == "int32_t" ) {
                                   pAbstraction->m_nType = type_int32_t;
                               }
                               else if ( strType == "long" ) {
                                   pAbstraction->m_nType = type_int32_t;
                               }
                               else if ( strType == "uint32_t" ) {
                                   pAbstraction->m_nType = type_uint32_t;
                               }
                               else if ( strType == "int64_t" ) {
                                   pAbstraction->m_nType = type_int64_t;
                               }
                               else if ( strType == "uint64_t" ) {
                                   pAbstraction->m_nType = type_uint64_t;
                               }
                               else if ( strType == "double" ) {
                                   pAbstraction->m_nType = type_double;
                               }
                               else if ( strType == "float" ) {
                                   pAbstraction->m_nType = type_float;
                               }
                               else if ( strType == "date" ) {
                                   pAbstraction->m_nType = type_date;
                               }
                               else if ( strType == "time" ) {
                                   pAbstraction->m_nType = type_time;
                               }
                               else if ( strType == "guid" ) {
                                   pAbstraction->m_nType = type_guid;
                               }
                               else if ( strType == "index8_int16_t" ) {
                                   pAbstraction->m_nType = type_index8_int16_t;
                               }
                               else if ( strType == "index8_uint16_t" ) {
                                   pAbstraction->m_nType = type_index8_uint16_t;
                               }
                               else if ( strType == "index8_int32_t" ) {
                                   pAbstraction->m_nType = type_guid;
                               }
                               else if ( strType == "index8_uint32_t" ) {
                                   pAbstraction->m_nType = type_index8_uint32_t;
                               }
                               else if ( strType == "index8_int64_t" ) {
                                   pAbstraction->m_nType = type_index8_int64_t;
                               }
                               else if ( strType == "index8_uint64_t" ) {
                                   pAbstraction->m_nType = type_index8_uint64_t;
                               }
                               else if ( strType == "index8_float" ) {
                                   pAbstraction->m_nType = type_index8_float;
                               }
                               else if ( strType == "index8_double" ) {
                                   pAbstraction->m_nType = type_index8_double;
                               }
                               else if ( strType == "index8_date" ) {
                                   pAbstraction->m_nType = type_index8_date;
                               }
                               else if ( strType == "index8_time" ) {
                                   pAbstraction->m_nType = type_index8_time;
                               }
                               else if ( strType == "index8_guid" ) {
                                   pAbstraction->m_nType = type_index8_guid;
                               }
                               else if ( strType == "index8_string" ) {
                                   pAbstraction->m_nType = type_index8_string;
                               }
                               else {
                                   pAbstraction->m_nType = type_unknown;
                               }

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == "name" ) {
                                       std::string str;
                                       str = child4->GetAttribute( "lang", "en"
       );

                                       if ( str == m_strLocale ) {
                                           pAbstraction->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( "en" == str ) &&
       pAbstraction->m_strName.empty() ) { pAbstraction->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() == "description"
       ) { std::string str; str = child4->GetAttribute( "lang", "en"
       ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pAbstraction->m_strDescription =
       child4->GetNodeContent().ToStdString(); mdfDescriptionFormat(
       pAbstraction->m_strDescription );
                                       }
                                       else if ( ( "en" == str ) &&
       pAbstraction->m_strName.empty() ) { pAbstraction->m_strDescription =
       child4->GetNodeContent().ToStdString(); mdfDescriptionFormat(
       pAbstraction->m_strDescription );
                                       }
                                   }
                                   else if ( child4->GetName() == "help" ) {
                                       std::string str;
                                       str = child4->GetAttribute( "lang", "en"
       );

                                       if ( str == m_strLocale ) {
                                           pAbstraction->m_strHelp =
       child4->GetNodeContent().ToStdString(); pAbstraction->m_strHelpType =
       child4->GetAttribute( "type", "text" ).ToStdString();

                                       }
                                       else if ( ( "en" == str ) ) {
                                           pAbstraction->m_strHelp =
       child4->GetNodeContent().ToStdString(); pAbstraction->m_strHelpType =
       child4->GetAttribute( "type", "text" ).ToStdString();

                                       }
                                   }
                                   else if ( child4->GetName() == "access" ) {

                                       // Set default access
                                       pAbstraction->m_nAccess =
       MDF_ACCESS_READ; std::string strAccessRights =
       child4->GetNodeContent().ToStdString();

                                       vscp_makeUpper( strAccessRights );
                                       if ( strAccessRights.npos !=
       strAccessRights.find("R" ) ) { pAbstraction->m_nAccess |=
       MDF_ACCESS_READ;
                                       }

                                       if ( strAccessRights.npos !=
       strAccessRights.find("W") ) { pAbstraction->m_nAccess |=
       MDF_ACCESS_WRITE;
                                       }

                                   }
                                   else if ( child4->GetName() == "valuelist" )
       {

                                       wxXmlNode *child5 =
       child4->GetChildren(); while (child5) {

                                           if ( child5->GetName() == "item" ) {

                                               CMDF_ValueListValue *pValueItem =
       new CMDF_ValueListValue; wxASSERT( NULL != pValueItem );
                                               pAbstraction->m_list_value.push_back(
       pValueItem ); pValueItem->m_strValue = child5->GetAttribute( "value", ""
       ).ToStdString();

                                               wxXmlNode *child6 =
       child5->GetChildren(); while (child6) {

                                                   if ( child6->GetName() ==
       ("name") ) { std::string str; str = child6->GetAttribute( "lang", "en"
       ).ToStdString();

                                                       if ( str == m_strLocale )
       { pValueItem->m_strName = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( "en" == str )
       && pValueItem->m_strName.empty() ) { pValueItem->m_strName =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == "description" ) { std::string str; str = child6->GetAttribute( "lang",
       "en" ).ToStdString();

                                                       if ( str == m_strLocale )
       { pValueItem->m_strDescription = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( "en" == str )
       && pValueItem->m_strName.empty() ) { pValueItem->m_strDescription =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == "help" ) { std::string str; str = child6->GetAttribute( "lang", "en"
       );

                                                       if ( str == m_strLocale )
       { pValueItem->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pValueItem->m_strHelpType
       = child6->GetAttribute( "type", "text" ).ToStdString();

                                                       }
                                                       else if ( ( "en" == str )
       ) { pValueItem->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pValueItem->m_strHelpType
       = child6->GetAttribute( "type", "text" ).ToStdString();

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
                   else if ( child2->GetName() == "registers" ) {

                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == "reg" ) {

                               std::string strType;
                               CMDF_Register *pRegister = new CMDF_Register;
                               wxASSERT( NULL != pRegister );

                               pRegister->m_rowInGrid = -1; // Set when reg is
       written to grid pRegister->m_nPage = vscp_readStringValue(
       child3->GetAttribute( ( "page" ), ("0") ).ToStdString() );
                               pRegister->m_nOffset = vscp_readStringValue(
       child3->GetAttribute( ( "offset" ), ("0") ).ToStdString() );
                               pRegister->m_nWidth = vscp_readStringValue(
       child3->GetAttribute( ( "width" ), ("8") ).ToStdString() );
                               pRegister->m_size = vscp_readStringValue(
       child3->GetAttribute( ( "size" ), ("1") ).ToStdString() );
                               pRegister->m_nMin = vscp_readStringValue(
       child3->GetAttribute( ( "min" ), ( "0" ) ).ToStdString() );
                               pRegister->m_nMax = vscp_readStringValue(
       child3->GetAttribute( ( "max" ), ("255") ).ToStdString() );
                               pRegister->m_strDefault = child3->GetAttribute( (
       "default" ), ("UNDEF") ).ToStdString(); pRegister->m_fgcolor =
       wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( (
       "fgcolor" ), ( "0x00000000" ) ).ToStdString() ) ); pRegister->m_bgcolor =
       wxUINT32_SWAP_ALWAYS( vscp_readStringValue( child3->GetAttribute( (
       "bgcolor" ), ( "0xffffffff" ) ).ToStdString() ) ); pRegister->m_fgcolor
       >>= 8; pRegister->m_bgcolor >>= 8; strType = child3->GetAttribute( (
       "type" ), ( "std" ) ).ToStdString(); vscp_trim( strType );
                               pRegister->m_type = REGISTER_TYPE_STANDARD;
                               if ( ("dmatrix1") == strType ) {
                                   if ( pRegister->m_size > 1 ) {
                                       pRegister->m_type =
       REGISTER_TYPE_DMATRIX1;
                                   }
                               }
                               else if ( ("block") == strType ) {
                                   if ( pRegister->m_size > 1 ) {
                                       pRegister->m_type = REGISTER_TYPE_BLOCK;
                                   }
                               }

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == ("name") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") );

                                       if ( str == m_strLocale ) {
                                           pRegister->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( ("en") == str ) &&
       pRegister->m_strName.empty() ) { pRegister->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() ==
       ("description") ) { std::string str; str = child4->GetAttribute( ( "lang"
       ), ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pRegister->m_strDescription =
       child4->GetNodeContent().ToStdString(); mdfDescriptionFormat(
       pRegister->m_strDescription );
                                       }
                                       else if ( ( ("en") == str ) &&
       pRegister->m_strName.empty() ) { pRegister->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() == ("access") ) {
                                       pRegister->m_nAccess = MDF_ACCESS_READ;
                                       std::string strAccessRights =
       child4->GetNodeContent().ToStdString();

                                       vscp_makeUpper( strAccessRights );
                                       if ( strAccessRights == "R" ) {
                                           pRegister->m_nAccess |=
       MDF_ACCESS_READ;
                                       }

                                       if ( strAccessRights == "W" ) {
                                           pRegister->m_nAccess |=
       MDF_ACCESS_WRITE;
                                       }

                                   }
                                   else if ( child4->GetName() == ("help") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") );

                                       if ( str == m_strLocale ) {
                                           pRegister->m_strHelp =
       child4->GetNodeContent().ToStdString(); pRegister->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                       else if ( ( ("en") == str ) ) {
                                           pRegister->m_strHelp =
       child4->GetNodeContent().ToStdString(); pRegister->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                   }
                                   // Not allowed for non standard regs
                                   else if ( ( child4->GetName() == ( "bitfield"
       ) ) && ( REGISTER_TYPE_STANDARD== pRegister->m_type ) ) {

                                       wxXmlNode *child5 =
       child4->GetChildren(); while (child5) {

                                           if ( child5->GetName() == ("bit") ) {

                                               CMDF_Bit *pBit = new CMDF_Bit;
                                               wxASSERT( NULL != pBit );
                                               pRegister->m_list_bit.push_back(
       pBit ); pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( (
       "pos" ), ("0") ).ToStdString() ); pBit->m_nWidth = vscp_readStringValue(
       child5->GetAttribute( ( "width" ), ("0") ).ToStdString() ); pBit->m_nPos
       = vscp_readStringValue( child5->GetAttribute( ( "default" ), ("0")
       ).ToStdString() );


                                               wxXmlNode *child6 =
       child5->GetChildren(); while (child6) {

                                                   if ( child6->GetName() ==
       ("name") ) { std::string str; str = child6->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pBit->m_strName = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pBit->m_strName.empty() ) { pBit->m_strName =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("description") ) { std::string str; str = child6->GetAttribute( (
       "lang" ), ("en") );

                                                       if ( str == m_strLocale )
       { pBit->m_strDescription = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pBit->m_strName.empty() ) { pBit->m_strDescription =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("help") ) { std::string str; str = child6->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pBit->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pBit->m_strHelpType =
       child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                       }
                                                       else if ( ( ("en") == str
       ) ) { pBit->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pBit->m_strHelpType =
       child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("access") ) {

                                                       pBit->m_nAccess =
       MDF_ACCESS_READ; std::string strAccessRights =
       child6->GetNodeContent().ToStdString();

                                                       vscp_makeUpper(
       strAccessRights ); if ( strAccessRights.npos !=
       strAccessRights.find(("R") ) ) { pBit->m_nAccess |= MDF_ACCESS_READ;
                                                       }

                                                       if ( strAccessRights.npos
       != strAccessRights.find(("W")) ) { pBit->m_nAccess |= MDF_ACCESS_WRITE;
                                                       }

                                                   }

                                                   child6 = child6->GetNext();

                                               } // while 6

                                           }

                                           child5 = child5->GetNext();

                                       } // while 5

                                   }
                                   // Not allowed for non standard regs
                                   else if ( ( child4->GetName() == (
       "valuelist" ) ) && ( REGISTER_TYPE_STANDARD == pRegister->m_type ) ) {

                                       wxXmlNode *child5 =
       child4->GetChildren(); while (child5) {

                                           if ( child5->GetName() == ("item") )
       {

                                               CMDF_ValueListValue *pValueItem =
       new CMDF_ValueListValue; wxASSERT( NULL != pValueItem );
                                               pRegister->m_list_value.push_back(
       pValueItem ); pValueItem->m_strValue = child5->GetAttribute( ( "value" ),
       ("") ).ToStdString();


                                               wxXmlNode *child6 =
       child5->GetChildren(); while (child6) {

                                                   if ( child6->GetName() ==
       ("name") ) { std::string str; str = child6->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pValueItem->m_strName = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pValueItem->m_strName.empty() ) { pValueItem->m_strName =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("description") ) { std::string str; str = child6->GetAttribute( (
       "lang" ), ("en") );

                                                       if ( str == m_strLocale )
       { pValueItem->m_strDescription = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pValueItem->m_strName.empty() ) { pValueItem->m_strDescription =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("help") ) { std::string str; str = child6->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pValueItem->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pValueItem->m_strHelpType
       = child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                       }
                                                       else if ( ( ("en") == str
       ) ) { pValueItem->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pValueItem->m_strHelpType
       = child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

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

                               // If register type is not 'std' then we need to
       add more registers
                               // at this point accoring to the 'size' attribute
                               if ( REGISTER_TYPE_DMATRIX1 == pRegister->m_type
       ) {

                                   for ( int idx = 0; idx < pRegister->m_size;
       idx++ ) { std::string str; CMDF_Register *pRegisterExt = new
       CMDF_Register; wxASSERT( NULL != pRegisterExt ); *pRegisterExt =
       *pRegister; pRegisterExt->m_size = 1; pRegisterExt->m_nOffset += idx;

                                       switch ( idx % 8 ) {
                                           case 0: // originating address
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Oaddr" ), 1+idx/8 );
                                               pRegisterExt->m_strDescription =
       ("Originating address.\nSet to nickname for node that should trigger
       action here. Oaddr is the originating address.\nMeans we are only
       interested in messages from the node given here.\n0x00 is segment
       controller and 0xff is a node without a nickname.\nIf bit 6 of flags is
       set oaddr will not be checked and events from all nodes will be
       accepted."); break; case 1: // flags pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Flags" ), 1+idx/8 );
                                               pRegisterExt->m_strDescription =
       ( "Flags. Set selection behaviour.\nThe enable bit can be used to disable
       a decion matrix row while it is edited.\nThe zone and use subzone bits
       can be activated to have a check on the zone / subzone information\nof an
       event.That is the zone / subzone of the machine must match the one of the
       event to trigger\nthe DM row." ); break; case 2: // class mask
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Class mask (low eight
       bits)" ), 1+idx/8 ); pRegisterExt->m_strDescription = ( "The lowest eight
       bits of the class mask that defines the events that should trigger the
       action of\nthis decision matrix row. Bit 8 of the class mask is stored in
       bit 1 of the flag byte." ); break; case 3: // class filter
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Class filter (low eight
       bits)" ), 1+idx/8 ); pRegisterExt->m_strDescription = ( "The lowest eight
       bits of the class filter that defines the events that should trigger the
       action of\nthis decision matrix row. Bit 8 of the class filter is stored
       in bit 1 of the flag byte." ); break; case 4: // type mask
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Type mask" ), 1+idx/8 );
                                               pRegisterExt->m_strDescription =
       ( "Type mask that defines the events that should trigger the action of
       this decision matrix row." ); break; case 5: // type filter
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Type filter" ), 1+idx/8 );
                                               pRegisterExt->m_strDescription =
       ( "Type filter that defines the events that should trigger the action of
       this decision matrix row." ); break; case 6: // action
                                               pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Action" ), 1+idx/8 );
                                               pRegisterExt->m_strDescription =
       ( "This is the action or operation that should be performed if the
       filtering is satisfied.\nOnly action code 0x00 is predefined and means
       No-Operation.\nAll other codes are application specific and typical
       application defined codes could do\nmeasurement, send predefined event
       etc." ); break; case 7: // action parameter pRegisterExt->m_strName =
       vscp_str_format( ( "Decision matrix row %d: Action parameter" ),
       1+idx/8 ); pRegisterExt->m_strDescription = ( "A numeric action parameter
       can be set and its meaning is application specific." ); break;
                                       }

                                       // push_back the register
                                       m_list_register.push_back( pRegisterExt
       );

                                   }

                                   // Detete the placeholder
                                   delete pRegister;

                               }
                               else if ( REGISTER_TYPE_BLOCK ==
       pRegister->m_type ) {

                                   for ( int idx = 0; idx < pRegister->m_size;
       idx++ ) { CMDF_Register *pRegisterExt = new CMDF_Register; wxASSERT( NULL
       != pRegisterExt ); *pRegisterExt = *pRegister; pRegisterExt->m_size = 1;
                                       pRegisterExt->m_nOffset += idx;
                                       pRegisterExt->m_strName =
       vscp_str_format( ( "%s%d" ), pRegisterExt->m_strName.c_str(), idx );
                                       // Append the register
                                       m_list_register.push_back( pRegisterExt
       );
                                   }

                                   // Detete the placeholder
                                   delete pRegister;

                               }
                               else {
                                   m_list_register.push_back( pRegister );
                               }

                           } // Reg

                           child3 = child3->GetNext();

                       } // while 3

                   }
                   else if ( child2->GetName() == ("dmatrix") ) {

                       //
                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == ("level") ) {
                               m_dmInfo.m_nLevel = vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
                           }
                           else if ( child3->GetName() == ("start") ) {
                               m_dmInfo.m_nStartPage =  vscp_readStringValue(
       child3->GetAttribute( "page", "0" ).ToStdString() );
                               m_dmInfo.m_nStartOffset = vscp_readStringValue(
       child3->GetAttribute( "offset", "0" ).ToStdString() ); std::string stridx
       = child3->GetAttribute( "indexed", "false" ).ToStdString();


                               vscp_makeLower( stridx );
                               if ( stridx.npos != stridx.find(("true")) ) {
                                   m_dmInfo.m_bIndexed = true;
                               }
                               else {
                                   m_dmInfo.m_bIndexed = false;
                               }
                           }
                           else if ( child3->GetName() == ("rowcnt") ) {
                               m_dmInfo.m_nRowCount = vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
                           }
                           else if ( child3->GetName() == ("rowsize") ) {
                               m_dmInfo.m_nRowSize = vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
                           }
                           else if ( child3->GetName() == ("action") ) {

                               CMDF_Action *pAction = new CMDF_Action;
                               wxASSERT( NULL != pAction );
                               m_dmInfo.m_list_action.push_back( pAction );

                               // Action parameters
                               pAction->m_nCode = vscp_readStringValue(
       child3->GetAttribute( ( "code" ), ("0") ).ToStdString() );


                               wxXmlNode *child4 = child3->GetChildren();
                               while ( child4 ) {

                                   if ( child4->GetName() == ("name") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pAction->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( ("en") == str ) &&
       pAction->m_strName.empty() ) { pAction->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() ==
       ("description") ) { std::string str; str = child4->GetAttribute( ( "lang"
       ), ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pAction->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( ("en") == str ) &&
       pAction->m_strName.empty() ) { pAction->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() == ("help") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pAction->m_strHelp =
       child4->GetNodeContent().ToStdString(); pAction->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                       else if ( ( ("en") == str ) ) {
                                           pAction->m_strHelp =
       child4->GetNodeContent().ToStdString(); pAction->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                   }
                                   else if ( child4->GetName() == ("param") ) {

                                       CMDF_ActionParameter *pActionParameter =
       new CMDF_ActionParameter; wxASSERT( NULL != pActionParameter );
                                       pAction->m_list_ActionParameter.push_back(
       pActionParameter );

                                       wxXmlNode *child5 =
       child4->GetChildren(); while ( child5 ) {

                                           if ( child5->GetName() == ("name") )
       { std::string str; str = child5->GetAttribute( ( "lang" ), ("en")
       ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pActionParameter->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( ("en") == str ) &&
       pActionParameter->m_strName.empty() ) { pActionParameter->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("description") ) { std::string str; str = child5->GetAttribute( ( "lang"
       ), ("en") ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pActionParameter->m_strDescription
       = child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( ("en") == str ) &&
       pActionParameter->m_strName.empty() ) {
                                                   pActionParameter->m_strDescription
       = child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("help") ) { std::string str; str = child5->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pActionParameter->m_strHelp =
       child5->GetNodeContent().ToStdString(); pActionParameter->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                               else if ( ( ("en") == str ) ) {
                                                   pActionParameter->m_strHelp =
       child5->GetNodeContent().ToStdString(); pActionParameter->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("bit") ) {

                                               CMDF_Bit *pBit = new CMDF_Bit;
                                               wxASSERT( NULL != pBit );
                                               pActionParameter->m_list_bit.push_back(
       pBit ); pBit->m_nPos = vscp_readStringValue( child5->GetAttribute( (
       "pos" ), ("0") ).ToStdString() ); pBit->m_nWidth = vscp_readStringValue(
       child5->GetAttribute( ( "width" ), ("0") ).ToStdString() );
                                               pBit->m_nDefault =
       vscp_readStringValue( child5->GetAttribute( ( "default" ), ("0")
       ).ToStdString() ); pBit->m_nAccess = vscp_readStringValue(
       child5->GetAttribute( ( "access" ), ("0") ).ToStdString() );


                                               wxXmlNode *child6 =
       child5->GetChildren(); while ( child6 ) {

                                                   if ( child6->GetName() ==
       ("name") ) { std::string str; str = child6->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pBit->m_strName = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pBit->m_strName.empty() ) { pBit->m_strName =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("description") ) { std::string str; str = child6->GetAttribute( (
       "lang" ), ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pBit->m_strDescription = child6->GetNodeContent().ToStdString();
                                                       }
                                                       else if ( ( ("en") == str
       ) && pBit->m_strName.empty() ) { pBit->m_strDescription =
       child6->GetNodeContent().ToStdString();
                                                       }
                                                   }
                                                   else if ( child6->GetName()
       == ("help") ) { std::string str;

                                                       str =
       child6->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                       if ( str == m_strLocale )
       { pBit->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pBit->m_strHelpType =
       child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                       }
                                                       else if ( ( ("en") == str
       ) ) { pBit->m_strHelp = child6->GetNodeContent().ToStdString();
                                                           pBit->m_strHelpType =
       child6->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                       }
                                                   }

                                                   child6 = child6->GetNext();

                                               } // while 6

                                           }
                                           else if ( child5->GetName() ==
       ("valuelist") ) {

                                               wxXmlNode *child6 =
       child5->GetChildren(); while ( child6 ) {

                                                   if ( child6->GetName() ==
       ("item") ) {

                                                       CMDF_ValueListValue
       *pValue = new CMDF_ValueListValue; wxASSERT( NULL != pValue );
                                                       pActionParameter->m_list_value.push_back(
       pValue ); pValue->m_strValue = child6->GetAttribute( ( "value" ), ("0")
       ).ToStdString();


                                                       wxXmlNode *child7 =
       child6->GetChildren(); while ( child7 ) {

                                                           if (
       child7->GetName() == ("name") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValue->m_strName =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pValue->m_strName.empty() ) { pValue->m_strName =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("description") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValue->m_strDescription =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pValue->m_strName.empty() ) { pValue->m_strDescription
       = child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child6->GetName() == ("help") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValue->m_strHelp =
       child7->GetNodeContent().ToStdString(); pValue->m_strHelpType =
       child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                               else if ( (
       ("en") == str ) ) { pValue->m_strHelp =
       child7->GetNodeContent().ToStdString(); pValue->m_strHelpType =
       child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                           }

                                                           child7 =
       child7->GetNext();

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
                   else if ( child2->GetName() == ("events") ) {

                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == ("event") ) {

                               CMDF_Event *pEvent = new CMDF_Event;
                               wxASSERT( NULL != pEvent );
                               m_list_event.push_back( pEvent );
                               pEvent->m_nClass = vscp_readStringValue(
       child3->GetAttribute( ( "class" ), ("0") ).ToStdString() );
                               pEvent->m_nType = vscp_readStringValue(
       child3->GetAttribute( ( "type" ), ("0") ).ToStdString() );
                               pEvent->m_nPriority = vscp_readStringValue(
       child3->GetAttribute( ( "priority" ), ("0") ).ToStdString() );

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == ("name") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pEvent->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( ("en") == str ) &&
       pEvent->m_strName.empty() ) { pEvent->m_strName =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() ==
       ("description") ) { std::string str; str = child4->GetAttribute( ( "lang"
       ), ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pEvent->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                       else if ( ( ("en") == str ) &&
       pEvent->m_strName.empty() ) { pEvent->m_strDescription =
       child4->GetNodeContent().ToStdString();
                                       }
                                   }
                                   else if ( child4->GetName() == ("help") ) {
                                       std::string str;
                                       str = child4->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                       if ( str == m_strLocale ) {
                                           pEvent->m_strHelp =
       child4->GetNodeContent().ToStdString(); pEvent->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                       else if ( ( ("en") == str ) ) {
                                           pEvent->m_strHelp =
       child4->GetNodeContent().ToStdString(); pEvent->m_strHelpType =
       child4->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                       }
                                   }
                                   else if ( child4->GetName() == ("data") ) {

                                       CMDF_EventData *pEventData = new
       CMDF_EventData; wxASSERT( NULL != pEventData );
                                       pEvent->m_list_eventdata.push_back(
       pEventData ); pEventData->m_nOffset = vscp_readStringValue(
       child4->GetAttribute( ( "offset" ), ("0") ).ToStdString() );


                                       wxXmlNode *child5 =
       child4->GetChildren(); while (child5) {

                                           if ( child4->GetName() == ("name") )
       { std::string str; str = child5->GetAttribute ( ( "lang" ), ("en")
       ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pEventData->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( ("en") == str ) &&
       pEventData->m_strName.empty() ) { pEventData->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child4->GetName() ==
       "description" ) { std::string str; str = child5->GetAttribute( "lang",
       "en" ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pEventData->m_strDescription
       = child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( "en" == str ) &&
       pEventData->m_strName.empty() ) { pEventData->m_strDescription =
       child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child4->GetName() ==
       ("help") ) { std::string str; str = child5->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pEventData->m_strHelp =
       child5->GetNodeContent().ToStdString(); pEventData->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                               else if ( ( ("en") == str ) ) {
                                                   pEventData->m_strHelp =
       child5->GetNodeContent().ToStdString(); pEventData->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("valuelist") ) {

                                               wxXmlNode *child6 =
       child5->GetChildren(); while (child6) {

                                                   if ( child6->GetName() ==
       ("item") ) {

                                                       CMDF_ValueListValue
       *pValueItem = new CMDF_ValueListValue; wxASSERT( NULL != pValueItem );
                                                       pEventData->m_list_value.push_back(
       pValueItem ); pValueItem->m_strValue = child6->GetAttribute( ( "value" ),
       ("") ).ToStdString();


                                                       wxXmlNode *child7 =
       child5->GetChildren(); while (child7) {

                                                           if (
       child7->GetName() == ("name") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValueItem->m_strName =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pValueItem->m_strName.empty() ) {
                                                                   pValueItem->m_strName
       = child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("description") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValueItem->m_strDescription =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pValueItem->m_strName.empty() ) {
                                                                   pValueItem->m_strDescription
       = child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("help") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pValueItem->m_strHelp =
       child7->GetNodeContent().ToStdString(); pValueItem->m_strHelpType =
       child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                               else if ( (
       ("en") == str ) ) { pValueItem->m_strHelp =
       child7->GetNodeContent().ToStdString(); pValueItem->m_strHelpType =
       child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                           }

                                                           child7 =
       child7->GetNext();

                                                       }

                                                   }

                                                   child6 = child6->GetNext();

                                               } // while 6

                                           }
                                           else if ( child5->GetName() ==
       ("bitfield") ) {

                                               wxXmlNode *child6 =
       child5->GetChildren(); while (child6) {

                                                   if ( child6->GetName() ==
       ("bit") ) {

                                                       CMDF_Bit *pBit = new
       CMDF_Bit; wxASSERT( NULL != pBit ); pEventData->m_list_bit.push_back(
       pBit ); pBit->m_nPos = vscp_readStringValue( child6->GetAttribute( "pos",
       "0" ).ToStdString() ); pBit->m_nWidth = vscp_readStringValue(
       child6->GetAttribute( "width", "0" ).ToStdString() ); pBit->m_nPos =
       vscp_readStringValue( child6->GetAttribute( "default", "0"
       ).ToStdString() );


                                                       wxXmlNode *child7 =
       child6->GetChildren(); while (child7) {

                                                           if (
       child7->GetName() == ("name") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pBit->m_strName = child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pBit->m_strName.empty() ) { pBit->m_strName =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("description") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pBit->m_strDescription =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                               else if ( (
       ("en") == str ) && pBit->m_strName.empty() ) { pBit->m_strDescription =
       child7->GetNodeContent().ToStdString();
                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("help") ) { std::string str; str =
       child7->GetAttribute( ( "lang" ), ("en") ).ToStdString();

                                                               if ( str ==
       m_strLocale ) { pBit->m_strHelp = child7->GetNodeContent().ToStdString();
                                                                   pBit->m_strHelpType
       = child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                               else if ( (
       ("en") == str ) ) { pBit->m_strHelp =
       child7->GetNodeContent().ToStdString(); pBit->m_strHelpType =
       child7->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                                               }
                                                           }
                                                           else if (
       child7->GetName() == ("access") ) {

                                                               pBit->m_nAccess =
       MDF_ACCESS_READ; std::string strAccessRights =
       child7->GetNodeContent().ToStdString();

                                                               vscp_makeUpper(
       strAccessRights ); if ( strAccessRights.npos !=
       strAccessRights.find(("R") ) ) { pBit->m_nAccess |= MDF_ACCESS_READ;
                                                               }

                                                               if (
       strAccessRights.npos != strAccessRights.find(("W")) ) { pBit->m_nAccess
       |= MDF_ACCESS_WRITE;
                                                               }

                                                           }

                                                           child7 =
       child7->GetNext();

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
                   else if ( child2->GetName() == ("alarm") ) {

                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == ("bitfield") ) {

                               wxXmlNode *child4 = child3->GetChildren();
                               while (child4) {

                                   if ( child4->GetName() == ("bit") ) {

                                       CMDF_Bit *pBit = new CMDF_Bit;
                                       wxASSERT( NULL != pBit );
                                       m_list_alarmbits.push_back( pBit );

                                       pBit->m_nPos = vscp_readStringValue(
       child4->GetAttribute( "pos", "0" ).ToStdString() ); pBit->m_nWidth =
       vscp_readStringValue( child4->GetAttribute( "width", "0" ).ToStdString()
       ); pBit->m_nPos = vscp_readStringValue( child4->GetAttribute( "default",
       "0" ).ToStdString() );


                                       wxXmlNode *child5 =
       child4->GetChildren(); while (child5) {

                                           if ( child5->GetName() == ("name") )
       { std::string str; str = child5->GetAttribute( ( "lang" ), ("en")
       ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pBit->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( ("en") == str ) &&
       pBit->m_strName.empty() ) { pBit->m_strName =
       child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("description") ) { std::string str; str = child5->GetAttribute( ( "lang"
       ), ("en") ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pBit->m_strDescription =
       child5->GetNodeContent().ToStdString();
                                               }
                                               else if ( ( ("en") == str ) &&
       pBit->m_strName.empty() ) { pBit->m_strDescription =
       child5->GetNodeContent().ToStdString();
                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("help") ) { std::string str; str = child5->GetAttribute( ( "lang" ),
       ("en") ).ToStdString();

                                               if ( str == m_strLocale ) {
                                                   pBit->m_strHelp =
       child5->GetNodeContent().ToStdString(); pBit->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                               else if ( ( ("en") == str ) ) {
                                                   pBit->m_strHelp =
       child5->GetNodeContent().ToStdString(); pBit->m_strHelpType =
       child5->GetAttribute( ( "type" ), ("text") ).ToStdString();

                                               }
                                           }
                                           else if ( child5->GetName() ==
       ("access") ) {

                                               pBit->m_nAccess =
       MDF_ACCESS_READ; std::string strAccessRights =
       child5->GetNodeContent().ToStdString();

                                               vscp_makeUpper( strAccessRights
       ); if ( strAccessRights.npos != strAccessRights.find(("R") ) ) {
                                                   pBit->m_nAccess |=
       MDF_ACCESS_READ;
                                               }

                                               if ( strAccessRights.npos !=
       strAccessRights.find(("W")) ) { pBit->m_nAccess |= MDF_ACCESS_WRITE;
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
                   else if ( child2->GetName() == ("boot") ) {

                       wxXmlNode *child3 = child2->GetChildren();
                       while (child3) {

                           if ( child3->GetName() == ("algorithm") ) {
                               m_bootInfo.m_nAlgorithm =  vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
                           }
                           else if ( child3->GetName() == ("blocksize") ) {
                               m_bootInfo.m_nBlockSize = vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
                           }
                           else if ( child3->GetName() == ("blockcount") ) {
                               m_bootInfo.m_nBlockSize = vscp_readStringValue(
       child3->GetNodeContent().ToStdString() );
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
       */
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfRegisters
//

uint32_t
CMDF::getNumberOfRegisters(uint32_t page)
{
    uint32_t nregisters = 0;

    std::deque<CMDF_Register *>::iterator iterValue;
    for (iterValue = m_list_register.begin();
         iterValue != m_list_register.end();
         ++iterValue) {
        CMDF_Register *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {
            if (page == pRecordValue->m_nPage) nregisters++;
        }
    }

    return nregisters;
};

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfPages
//

uint32_t
CMDF::getPages(std::set<long> &pages)
{
    // bool bFound;

    std::deque<CMDF_Register *>::iterator iterValue;
    for (iterValue = m_list_register.begin();
         iterValue != m_list_register.end();
         ++iterValue) {

        CMDF_Register *pRecordValue = *iterValue;
        if (NULL != pRecordValue) {

            /*bFound = false;
            for ( uint32_t i=0; i<array.Count(); i++ ) {
                uint16_t ttt = array.Item( i );
                if ( -1 == ttt ) continue;
                if ( pRecordValue->m_nPage == ttt ) {
                    bFound = true;
                    break;
                }
            }*/

            // Add page if not already in set
            if (pages.end() == pages.find(pRecordValue->m_nPage)) {
                pages.insert(pRecordValue->m_nPage);
            }
        }
    }

    return pages.size();
};

///////////////////////////////////////////////////////////////////////////////
//  getMDFRegister
//

CMDF_Register *
CMDF::getMDFRegister(uint8_t reg, uint16_t page)
{
    std::deque<CMDF_Register *>::iterator iter;
    for (iter = m_list_register.begin(); iter != m_list_register.end();
         ++iter) {

        CMDF_Register *preg = *iter;
        if ((reg == preg->m_nOffset) && (page == preg->m_nPage)) {
            return preg;
        }
    }

    return NULL;
}
