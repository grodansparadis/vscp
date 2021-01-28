// FILE: canal_config.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <expat.h>

#include "vscp.h"
#include "vscphelper.h"

#include "canal_xmlconfig.h"

#include <climits>
#include <cfloat>

// Buffer for XML parser
#define XML_BUFF_SIZE 100000

// ----------------------------------------------------------------------------
//                       CANAL XML configuration parser
// ----------------------------------------------------------------------------

static int depth_setup_parser = 0;
static char *last_content = NULL;   // Last read content
static bool bConfig = false;        // <config> read
static bool bItems = false;         // <items> read
static bool bSingleItem = false;    // <item> read
static bool bFlags = false;         // <flags> read

static bool bOptionalSet = false;   // Set after first optional item attribute read

static bool bContentDescription = false; // <description> content
static bool bContentLevel = false;       // <level> content
static bool bContentBlocking = false;    // <blocking> content
static bool bContentInfoUrl = false;     // <infoutl> content

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    canalXmlConfig *pConfig = (canalXmlConfig *)data;
    if ( NULL == pConfig ) return;

    if ( ( 0 == strcmp( name, "config") ) &&  ( 0 == depth_setup_parser ) ) {
        bConfig = true;
    }
    else if ( bConfig && ( 0 == strcmp( name, "description") ) &&  ( 1 == depth_setup_parser ) ) {        
        bContentDescription = true;   
    }
    else if ( bConfig && ( 0 == strcmp( name, "level") ) &&  ( 1 == depth_setup_parser ) ) {
        bContentLevel = true;
    }
    else if ( bConfig && ( 0 == strcmp( name, "blocking") ) &&  ( 1 == depth_setup_parser ) ) {
        bContentBlocking = true;
    }
    else if ( bConfig && ( 0 == strcmp( name, "infourl") ) &&  ( 1 == depth_setup_parser ) ) {
        bContentInfoUrl = true;
    }
    else if ( ( 0 == strcmp( name, "items") ) && ( 1 == depth_setup_parser ) ) {
        bItems = true;
    }
    else if ( ( 0 == strcmp( name, "flags") ) && ( 1 == depth_setup_parser ) ) {
        bFlags = true;
    }
    else if ( bItems && ( 0 == strcmp( name, "item") ) &&   ( 1 == depth_setup_parser ) ) {
        
        bSingleItem = true;

        bool bOptional = false;
        wizardStepBase::wizardType type = wizardStepBase::wizardType::NONE;
        std::string description;
        std::string infourl;

        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "type") ) {
                vscp_makeUpper(attribute);
                if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("none")) {
                        type = wizardStepBase::wizardType::NONE;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("STRING")) {
                        type = wizardStepBase::wizardType::STRING;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("BOOL")) {
                        type = wizardStepBase::wizardType::BOOL;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("INT32")) {
                        type = wizardStepBase::wizardType::INT32;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("UINT32")) {
                        type = wizardStepBase::wizardType::UINT32;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("INT64")) {
                        type = wizardStepBase::wizardType::INT64;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("UINT64")) {
                        type = wizardStepBase::wizardType::UINT64;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("FLOAT")) {
                        type = wizardStepBase::wizardType::STRING;
                    }
                }
                else if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("CHOICE")) {
                        type = wizardStepBase::wizardType::CHOICE;
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "optional") ) {
                vscp_makeLower(attribute);
                if ( !attribute.empty() && (std::string::npos != attribute.find("true")) ) {
                    bOptional = true;
                    bOptionalSet = true;    // All after this one will be optional
                }                
            }
            else if ( 0 == strcmp( attr[i], "description") ) {
                if ( !attribute.empty() ) {
                    description = attribute;
                }
            }
            else if ( 0 == strcmp( attr[i], "infourl") ) {
                if ( !attribute.empty() ) {
                    infourl = attribute;
                }
            }

        }

        if (bOptionalSet) {
            bOptional = true;
        }

        switch (type) {

            case wizardStepBase::wizardType::NONE:
                //wizardStepBase *item = new wizardStepBase();
                break;
            
            case wizardStepBase::wizardType::STRING:
                {
                    wizardStepString *item = new wizardStepString();
                }
                break;
            
            case wizardStepBase::wizardType::BOOL:
                {
                    wizardStepBool *item = new wizardStepBool();
                }
                break;
            
            case wizardStepBase::wizardType::INT32:
                {
                    wizardStepInt32 *item = new wizardStepInt32();
                }
                break;
            
            case wizardStepBase::wizardType::UINT32:
                {
                    wizardStepUInt32 *item = new wizardStepUInt32();
                }
                break;
            
            case wizardStepBase::wizardType::INT64:
                {
                    wizardStepInt64 *item = new wizardStepInt64();
                }
                break;
            
            case wizardStepBase::wizardType::UINT64:
                {
                    wizardStepUInt64 *item = new wizardStepUInt64();
                }
                break;
            
            case wizardStepBase::wizardType::FLOAT:
                {
                    wizardStepFloat *item = new wizardStepFloat();
                }
                break;

            case wizardStepBase::wizardType::CHOICE:
                {
                    wizardStepChoice *item = new wizardStepChoice();
                }
                break;    

            default:
                // we do nothing
                break;                               
        }

    }
    else if ( bFlags && ( 0 == strcmp( name, "flags") ) &&   ( 1 == depth_setup_parser ) ) {

    }

    depth_setup_parser++;

}

void
handle_data(void *data, const char *content, int length)
{
    canalXmlConfig *pConfig = (canalXmlConfig *)data;
    if ( NULL == pConfig ) return;

    std::string str(content,length);

    if ( bContentDescription ) {        
        std::string description(content,length);
        if ( !description.empty() ) pConfig->setDescription(description);
    }
    else if ( bContentLevel ) {
        std::string str(content,length);
        int level = vscp_readStringValue(str);
        if (VSCP_LEVEL1 == level) {
            pConfig->setLevel(VSCP_LEVEL1);
        } 
        else if (VSCP_LEVEL2 == level) {
            pConfig->setLevel(VSCP_LEVEL2);
        }
    }
    else if ( bContentBlocking ) {
        std::string blocking(content,length);
        vscp_makeLower(blocking);
        if (std::string::npos != blocking.find("no")) {
            pConfig->setBlocking(false);
        }
        else if (std::string::npos != blocking.find("false")) {
            pConfig->setBlocking(false);
        }
    }
    else if ( bContentInfoUrl) {
        std::string infourl(content,length); 
        if ( !infourl.empty() ) pConfig->setInfoUrl(infourl);
    }
}

void
endSetupParser( void *data, const char *name ) 
{
    depth_setup_parser--;

    if (1 == depth_setup_parser) {
        if (0 == strcmp( name, "items")) {
            bItems = false;
        }        
        else if (0 == strcmp( name, "flags")) {
            bFlags = false;
        }
        else if (0 == strcmp( name, "description")) {        
            bContentDescription = false;   
        }
        else if (0 == strcmp( name, "level")) {
            bContentLevel = false;
        }
        else if (0 == strcmp( name, "blocking")) {
            bContentBlocking = false;
        }
        else if (0 == strcmp( name, "infourl")) {
            bContentInfoUrl = false;
        }
    }
    else if (2 == depth_setup_parser) {
        if (0 == strcmp( name, "item")) {
            bSingleItem = false;
        }
    }
} 


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardChoiceItem::wizardChoiceItem()
{
    // Set defaults    
    m_description = "CANAL driver configuration item";        
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardChoiceItem::~wizardChoiceItem()
{

}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardChoiceFlagItemData::wizardChoiceFlagItemData()
{
    // Set defaults    
    m_description = "CANAL driver configuration item";        
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardChoiceFlagItemData::~wizardChoiceFlagItemData()
{

}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepBase::wizardStepBase()
{
    m_bOptional = false;
    m_description = "";
    m_infourl = "https://www.vscp.org";
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepBase::~wizardStepBase()
{
    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepString::wizardStepString() :
    wizardStepBase()
{
    m_value = "";
    m_type = wizardStepBase::wizardType::STRING;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepString::~wizardStepString()
{
    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepBool::wizardStepBool() :
    wizardStepBase()
{
    m_value = false;
    m_type = wizardStepBase::wizardType::BOOL;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepBool::~wizardStepBool()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepBool::setValueFormString(const std::string& strValue)
{
    std::string str = strValue;
    vscp_makeLower(str);      
    if (std::string::npos != str.find("true")) { 
        m_value = true;
    }
    else if (std::string::npos != str.find("yes")) { 
        m_value = true;
    }
    else if (std::string::npos != str.find("on")) { 
        m_value = true;
    }
    else if (std::string::npos != str.find("1")) { 
        m_value = true;        
    }
    else if (std::string::npos != str.find("false")) { 
        m_value = false;
    }
    else if (std::string::npos != str.find("no")) { 
        m_value = false;
    }
    else if (std::string::npos != str.find("off")) { 
        m_value = false;
    }
    else if (std::string::npos != str.find("0")) { 
        m_value = false;
    }
    else {
        m_value = false;
    }
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepBool::getValueAsString(void)
{
    if ( m_value) {
        return std::string("true");
    }
    else {
        return std::string("false");
    }
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepBool::isValueValid(const std::string& strValue)
{
    std::string str = strValue;
    vscp_makeLower(str);      
    if (std::string::npos != str.find("true")) { 
        return true;
    }
    else if (std::string::npos != str.find("yes")) { 
        return true;
    }
    else if (std::string::npos != str.find("on")) { 
        return true;
    }
    else if (std::string::npos != str.find("1")) { 
        return true;        
    }
    else if (std::string::npos != str.find("false")) { 
        return false;
    }
    else if (std::string::npos != str.find("no")) { 
        return false;
    }
    else if (std::string::npos != str.find("off")) { 
        return false;
    }
    else if (std::string::npos != str.find("0")) { 
        return false;
    }

    return false;
}

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepInt32::wizardStepInt32() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::INT32;
    m_value = 0;
    m_min = std::numeric_limits<int32_t>::min();
    m_max = std::numeric_limits<int32_t>::max();
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepInt32::~wizardStepInt32()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepInt32::setValueFormString(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepInt32::getValueAsString(void)
{
    std::string str;

    str = vscp_str_format("%d",(int)m_value);
    return str;
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepInt32::isValueValid(const std::string& strValue)
{
    int32_t value = vscp_readStringValue(strValue);
    if ((value < m_min) || (value > m_max)) {
        return false;
    }

    return true;    
}

// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepUInt32::wizardStepUInt32() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::UINT32;
    m_value = 0;
    m_min = std::numeric_limits<uint32_t>::min();
    m_max = std::numeric_limits<uint32_t>::max();
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepUInt32::~wizardStepUInt32()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepUInt32::setValueFormString(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepUInt32::getValueAsString(void)
{
    std::string str;

    str = vscp_str_format("%u",(int)m_value);
    return str;
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepUInt32::isValueValid(const std::string& strValue)
{
    uint32_t value = vscp_readStringValue(strValue);
    if ((value < m_min) || (value > m_max)) {
        return false;
    }

    return true;    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepInt64::wizardStepInt64() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::INT64;
    m_value = 0;
    m_min = std::numeric_limits<int64_t>::min();
    m_max = std::numeric_limits<int64_t>::max();
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepInt64::~wizardStepInt64()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepInt64::setValueFormString(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepInt64::getValueAsString(void)
{
    std::string str;

    str = vscp_str_format("%lld",m_value);
    return str;
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepInt64::isValueValid(const std::string& strValue)
{
    int64_t value = vscp_readStringValue(strValue);
    if ((value < m_min) || (value > m_max)) {
        return false;
    }

    return true;    
}


// ----------------------------------------------------------------------------



//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepUInt64::wizardStepUInt64() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::UINT64;
    m_value = 0;
    m_min = std::numeric_limits<uint64_t>::min();
    m_max = std::numeric_limits<uint64_t>::max();
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepUInt64::~wizardStepUInt64()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepUInt64::setValueFormString(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepUInt64::getValueAsString(void)
{
    std::string str;

    str = vscp_str_format("%llu",m_value);
    return str;
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepUInt64::isValueValid(const std::string& strValue)
{
    uint64_t value = vscp_readStringValue(strValue);
    if ((value < m_min) || (value > m_max)) {
        return false;
    }

    return true;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepFloat::wizardStepFloat() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::FLOAT;
    m_value = 0;
    m_min = std::numeric_limits<double>::min();
    m_max = std::numeric_limits<double>::max();
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepFloat::~wizardStepFloat()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValueFormString
//

void wizardStepFloat::setValueFormString(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValueAsString
//

std::string wizardStepFloat::getValueAsString(void)
{
    std::string str;

    str = vscp_str_format("%f",m_value);
    return str;
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardStepFloat::isValueValid(const std::string& strValue)
{
    double value = atof(strValue.c_str());
    if ((value < m_min) || (value > m_max)) {
        return false;
    }

    return true;    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardStepChoice::wizardStepChoice() :
    wizardStepBase()
{
    m_type = wizardStepBase::wizardType::CHOICE;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepChoice::~wizardStepChoice()
{
    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

canalXmlConfig::canalXmlConfig()
{
    // Set defaults
    m_description = "CANAL driver";
    m_level = VSCP_LEVEL1;
    m_bBlocking = true;
    m_infourl = "https://www.vscp.org";
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

canalXmlConfig::~canalXmlConfig()
{

}

//////////////////////////////////////////////////////////////////////////////
// addWizardStep
//

bool canalXmlConfig::addWizardStep(const wizardStepBase *step)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// parseXML
//

bool canalXmlConfig::canalXmlConfig::parseXML(const std::string& xmlcfg)
{
    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData( xmlParser, this );
    XML_SetElementHandler( xmlParser,
                            startSetupParser,
                            endSetupParser ) ;
    XML_SetCharacterDataHandler(xmlParser, handle_data);                            

    int bytes_read;
    void *buff = XML_GetBuffer( xmlParser, XML_BUFF_SIZE );

    strncpy( (char *)buff, xmlcfg.c_str(), xmlcfg.length() );

    bytes_read = xmlcfg.length();
    if ( !XML_ParseBuffer( xmlParser, bytes_read, bytes_read == 0 ) ) {
        return false;
    }

    XML_ParserFree( xmlParser );
    return true;
}
