// FILE: canal_config.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2024 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#ifdef WIN32
#include <pch.h>
#include <limits>
#endif

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <expat.h>

#include "vscp.h"
#include "vscphelper.h"

#include "canal_xmlconfig.h"

#include <limits>
#include <cfloat>

// Uncomment to get some debug info
//#define DEBUG_CANAL_XMLCONFIG

// Buffer for XML parser
#define XML_BUFF_SIZE 100000

// ----------------------------------------------------------------------------
//                       CANAL XML configuration parser
// ----------------------------------------------------------------------------

static int depth_setup_parser = 0;
static char *last_content = NULL;   // Last read content
static bool bConfig = false;        // <config> read
static bool bItems = false;         // <items> read
static bool bItem = false;          // <item> read
static bool bFlags = false;         // <flags> read
static bool bBit = false;           // <bit> read

static bool bOptionalSet = false;   // Set after first optional item attribute read

static bool bContentDescription = false; // <description> content
static bool bContentLevel = false;       // <level> content
static bool bContentBlocking = false;    // <blocking> content
static bool bContentInfoUrl = false;     // <infoutl> content

// When a choice item is found and allocated a pointer is is set to
// it here. This pointer is set to NULL in the end item tag
static wizardStepChoice *lastItemChoice = NULL;   

// When a choice flag bit is found and allocated a pointer is is set to
// it here. This pointer is set to NULL in the end bit tag
static wizardFlagBitChoice *lastBitChoice = NULL;

static void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    canalXmlConfig *pConfig = (canalXmlConfig *)data;
    if ( NULL == pConfig ) return;

#ifdef DEBUG_CANAL_XMLCONFIG
    fprintf(stderr, "START: %s - %d\n", name, depth_setup_parser);
#endif    

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
    else if ( bItems && ( 0 == strcmp( name, "item") ) && ( 2 == depth_setup_parser ) ) {
        
        bItem = true;

        bool bOptional = false;
        wizardStepBase::wizardType type = wizardStepBase::wizardType::NONE;
        std::string description;
        std::string infourl;
        std::string initialval;

        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "type") ) {
                vscp_makeUpper(attribute);
                if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("none")) {
                        type = wizardStepBase::wizardType::NONE;
                    }
                    else if (std::string::npos != attribute.find("STRING")) {
                        type = wizardStepBase::wizardType::STRING;
                    }
                    else if (std::string::npos != attribute.find("BOOL")) {
                        type = wizardStepBase::wizardType::BOOL;
                    }
                    else if (std::string::npos != attribute.find("UINT32")) {
                        type = wizardStepBase::wizardType::UINT32;
                    }
                    else if (std::string::npos != attribute.find("INT32")) {
                        type = wizardStepBase::wizardType::INT32;
                    }
                    else if (std::string::npos != attribute.find("UINT64")) {
                        type = wizardStepBase::wizardType::UINT64;
                    }
                    else if (std::string::npos != attribute.find("INT64")) {
                        type = wizardStepBase::wizardType::INT64;
                    }                    
                    else if (std::string::npos != attribute.find("FLOAT")) {
                        type = wizardStepBase::wizardType::FLOAT;
                    }
                    else if (std::string::npos != attribute.find("CHOICE")) {
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
            else if ( 0 == strcmp( attr[i], "default") ) {
                if ( !attribute.empty() ) {
                    initialval = attribute;
                }
            }

        } // for

        if (bOptionalSet) {
            bOptional = true;
        }

        wizardStepBase *item = NULL;
        switch (type) {

            case wizardStepBase::wizardType::NONE:
                item = NULL; 
                break;
            
            case wizardStepBase::wizardType::STRING:
                item = new wizardStepString();
                break;
            
            case wizardStepBase::wizardType::BOOL:
                item = new wizardStepBool();
                break;
            
            case wizardStepBase::wizardType::INT32:
                item = new wizardStepInt32();
                break;
            
            case wizardStepBase::wizardType::UINT32:
                item = new wizardStepUInt32();
                break;
            
            case wizardStepBase::wizardType::INT64:
                item = new wizardStepInt64();
                break;
            
            case wizardStepBase::wizardType::UINT64:
                item = new wizardStepUInt64();
                break;
            
            case wizardStepBase::wizardType::FLOAT:
                item = new wizardStepFloat();
                break;

            case wizardStepBase::wizardType::CHOICE:
                item = lastItemChoice = new wizardStepChoice();
                break;    

            default:
                // we do nothing
                break;                               
        }

        // Add the item
        if (NULL != item) {
            item->setOptional(bOptional);
            item->setInfoUrl(infourl);
            item->setDescription(description);
            item->setValue(initialval);
            pConfig->addWizardStep(item);            
        }

    }
    else if ( bFlags && ( 0 == strcmp( name, "bit") ) &&   ( 2 == depth_setup_parser ) ) {

        bBit = true;

        uint8_t pos = 0;
        uint8_t width = 1;
        wizardStepBase::wizardType type = wizardStepBase::wizardType::NONE;
        std::string initialval;
        std::string description;
        std::string infourl;

        for ( int i = 0; attr[i]; i += 2 ) {
            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "pos") ) {
                pos = vscp_readStringValue(attribute);
            }
            else if ( 0 == strcmp( attr[i], "width") ) {
                width = vscp_readStringValue(attribute);
            }
            else if ( 0 == strcmp( attr[i], "default") ) {
                initialval = attribute;
            }
            else if ( 0 == strcmp( attr[i], "description") ) {
                description = attribute;
            }
            else if ( 0 == strcmp( attr[i], "infourl") ) {
                infourl = attribute;
            }
            else if ( 0 == strcmp( attr[i], "type") ) {
                vscp_makeUpper(attribute);
                if ( !attribute.empty() ) {
                    if (std::string::npos != attribute.find("none")) {
                        type = wizardStepBase::wizardType::NONE;
                    }
                    else if (std::string::npos != attribute.find("BOOL")) {
                        type = wizardStepBase::wizardType::BOOL;
                    }
                    else if (std::string::npos != attribute.find("NUMBER")) {
                        type = wizardStepBase::wizardType::UINT32;
                    }
                    else if (std::string::npos != attribute.find("CHOICE")) {
                        type = wizardStepBase::wizardType::CHOICE;
                    }
                }
            }
        } // for

        wizardFlagBitBase *item = NULL;
        switch (type) {

            case wizardStepBase::wizardType::NONE:
                item = NULL; 
                break;
            
            case wizardStepBase::wizardType::BOOL:
                item = new wizardFlagBitBool();
                break;
            
            case wizardStepBase::wizardType::UINT32:
                item = new wizardFlagBitNumber();
                break;

            case wizardStepBase::wizardType::CHOICE:
                item = lastBitChoice = new wizardFlagBitChoice();                
                break;    

            default:
                // we do nothing
                break;                               
        }

        // Add the item
        if (NULL != item) {
            item->setPos(pos);
            item->setWidth(width);
            item->setInfoUrl(infourl);
            item->setDescription(description);
            pConfig->addFlagWizardStep(item);
        }

    }
    else if ( bItem && ( 0 == strcmp( name, "choice") ) && ( 3 == depth_setup_parser ) ) {
        
        std::string value;
        std::string description;        

        for ( int i = 0; attr[i]; i += 2 ) {
            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "value") ) {
                value = attribute;
            }
            else if ( 0 == strcmp( attr[i], "description") ) {
                description = attribute;
            }
        }

        wizardChoiceItem *choice = new wizardChoiceItem();
        choice->setValue(value);
        choice->setDescription(description);
        if (NULL != lastItemChoice) {
            lastItemChoice->addChoice(choice);
        }

    }
    else if ( bBit && ( 0 == strcmp( name, "choice") ) && ( 3 == depth_setup_parser ) ) {

        std::string value;
        std::string description;        

        for ( int i = 0; attr[i]; i += 2 ) {
            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "value") ) {
                value = attribute;
            }
            else if ( 0 == strcmp( attr[i], "description") ) {
                description = attribute;
            }
        }

        wizardBitChoice *choice = new wizardBitChoice();
        choice->setValue(value);
        choice->setDescription(description);
        if (NULL != lastBitChoice) {
            lastBitChoice->addChoice(choice);
        }
    }

    depth_setup_parser++;

}

static void
handle_data(void *data, const char *content, int length)
{
    canalXmlConfig *pConfig = (canalXmlConfig *)data;
    if ( NULL == pConfig ) return;

    std::string str(content,length);

#ifdef DEBUG_CANAL_XMLCONFIG
    fprintf(stderr, "DATA: %s - %d\n", (const char *)data, depth_setup_parser);
#endif    

    if ( bContentDescription ) {         
        std::string description(content, length);
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

static void
endSetupParser( void *data, const char *name ) 
{
    depth_setup_parser--;

#ifdef DEBUG_CANAL_XMLCONFIG
    fprintf(stderr, "END: %s - %d\n", name, depth_setup_parser);
#endif    

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
            bItem = false;
            lastItemChoice = NULL;   
        }
    }
    else if (2 == depth_setup_parser) {
        if (0 == strcmp( name, "bit")) {
            bBit = false;
            lastBitChoice = NULL;
        }
    }
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
// setValue
//

void wizardStepBool::setValue(const std::string& strValue)
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
// getValue
//

std::string wizardStepBool::getValue(void)
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
#ifdef WIN32
  // TODO: implement this for Windows
#else
    m_min = std::numeric_limits<int32_t>::min();
    m_max = std::numeric_limits<int32_t>::max();
#endif    
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepInt32::~wizardStepInt32()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardStepInt32::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardStepInt32::getValue(void)
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
#ifdef WIN32
  // TODO: implement this for Windows
#else    
    m_min = std::numeric_limits<uint32_t>::min();
    m_max = std::numeric_limits<uint32_t>::max();
#endif    
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepUInt32::~wizardStepUInt32()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardStepUInt32::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardStepUInt32::getValue(void)
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
#ifdef WIN32
  // TODO: implement this for Windows
#else
    m_min = std::numeric_limits<int64_t>::min();
    m_max = std::numeric_limits<int64_t>::max();
#endif
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepInt64::~wizardStepInt64()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardStepInt64::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardStepInt64::getValue(void)
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
#ifdef WIN32
  // TODO: implement this for Windows
#else    
    m_min = std::numeric_limits<uint64_t>::min();
    m_max = std::numeric_limits<uint64_t>::max();
#endif
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepUInt64::~wizardStepUInt64()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardStepUInt64::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardStepUInt64::getValue(void)
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
#ifdef WIN32
  // TODO: implement this for Windows
#else    
    m_min = std::numeric_limits<double>::min();
    m_max = std::numeric_limits<double>::max();
#endif
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardStepFloat::~wizardStepFloat()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardStepFloat::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);    
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardStepFloat::getValue(void)
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

wizardChoiceItem::wizardChoiceItem()
{
    // Set defaults    
    m_description = "CANAL driver configuration item";    
    m_value = "Default";    
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

//////////////////////////////////////////////////////////////////////////////
// getRealValue
//

std::string wizardStepChoice::getRealValue(const std::string& input)
{
    std::string rv;

    uint16_t idx = vscp_readStringValue(input);
    if ( idx >= m_choices.size() ) {

    }
    else {
        wizardChoiceItem *choice = m_choices[idx];
        rv = choice->getValue();
    }

    return rv;
}




// ----------------------------------------------------------------------------
//                                 Flags
// ----------------------------------------------------------------------------





//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardFlagBitBase::wizardFlagBitBase()
{
    m_type = wizardStepBase::wizardType::STRING;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardFlagBitBase::~wizardFlagBitBase()
{
    
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardFlagBitBool::wizardFlagBitBool() :
    wizardFlagBitBase()
{
    m_type = wizardStepBase::wizardType::BOOL;
    m_value = false;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardFlagBitBool::~wizardFlagBitBool()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardFlagBitBool::setValue(const std::string& strValue)
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
// getValue
//

std::string wizardFlagBitBool::getValue(void)
{
    return m_value ? "true" : "false";
}

//////////////////////////////////////////////////////////////////////////////
// isValueValid
//

bool wizardFlagBitBool::isValueValid(const std::string& strValue)
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
        return true;
    }
    else if (std::string::npos != str.find("no")) { 
       return true;
    }
    else if (std::string::npos != str.find("off")) { 
        return true;
    }
    else if (std::string::npos != str.find("0")) { 
        return true;
    }
    else {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////////
// getRealValue
//

uint32_t wizardFlagBitBool::getRealValue(const std::string& input)
{
    uint32_t rv = 0;
    
    if (std::string::npos != input.find("true")) {        
        rv = 1 << getPos();
    }
    return rv;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardFlagBitNumber::wizardFlagBitNumber() :
    wizardFlagBitBase()
{
    m_type = wizardStepBase::wizardType::UINT32;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardFlagBitNumber::~wizardFlagBitNumber()
{
    
}


//////////////////////////////////////////////////////////////////////////////
// setValue
//

void wizardFlagBitNumber::setValue(const std::string& strValue)
{
    m_value = vscp_readStringValue(strValue);
}

//////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string wizardFlagBitNumber::getValue(void)
{
    return vscp_str_format("%lu", m_value);
}

/////////////////////////////////////////////////////////////////////////////
// getRealValue
//

uint32_t wizardFlagBitNumber::getRealValue(const std::string& input)
{
    uint32_t value = vscp_readStringValue(input);
    value &= (uint32_t)pow(2, m_width) - 1;    // Mask
    value = value << m_pos;                    // Shift to pos
    return value;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardBitChoice::wizardBitChoice()
{
    // Set defaults    
    m_description = "CANAL driver configuration item";
    m_value = "0";        
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardBitChoice::~wizardBitChoice()
{

}


// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// CTor
//

wizardFlagBitChoice::wizardFlagBitChoice() :
    wizardFlagBitBase()
{
    m_type = wizardStepBase::wizardType::CHOICE;
}

//////////////////////////////////////////////////////////////////////////////
// DTor
//

wizardFlagBitChoice::~wizardFlagBitChoice()
{
    
}

//////////////////////////////////////////////////////////////////////////////
// getFlagsValue
//
// value & (2^width - 1) gives valied values for a field
//  value << pos move the value to the right pos.

uint32_t wizardFlagBitChoice::getFlagsValue(void)
{
    uint32_t value = vscp_readStringValue(m_value);
    value &= (uint32_t)pow(2,(m_width - 1));    // Mask
    value = value << m_pos;                     // Shift to pos
    return value;
}

//////////////////////////////////////////////////////////////////////////////
// getRealValue
//

uint32_t wizardFlagBitChoice::getRealValue(const std::string& input)
{
    uint32_t rv;

    uint16_t idx = vscp_readStringValue(input);
    if ( idx >= m_choices.size() ) {

    }
    else {
        wizardBitChoice *choice = m_choices[idx];
        rv = vscp_readStringValue(choice->getValue());
    }

    return rv;
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

bool canalXmlConfig::addWizardStep(wizardStepBase *item)
{
    m_stepsWizardItems.push_back(item);
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// addFlagWizardStep
//

bool canalXmlConfig::addFlagWizardStep(wizardFlagBitBase *item)
{
    m_stepsWizardFlags.push_back(item);
    return true;
}

#if defined(DEBUG_CANAL_XMLCONFIG)
#define XML_CONFIG  "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>"                             \
                    "<!-- Version 1.0   2021-01-26   -->"                                         \
                    "<config>"                                                                    \
                    "<description format=\"text\">VSCP level I (CANAL) SocketCan driver</description>"\
                    "<level>1</level>"                                                            \
                    "<blocking>yes</blocking>"                                                    \
                    "<infourl>https://github.com/grodansparadis/vscpl1drv-socketcan#vscpl1drv-socketcan---vscp-level-i-socketcan-driver</infourl>"    \
                    "<items>"                                                                     \
                    "<item type=\"string\" optional=\"false\" default=\"vcan0\" "                 \
                    "  description=\"socketcan interface (vcan, can0, can1...)\" "                \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"string\" optional=\"true\" default=\"1234\" "                                    \
                    "  description=\"mask (priority,class,type,guid)\" "                          \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"string\" optional=\"true\" default=\"5678\""                                    \
                    "  description=\"filter (priority,class,type,guid)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"string\" optional=\"true\" default=\"ABCD\" "                                    \
                    "  description=\"yyyyyyyy (Special)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"choice\" optional=\"true\" "                                    \
                    "  description=\"A list fi coices\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" >" \
                    "<choice value=\"1\" description=\"First value\"  /> "                        \
                    "<choice value=\"2\" description=\"Second value\" /> "                        \
                    "<choice value=\"3\" description=\"Third value\"  /> "                        \
                    "<choice value=\"4\" description=\"Fourth value\" /> "                        \
                    "</item>"                                                                     \
                    "<item type=\"bool\" optional=\"true\" default=\"true\" "                                    \
                    "  description=\"A boolean value (default=true)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"bool\" optional=\"true\" default=\"false\" "                                    \
                    "  description=\"A boolean value (default=true)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"int32\" optional=\"true\" default=\"-34\" "                                    \
                    "  description=\"A int32 value (default=-34)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"uint32\" optional=\"true\" default=\"987\" "                                    \
                    "  description=\"A uint32 value (default=-987)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"int64\" optional=\"true\" default=\"-34\" "                                    \
                    "  description=\"A int64 value (default=-34)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"uint64\" optional=\"true\" default=\"987\" "                                    \
                    "  description=\"A uint64 value (default=-987)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "<item type=\"float\" optional=\"true\" default=\"123.45\" "                                    \
                    "  description=\"A float value (default=123.5)\" "                        \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#configuration-string\" />" \
                    "</items>"                                                                    \
                    "<flags>"                                                                     \
                    "<bit pos=\"0\" width=\"1\" type=\"bool\" "                                   \
                    "  description=\"Test1\" "                                                    \
                    "  infourl=\"https://test.com\" /> "                                          \
                    "<bit pos=\"1\" width=\"4\" type=\"number\" "                                 \
                    "  description=\"Test2\" "                                                    \
                    "  infourl=\"https://test.com\" /> "                                          \
                    "<bit pos=\"5\" width=\"2\" type=\"choice\" "                                 \
                    "  description=\"Test3\" "                                                    \
                    "  infourl=\"https://test.com\" > "                                           \
                    "<bit pos=\"31\" width=\"1\" type=\"bool\" "                                  \
                    "  description=\"Enable debug\" "                                             \
                    "  infourl=\"https://github.com/grodansparadis/vscpl1drv-socketcan#flags\" /> "\
                    "<choice value=\"0\" description=\"First field value\"  /> "                  \
                    "<choice value=\"1\" description=\"Second field value\" /> "                  \
                    "<choice value=\"2\" description=\"Third field value\"  /> "                  \
                    "<choice value=\"3\" description=\"Fourth field value\" /> "                  \
                    "</bit>"                                                                      \
                    "</flags>"                                                                    \
                    "</config>"                                                                   
#endif

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

#ifdef DEBUG_CANAL_XMLCONFIG
    strncpy( (char *)buff, XML_CONFIG, strlen(XML_CONFIG) );
    bytes_read = strlen(XML_CONFIG);
#else
    strncpy( (char *)buff, xmlcfg.c_str(), xmlcfg.length() );
    bytes_read = (int)xmlcfg.length();
#endif    

    
    if ( !XML_ParseBuffer( xmlParser, bytes_read, bytes_read == 0 ) ) {
        return false;
    }

    XML_ParserFree( xmlParser );

// ----------------------------------------------------------------------------
//                             Debug output start
// ----------------------------------------------------------------------------
#ifdef DEBUG_CANAL_XMLCONFIG
    fprintf(stderr, "Description: %s\n", m_description.c_str());
    fprintf(stderr, "Info URL: %s\n", m_infourl.c_str());
    fprintf(stderr, "Level: %d\n", m_level);
    fprintf(stderr, "The driver is %s\n\n", m_bBlocking ? "Blocking" : "non Blocking");
    fprintf(stderr, "Item steps count %zu\n", m_stepsWizardItems.size() );
    fprintf(stderr, "Flag steps count %zu\n", m_stepsWizardFlags.size() );

    fprintf(stderr, "\nWizard Steps\n");
    fprintf(stderr, "------------\n");
    {
        std::list<wizardStepBase *>::iterator it;
        for (it = m_stepsWizardItems.begin(); it != m_stepsWizardItems.end(); ++it){
            
            fprintf(stderr,"\tType = %d\n", static_cast<int>((*it)->m_type));
            
            switch ((*it)->m_type) {

                case wizardStepBase::wizardType::NONE:
                    fprintf(stderr, "\twizardType::NONE\n"); 
                    break;
                
                case wizardStepBase::wizardType::STRING:
                    {
                        fprintf(stderr, "\twizardType::STRING\n"); 
                        wizardStepString *item = (wizardStepString *)(*it);
                        fprintf(stderr, "\tValue = %s\n", item->getValue().c_str());
                    }
                    break;
                
                case wizardStepBase::wizardType::BOOL:
                    {
                        fprintf(stderr, "\twizardType::BOOL\n"); 
                        wizardStepBool *item = (wizardStepBool *)(*it);
                    }
                    break;
                
                case wizardStepBase::wizardType::INT32:
                    {
                        fprintf(stderr, "\twizardType::INT32\n"); 
                        wizardStepInt32 *item = (wizardStepInt32 *)(*it);
                    }
                    break;
                
                case wizardStepBase::wizardType::UINT32:
                    {
                        fprintf(stderr, "\twizardType::UINT32\n"); 
                        wizardStepUInt32 *item = (wizardStepUInt32 *)(*it);
                    }
                    break;
                
                case wizardStepBase::wizardType::INT64:
                    {
                        fprintf(stderr, "\twizardType::INT64\n");
                        wizardStepInt64 *item = (wizardStepInt64 *)(*it); 
                    }
                    break;
                
                case wizardStepBase::wizardType::UINT64:
                    {
                        fprintf(stderr, "\twizardType::UINT64\n");
                        wizardStepUInt64 *item = (wizardStepUInt64 *)(*it); 
                    }
                    break;
                
                case wizardStepBase::wizardType::FLOAT:
                    {
                        fprintf(stderr, "\twizardType::FLOAT\n");
                        wizardStepFloat *item = (wizardStepFloat *)(*it); 
                    }
                    break;

                case wizardStepBase::wizardType::CHOICE:
                    {
                        fprintf(stderr, "\twizardType::CHOICE\n");
                        wizardStepChoice *item = (wizardStepChoice *)(*it);

                        std::vector<wizardChoiceItem *>::iterator it;
                        for (it = item->m_choices.begin(); it != item->m_choices.end(); ++it){
                            wizardChoiceItem *itemChoice = (wizardChoiceItem *)(*it);
                            fprintf(stderr, 
                                        "\t\tChoice Value=%s Description=%s\n",
                                        itemChoice->getValue().c_str(), 
                                        itemChoice->getDescription().c_str());
                        }
                    }
                    break;    

                default:
                    // we do nothing
                    break;                               
            }
        }
    }

    fprintf(stderr, "\nFlag Steps\n");
    fprintf(stderr, "----------\n");
    {
        std::list<wizardFlagBitBase *>::iterator it;
        for (it = m_stepsWizardFlags.begin(); it != m_stepsWizardFlags.end(); ++it){

            fprintf(stderr,"\tType = %d\n", static_cast<int>((*it)->m_type));

            switch ((*it)->m_type) {

                case wizardStepBase::wizardType::NONE:
                    fprintf(stderr, "\twizardType::NONE\n"); 
                    break;
                
                case wizardStepBase::wizardType::BOOL:
                    {
                        fprintf(stderr, "\twizardType::BOOL\n"); 
                        wizardFlagBitBool *item = (wizardFlagBitBool *)(*it);
                    }
                    break;
                
                case wizardStepBase::wizardType::UINT32:
                    {
                        fprintf(stderr, "\twizardType::UINT32\n"); 
                        wizardFlagBitNumber *item = (wizardFlagBitNumber *)(*it);
                    }
                    break;

                case wizardStepBase::wizardType::CHOICE:
                    {
                        fprintf(stderr, "\twizardType::CHOICE\n");
                        wizardFlagBitChoice *item = (wizardFlagBitChoice *)(*it);
   
                        std::vector<wizardBitChoice *>::iterator it;
                        for (it = item->m_choices.begin(); it != item->m_choices.end(); ++it){
                            wizardBitChoice *itemChoice = (wizardBitChoice *)(*it);
                            fprintf(stderr, 
                                        "\t\tChoice Value=%s Description=%s\n",
                                        itemChoice->getValue().c_str(), 
                                        itemChoice->getDescription().c_str());
                        }
                    }
                    break;    

                default:
                    // we do nothing
                    break;                               
            }
        }
    }
#endif

// ----------------------------------------------------------------------------
//                             Debug output end
// ----------------------------------------------------------------------------

    return true;
}
