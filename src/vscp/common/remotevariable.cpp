// vscpvariable.cpp
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

#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>

#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <duktape.h>
#include <lua.h>

#include <expat.h>
#include <json.hpp> // Needs C++11  -std=c++11

#include <controlobject.h>
#include <guid.h>
#include <variablecodes.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpbase64.h>
#include <vscpdatetime.h>
#include <vscpdb.h>
#include <vscphelper.h>

#include "remotevariable.h"

#define XML_BUFF_SIZE 512000 // Size of XML parser buffer

// https://github.com/nlohmann/json
using json = nlohmann::json;

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

// The global control object
extern CControlObject *gpobj;

// XML parser
static void
startLoadVarParser(void *data, const char *name, const char **attr);
static void
endLoadVarParser(void *data, const char *name);

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVSCPVariable::CVSCPVariable(void)
{
    init();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPVariable::~CVSCPVariable(void)
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
CVSCPVariable::init(void)
{
    m_id = 0;
    m_name.clear();
    m_type         = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    m_bPersistent  = false;               // Not persistent by default
    m_accessRights = PERMISSON_OWNER_ALL; // Owner can do everything.
    m_userid       = USER_ID_ADMIN;       // Admin owns variable by default
    m_lastChanged  = vscpdatetime::Now();
    m_bStock       = false; // This is not a stock variable
}

///////////////////////////////////////////////////////////////////////////////
// fixName
//

void
CVSCPVariable::fixName(void)
{
    vscp_trim(m_name);
    vscp_makeUpper(m_name);

    // Works only for ASCII names. Should be fixed so
    // UTF8 names can be used TODO
    for (int i = 0; i < m_name.length(); i++) {
        switch ((const char)m_name[i]) {
            case ';':
            case '\'':
            case '\"':
            case ',':
            case ' ':
                m_name[i] = '_';
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// makeAccessTightString
//

void
CVSCPVariable::makeAccessRightString(uint32_t accessrights,
                                     std::string &strAccessRights)
{
    strAccessRights.clear();

    // other
    strAccessRights = (accessrights & PERMISSON_OTHER_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_OTHER_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_OTHER_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;

    // group
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = (accessrights & PERMISSON_GROUP_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_GROUP_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_GROUP_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;

    // owner
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = (accessrights & PERMISSON_OWNER_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_OWNER_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSON_OWNER_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableType
//

uint16_t
CVSCPVariable::getVariableTypeFromString(const std::string &strVariableType)
{
    uint16_t type   = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    std::string str = strVariableType;
    long val;

    vscp_trim(str);

    // Read as numeric if it looks like it is numeric
    if (vscp_isNumber(str)) {
        type = vscp_readStringValue(str);
    } else {
        if (0 == vscp_strcasecmp(str.c_str(), "string")) {
            type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        } else if (0 == vscp_strcasecmp(str.c_str(), "bool")) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        } else if (0 == vscp_strcasecmp(str.c_str(), "boolean")) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        } else if (0 == vscp_strcasecmp(str.c_str(), "int")) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        } else if (0 == vscp_strcasecmp(str.c_str(), "integer")) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        } else if (0 == vscp_strcasecmp(str.c_str(), "long")) {
            type = VSCP_DAEMON_VARIABLE_CODE_LONG;
        } else if (0 == vscp_strcasecmp(str.c_str(), "float")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "double")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "measurement")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventguid")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventdata")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventclass")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventtype")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventtimestamp")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        } else if (0 == vscp_strcasecmp(str.c_str(), "event")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "guid")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        } else if (0 == vscp_strcasecmp(str.c_str(), "data")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        } else if (0 == vscp_strcasecmp(str.c_str(), "vscpclass")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        } else if (0 == vscp_strcasecmp(str.c_str(), "vscptype")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "timestamp")) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        } else if (0 == vscp_strcasecmp(str.c_str(), "datetime")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DATETIME;
        } else if (0 == vscp_strcasecmp(str.c_str(), "date")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DATE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "time")) {
            type = VSCP_DAEMON_VARIABLE_CODE_TIME;
        } else if (0 == vscp_strcasecmp(str.c_str(), "blob")) {
            type = VSCP_DAEMON_VARIABLE_CODE_BLOB;
        } else if (0 == vscp_strcasecmp(str.c_str(), "mime")) {
            type = VSCP_DAEMON_VARIABLE_CODE_MIME;
        } else if (0 == vscp_strcasecmp(str.c_str(), "html")) {
            type = VSCP_DAEMON_VARIABLE_CODE_HTML;
        } else if (0 == vscp_strcasecmp(str.c_str(), "javascript")) {
            type = VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "json")) {
            type = VSCP_DAEMON_VARIABLE_CODE_JSON;
        } else if (0 == vscp_strcasecmp(str.c_str(), "xml")) {
            type = VSCP_DAEMON_VARIABLE_CODE_XML;
        } else if (0 == vscp_strcasecmp(str.c_str(), "sql")) {
            type = VSCP_DAEMON_VARIABLE_CODE_SQL;
        } else if (0 == vscp_strcasecmp(str.c_str(), "lua")) {
            type = VSCP_DAEMON_VARIABLE_CODE_LUA;
        } else if (0 == vscp_strcasecmp(str.c_str(), "luares")) {
            type = VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "ux1")) {
            type = VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1;
        } else if (0 == vscp_strcasecmp(str.c_str(), "dmrow")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DM_ROW;
        } else if (0 == vscp_strcasecmp(str.c_str(), "driver")) {
            type = VSCP_DAEMON_VARIABLE_CODE_DRIVER;
        } else if (0 == vscp_strcasecmp(str.c_str(), "user")) {
            type = VSCP_DAEMON_VARIABLE_CODE_USER;
        } else if (0 == vscp_strcasecmp(str.c_str(), "filter")) {
            type = VSCP_DAEMON_VARIABLE_CODE_FILTER;
        } else {
            type = vscp_readStringValue(str);
        }
    }

    return type;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableTypeAsString
//

const char *
CVSCPVariable::getVariableTypeAsString(int type)
{
    switch (type) {

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            return "Unassigned";

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            return "String";

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            return "Boolean";

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            return "Integer";

        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            return "Long";

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            return "Double";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            return "Measurement";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            return "VscpEvent";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            return "VscpGuid";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            return "VscpData";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            return "VscpClass";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            return "VscpType";

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            return "Timestamp";

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            return "DateTime";

        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
            return "BLOB";

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            return "Date";

        case VSCP_DAEMON_VARIABLE_CODE_TIME:
            return "Time";

        case VSCP_DAEMON_VARIABLE_CODE_MIME:
            return "Mime";

        case VSCP_DAEMON_VARIABLE_CODE_HTML:
            return "Html";

        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
            return "Javascript";

        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            return "Json";

        case VSCP_DAEMON_VARIABLE_CODE_XML:
            return "XML";

        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            return "SQL";

        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            return "LUA";

        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            return "LUARES";

        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            return "UX1";

        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
            return "DMrow";

        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
            return "Driver";

        case VSCP_DAEMON_VARIABLE_CODE_USER:
            return "User";

        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
            return "Filter";

        default:
            return "Unknown";
    }
}

///////////////////////////////////////////////////////////////////////////////
// isNumerical
//

bool
CVSCPVariable::isNumerical(void)
{
    if ((VSCP_DAEMON_VARIABLE_CODE_INTEGER == m_type) ||
        (VSCP_DAEMON_VARIABLE_CODE_LONG == m_type) ||
        (VSCP_DAEMON_VARIABLE_CODE_DOUBLE == m_type)) {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setType
//

bool
CVSCPVariable::setType(const std::string &strType)
{
    unsigned long lval;

    std::string str = strType;
    vscp_trim(str);

    if (vscp_isNumber(str)) {

        lval = vscp_readStringValue(strType);
        if (lval > 0xffff) return false;
    } else {

        if (VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED ==
            (lval = getVariableTypeFromString(strType))) {
            return false;
        }
    }

    // Set as numeric
    m_type = (uint16_t)lval;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//

std::string
CVSCPVariable::getAsString(bool bShort)
{
    std::string str;

    str = m_name;
    str += (";");
    str += vscp_str_format("%u", (unsigned short)m_type);
    str += (";");
    str += vscp_str_format("%lu", (unsigned long)m_userid);
    str += (";");
    str += vscp_str_format("%lu", (unsigned long)m_accessRights);
    str += (";");
    str += m_bPersistent ? ("true") : ("false");
    str += (";");
    str += m_lastChanged.getISODateTime();

    // Long format can be dangerous as it can give **VERY** long lines.
    if (!bShort) {
        str += ";";
        std::string str = m_strValue;
        vscp_base64_std_encode(str);
        str += str;
        str += ";";
        str = m_note;
        vscp_base64_std_encode(str);
        str += str;
    }

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// getAsJSON
//

bool
CVSCPVariable::getAsJSON(std::string &strVariable)
{
    std::string str = m_strValue;
    vscp_base64_std_encode(str);

    // name,type,user,rights,persistence,last,bnumerical,bbase64,value,note
    // value is numerical for a numerical variable else string
    vscp_str_format(strVariable,
                       VARIABLE_JSON_TEMPLATE,
                       m_name,
                       (unsigned short int)m_type,
                       (unsigned long int)m_userid,
                       (unsigned long int)m_accessRights,
                       m_bPersistent ? "true" : "false",
                       (const char *)m_lastChanged.getISODateTime().c_str(),
                       isNumerical() ? "true" : "false",
                       "true",
                       isNumerical() ? m_strValue : "\"" + str + "\"",
                       m_note);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setFromJSON
//
// "head": 2,
// "obid"; 123,
// "datetime": "2017-01-13T10:16:02",
// "timestamp":50817,
// "class": 10,
// "type": 8,
// "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
// "data": [1,2,3,4,5,6,7],
// "note": "This is some text"

bool
CVSCPVariable::setFromJSON(std::string &strVariable)
{
    try {

        auto j = json::parse(strVariable);

        // Mark last changed as now
        setLastChangedToNow();

        if (j.find("name") != j.end()) {
            m_name = j.at("name").get<std::string>();
        }

        if (j.find("type") != j.end()) {
            m_type = j.at("type").get<uint16_t>();
        }

        if (j.find("user") != j.end()) {
            m_userid = j.at("user").get<uint32_t>();
        }

        if (j.find("accessrights") != j.end()) {
            m_accessRights = j.at("accessrights").get<uint32_t>();
        }

        if (j.find("persistence") != j.end()) {
            m_bPersistent = j.at("persistence").get<bool>();
        }

        if (j.find("value") != j.end()) {

            std::string str;

            if (isNumerical()) {

                if (VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType()) {
                    int val = j.at("value").get<int>();
                    setValue(val);
                } else if (VSCP_DAEMON_VARIABLE_CODE_LONG == getType()) {
                    long val = j.at("value").get<long>();
                    setValue(val);
                } else if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType()) {
                    double val = j.at("value").get<double>();
                    setValue(val);
                }

            } else {

                if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType()) {
                    bool bVal = j.at("value").get<bool>();
                    setValue(bVal);
                } else {
                    str = j.at("value").get<std::string>();
                    setValue(str, true);
                }
            }
        }

        if (j.find("note") != j.end()) {
            std::string str = j.at("note").get<std::string>();
            setNote(str, false);
        }

    } catch (...) {
        syslog(LOG_ERR,
               ("Remote variable, setFromJSON: "
                "Failed to parse JSON object!"));
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsXML
//

bool
CVSCPVariable::getAsXML(std::string &strVariable)
{
    std::string str = m_strValue;
    vscp_base64_std_encode(str);

    vscp_str_format(strVariable,
                       VARIABLE_XML_TEMPLATE,
                       m_name,
                       (unsigned short int)m_type,
                       (unsigned long int)m_userid,
                       (unsigned long int)m_accessRights,
                       m_bPersistent ? ("true") : ("false"),
                       (const char *)m_lastChanged.getISODateTime().c_str(),
                       str,
                       m_note);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setFromXML
//

bool
CVSCPVariable::setFromXML(std::string &strVariable)
{

    std::string str;
    unsigned long lval;

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser, startLoadVarParser, endLoadVarParser);

    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        syslog(LOG_CRIT, "Failed to allocate buffer for XML parser (string)");
        return false;
    }

    memset(buf, 0, XML_BUFF_SIZE);
    memcpy(buf, strVariable.c_str(), strVariable.length());

    size_t size = strVariable.length();
    if (!XML_ParseBuffer(xmlParser, size, size == 0)) {
        syslog(LOG_ERR, "Failed parse XML configuration file.");
        XML_ParserFree(xmlParser);
        return false;
    }

    XML_ParserFree(xmlParser);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setName
//

bool
CVSCPVariable::setName(const std::string &strName)
{
    std::string str = strName;
    vscp_trim(str);

    if (str.empty()) {
        // Name can't be null
        syslog(LOG_ERR,
               "Setting variable name: "
               "Variable name can't be empty.");
        return false;
    }

    m_name = str;
    fixName();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setOwnerId
//

bool
CVSCPVariable::setOwnerId(uint32_t userid)
{
    // Check for admin user
    if (0 == userid) {
        m_userid = USER_ID_ADMIN;
        return true;
    }

    // Check if id is defined
    // if ( !CUserItem::isUserInDB( userid ) ) return false;

    // Assign userid
    m_userid = userid;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setOwnerIdFromUserName
//

bool
CVSCPVariable::setOwnerIdFromUserName(std::string &strUser)
{
    long userid;

    // Check for admin user
    if (0 == vscp_strcasecmp(gpobj->m_admin_user.c_str(), strUser.c_str())) {
        m_userid = USER_ID_ADMIN;
        return true;
    }

    if (!CUserItem::isUserInDB(strUser, &userid)) return false;

    m_userid = userid;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeValueToString
//

void
CVSCPVariable::writeValueToString(std::string &strValueOut, bool bBase64)
{
    strValueOut = m_strValue;

    if (!bBase64) {
        vscp_base64_std_decode(strValueOut);
    }
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

std::string
CVSCPVariable::getValue(bool bBase64)
{
    std::string str = m_strValue;
    if (bBase64) {
        vscp_base64_std_encode(str);
    }

    return str;
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVSCPVariable::getValue(std::string *pval, bool bBase64)
{
    std::string str = m_strValue;
    if (bBase64) {
        vscp_base64_std_encode(str);
    }

    *pval = str;
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVSCPVariable::setValue(bool val)
{
    vscp_str_format(m_strValue, "%s", val ? "true" : "false");
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVSCPVariable::getValue(bool *pValue)
{
    if (0 == vscp_strcasecmp(m_strValue.c_str(), "true")) {
        *pValue = true;
    } else {
        *pValue = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::setValue(int val)
{
    vscp_str_format(m_strValue, "%d", val);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::getValue(int *pValue)
{
    *pValue = (int)vscp_readStringValue(m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::setValue(long val)
{
    vscp_str_format(m_strValue, "%ld", val);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::getValue(long *pValue)
{
    *pValue = vscp_readStringValue(m_strValue);
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::setValue(double val)
{
    vscp_str_format(m_strValue, "%lf", val);
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVSCPVariable::getValue(double *pValue)
{
    *pValue = stod(m_strValue);
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void
CVSCPVariable::setValue(vscpEvent &event)
{
    vscp_writeVscpEventToString(&event, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void
CVSCPVariable::getValue(vscpEvent *pEvent)
{
    if (NULL != pEvent) return;
    vscp_setVscpEventFromString(pEvent, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void
CVSCPVariable::setValue(vscpEventEx &eventex)
{
    vscp_writeVscpEventExToString(&eventex, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void
CVSCPVariable::getValue(vscpEventEx *pEventEx)
{
    if (NULL != pEventEx) return;
    vscp_setVscpEventExFromString(pEventEx, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::getValue(vscpdatetime *pValue)
{
    m_strValue = pValue->getISODateTime();
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVSCPVariable::setValue(vscpdatetime &val)
{
    if (val.isValid()) {
        m_strValue = val.getISODateTime();
    } else {
        m_strValue = ("00-00-00T00:00:00");
    }
}

///////////////////////////////////////////////////////////////////////////////
// setValueFromString
//

bool
CVSCPVariable::setValueFromString(int type,
                                  const std::string &strValue,
                                  bool bBase64)
{
    std::string str = strValue;

    // Convert to uppercase
    std::string strUpper;

    // Must decode if encoded
    if (bBase64) {
        vscp_base64_std_decode(str);
    }

    // Update last changed
    setLastChangedToNow();

    switch (type) {

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN: {
            vscp_makeUpper(strUpper);
            if (0 == vscp_strcasecmp(strUpper.c_str(), "true")) {
                m_strValue = "true";
            } else {
                m_strValue = "false";
            }
        } break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER: {
            long lval;
            lval       = vscp_readStringValue(str);
            m_strValue = vscp_str_format("%d", (int)lval);
        } break;

        case VSCP_DAEMON_VARIABLE_CODE_LONG: {
            long lval;
            lval       = vscp_readStringValue(str);
            m_strValue = vscp_str_format("%ld", lval);
        } break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE: {
            double dval = std::stod(str);
            m_strValue  = vscp_str_format("%lf", dval);
        } break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATE:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_TIME:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_JSON:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_XML:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_SQL:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_LUA:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_USER:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:

            // Fall through - Not allowed here

        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getNote
//

std::string
CVSCPVariable::getNote(bool bBase64)
{
    std::string strNote = m_note;
    if (bBase64) vscp_base64_std_encode(strNote);

    return strNote;
}

///////////////////////////////////////////////////////////////////////////////
// getNote
//

bool
CVSCPVariable::getNote(std::string &strNote, bool bBase64)
{
    strNote = m_note;

    if (bBase64) vscp_base64_std_encode(strNote);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableFromString
//
// Format is: "variable
// name";"type";"persistence";"user";"rights";"value";"note"
//

bool
CVSCPVariable::setVariableFromString(const std::string &strVariable,
                                     bool bBase64,
                                     const std::string &strUser)
{
    std::string strRights;    // User rights for variable
    bool bPersistent = false; // Persistence of variable
    bool brw         = true;  // Writable

    // Update last changed
    setLastChangedToNow();

    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    // Name
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (str.length()) setName(str);
    } else {
        return false;
    }

    // Type
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (vscp_isNumber(str)) {
            m_type = vscp_readStringValue(str);
        } else {
            if (str.length()) {
                m_type = getVariableTypeFromString(str);
            } else {
                m_type = getVariableTypeFromString(("STRING"));
            }
        }
    } else {
        return false;
    }

    // Persistence
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(str);
        vscp_trim(str);
        if (str.length()) {
            if (0 == vscp_strcasecmp(str.c_str(), "true")) {
                setPersistent(true);
            } else {
                setPersistent(false);
            }
        } else {
            setPersistent(false);
        }
    } else {
        return false;
    }

    // User - Numeric or by name
    if (!tokens.empty()) {
        unsigned long userid = 0;
        std::string str      = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (str.length()) {
            userid = vscp_readStringValue(str);
            if (userid) {
                setOwnerId(userid);
            } else {
                std::string str = strUser;
                if (!setOwnerIdFromUserName(str)) {
                    return false;
                }
            }
        } else {
            std::string str = strUser;
            if (!setOwnerIdFromUserName(str)) {
                return false;
            }
        }
    } else {
        return false;
    }

    // Access rights
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (str.length()) {
            uint32_t rights = vscp_readStringValue(str);
            setAccessRights(rights);
        } else {
            setAccessRights(0x744);
        }
    } else {
        return false;
    }

    // Get the value of the variable
    if (!tokens.empty()) {

        std::string value = tokens.front();
        tokens.pop_front();
        vscp_trim(value);

        if (value.length()) {
            setValueFromString(m_type, value);
        } else {
            // If no value given use RESET value
            Reset();
        }

    } else {
        return false;
    }

    // Get the note (if any)
    // Get the value of the variable
    if (!tokens.empty()) {

        std::string note = tokens.front();
        tokens.pop_front();

        // If the string starts with "Base64" it should be encoded
        std::string str = note;
        vscp_std_decodeBase64IfNeeded(str, note);

        setNote(note);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

bool
CVSCPVariable::setValue(const std::string &strValue, bool bBase64)
{
    std::string str = strValue;

    // Must decode if encoded
    if (bBase64) {
        vscp_base64_std_decode(str);
    }

    // Update last changed
    setLastChangedToNow();

    m_strValue = str;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setNote
//

bool
CVSCPVariable::setNote(const std::string &strNote, bool bBase64)
{
    std::string str = strNote;

    // Must decode if encoded
    if (bBase64) {
        vscp_base64_std_decode(str);
    }

    m_note = str;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Reset
//

void
CVSCPVariable::Reset(void)
{
    // Update lastchanged
    setLastChangedToNow();

    if (m_bPersistent) {
        // setValueFromString( m_type, m_bPersistent );
    } else {
        switch (m_type) {

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                m_strValue = ("false");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                m_strValue =
                  ("0,0,0,255,255"); // value,unit,sensor-index,zone,subzone
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                m_strValue =
                  ("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATE:
                m_strValue = vscpdatetime::Now().getISODate();
                break;

            case VSCP_DAEMON_VARIABLE_CODE_TIME:
                m_strValue = vscpdatetime::Now().getISOTime();
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                m_strValue = vscpdatetime::Now().getISODateTime();
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BLOB:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_MIME:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_HTML:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_JSON:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_XML:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_SQL:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LUA:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_USER:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_FILTER:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:

                // Fall through - Not allowed here

            default:
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// isTrue
//

bool
CVSCPVariable::isTrue(void)
{
    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() &&
        (0 == vscp_strcasecmp(m_strValue.c_str(), "TRUE"))) {
        return true;
    }

    if (VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() &&
        std::stoi(m_strValue)) {
        return true;
    }

    if (VSCP_DAEMON_VARIABLE_CODE_LONG == getType() && std::stol(m_strValue)) {
        return true;
    }

    if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() &&
        std::stod(m_strValue)) {
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setTrue
//

void
CVSCPVariable::setTrue(void)
{
    // Update lastchanged
    setLastChangedToNow();

    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType()) {
        m_strValue = ("true");
    } else if (VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType()) {
        m_strValue = ("1");
    } else if (VSCP_DAEMON_VARIABLE_CODE_LONG == getType()) {
        m_strValue = ("1");
    } else if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType()) {
        m_strValue = ("1.0");
    } else if (VSCP_DAEMON_VARIABLE_CODE_STRING == getType()) {
        m_strValue = ("true");
    }
}

///////////////////////////////////////////////////////////////////////////////
// setFalse
//

void
CVSCPVariable::setFalse(void)
{
    // Update lastchanged
    setLastChangedToNow();

    if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType()) {
        m_strValue = ("false");
    } else if (VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType()) {
        m_strValue = ("0");
    } else if (VSCP_DAEMON_VARIABLE_CODE_LONG == getType()) {
        m_strValue = ("0");
    } else if (VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType()) {
        m_strValue = ("0");
    } else if (VSCP_DAEMON_VARIABLE_CODE_STRING == getType()) {
        m_strValue = ("false");
    }
}

//*****************************************************************************
//                       V A R I A B L E   S T O R A G E
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVariableStorage::CVariableStorage()
{
    m_db_vscp_external_variable = NULL;
    m_db_vscp_internal_variable = NULL;

    // Set the default dm configuration path
    m_xmlPath    = "/srv/vscp/variable.xml";
    m_dbFilename = "/srv/vscp/variable.sqlite3";

    // We just read variables
    m_lastSaveTime = vscpdatetime::Now();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVariableStorage::~CVariableStorage()
{
    // Close the internal variable database
    if (NULL != m_db_vscp_internal_variable) {
        sqlite3_close(m_db_vscp_internal_variable);
        m_db_vscp_internal_variable = NULL;
    }

    // Close the external variable database
    if (NULL != m_db_vscp_external_variable) {
        sqlite3_close(m_db_vscp_external_variable);
        m_db_vscp_external_variable = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// init
//

bool
CVariableStorage::init(void)
{
    std::string str;
    CVSCPVariable variable;

    // * * * VSCP Daemon external variable database * * *

    // fprintf( stderr, str.c_str() );
    syslog(LOG_DEBUG,
           "[Variable database init] Path=%s\n",
           (const char *)m_dbFilename.c_str());

    // Check filename
    if (vscp_fileExists(m_dbFilename)) {

        if (SQLITE_OK != sqlite3_open((const char *)m_dbFilename.c_str(),
                                      &m_db_vscp_external_variable)) {

            // Failed to open/create the database file
            syslog(LOG_ERR,
                   "[Variable database init] VSCP Daemon external "
                   "variable database could not be opened."
                   " - Will not be used."
                   "Path=%s error=%s",
                   (const char *)m_dbFilename.c_str(),
                   sqlite3_errmsg(m_db_vscp_external_variable));
        }

    } else {

        // We need to create the database from scratch. This may not work if
        // the database is in a read only location.
        syslog(LOG_INFO,
               "[Variable database init] VSCP Daemon external "
               "variable database does not exist - will be created. Path=%s",
               (const char *)m_dbFilename.c_str());

        if (SQLITE_OK == sqlite3_open((const char *)m_dbFilename.c_str(),
                                      &m_db_vscp_external_variable)) {
            // create the table.
            doCreateExternalVariableTable();

        } else {

            // Failed to create the variable database
            syslog(LOG_ERR,
                   "[Variable database init] VSCP "
                   "Daemon external variable database open  Err=%s",
                   sqlite3_errmsg(m_db_vscp_external_variable));
        }
    }

    // * * * VSCP Daemon internal variable database - Always created in-memory *
    // * *

    if (SQLITE_OK == sqlite3_open(":memory:", &m_db_vscp_internal_variable)) {
        // Should always be created
        doCreateInternalVariableTable();
    } else {
        // Failed to open/create the database file
        syslog(LOG_ERR,
               "[Variable database init] VSCP Daemon internal "
               "variable database could not be opened - "
               "Will not be used."
               "Error=%s",
               sqlite3_errmsg(m_db_vscp_internal_variable));
        if (NULL != m_db_vscp_internal_variable)
            sqlite3_close(m_db_vscp_internal_variable);
        m_db_vscp_internal_variable = NULL;
    }

    //**************************************************************************
    //                    I N I T   S T O C K  V A R I A B L E S
    //
    // Stock variables is added to the internal variable database for
    // sorting/listing. !!!! Values are not read or written here !!!!
    //**************************************************************************

    // Set common values
    variable.setID(ID_NON_PERSISTENT);
    variable.setStockVariable(true);
    variable.setPersistent(false);
    variable.setOwnerId(USER_ID_ADMIN);
    variable.setLastChangedToNow();
    variable.setNote((""));

    // *************************************************************************
    //                               Version
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.version.major"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("VSCP major version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.version.minor"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("VSCP minor version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.version.release"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("VSCP release version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.version.build"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("VSCP build version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.version.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("VSCP version string."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("VSCP copyright."));
    addStockVariable(variable);

    // *************************************************************************
    //                                SQLite
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.version.major"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("SQLite3 major version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.version.minor"));
    variable.setNote(("SQLite3 minor version number."));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.version.release"));
    variable.setNote(("SQLite3 release version number."));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.version.build"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("SQLite3 build version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.version.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("SQLite3 version string."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.sqlite.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("SQLite3 copyright."));
    addStockVariable(variable);

    // *************************************************************************
    //                               OPENSSL
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.openssl.version.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Open SSL version string."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.openssl.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Open SSL copyright string."));
    addStockVariable(variable);

    // *************************************************************************
    //                               Duktape
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.duktape.version.major"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Duktape major version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.duktape.version.minor"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Duktape minor version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.duktape.version.release"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Duktape release version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.duktape.version.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Version for Duktape library."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.duktape.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Licens for Duktape library."));
    addStockVariable(variable);

    // *************************************************************************
    //                                Civitweb
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.civetweb.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Licens for Civetweb library."));
    addStockVariable(variable);

    // *************************************************************************
    //                                 Lua
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.lua.version.major"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("LUA major version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.lua.version.minor"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("LUA minor version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.lua.version.release"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("LUA release version number."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.lua.version.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("LUA version string."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.lua.copyright"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("LUA copyright."));
    addStockVariable(variable);

    // *************************************************************************
    //                               System
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Operating system information string."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.width"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Operating system CPU word width (numerical)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.width.is64bit"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("True if operating system CPU word width is 64-bit."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.width.is32bit"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("True if operating system CPU word width is 32-bit."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.width.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Operating system CPU word width (text form)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.endiness.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Operating system endiness."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.os.endiness.islittleendian"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("True if Operating system endiness is little endian."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.name"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Name of host machine."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.ip"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("IP address for host machine."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.mac"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("MAC address for host machine."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.userid"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("The user the VSCP daemon is running as."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.username"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Full name for the user the VSCP daemon is running as."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.guid"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
    variable.setNote(("GUID for host."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.loglevel"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Active log level (numerical form)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.loglevel.str"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Active log level (text form)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.client.receivequeue.max"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Maximum number of events in client receive queue."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.host.root.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Root path for VSCP daemon."));
    addStockVariable(variable);

    // *************************************************************************
    //                             TCP/IP
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.tcpip.address"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Address for VSCP daemon TCP/IP interface."));
    addStockVariable(variable);

    // *************************************************************************
    //                               Multicast
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.multicast.address"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Address for VSCP daemon multicast TCP/IP "
                      "interface (Default: 224.0.23.158)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.multicast.ttl"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("TTL for VSCP daemon multicast TCP/IP interface "
                      "(Default: 224.0.23.158)."));
    addStockVariable(variable);

    // *************************************************************************
    //                                UDP
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.udp.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon simple UDP interface."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.udp.address"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Address for VSCP daemon simple UDP interface."));
    addStockVariable(variable);

    // *************************************************************************
    //                              Discovery
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.discovery.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon discovery."));
    addStockVariable(variable);

    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.calc.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last VSCP daemon automation "
                      "calculation."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.longitude"));
    variable.setNote(("VSCP daemon automation longitude setting."));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.latitude"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
    variable.setNote(("VSCP daemon automation latitude setting."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.heartbeat.enable"));
    variable.setNote(
      ("Enable flag for VSCP daemon automation heartbeat event."));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.heartbeat.period"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Period in seconds for VSCP daemon automation heartbeat "
                      "event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.heartbeat.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "heart beat event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.segctrl-heartbeat.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation segment "
                      "controller heartbeat event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.segctrl-heartbeat.period"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Period in seconds for VSCP daemon automation heart "
                      "beat event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.segctrl-heartbeat.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "heart beat event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.daylength"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("VSCP daemon automation calculated daylength."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.declination"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
    variable.setNote(("VSCP daemon automation calculated sun declination."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.sun.max.altitude"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
    variable.setNote(("VSCP daemon automation calculated max sun altitude."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.twilightsunriseevent.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation twilight "
                      "sunrise event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.twilightsunriseevent.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "twilight sunrise event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.sunriseevent.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation sunrise "
                      "event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.sunriseevent.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "sunrise event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.sunsetevent.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation sunset event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.sunsetevent.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "sunset event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.twilightsunsetevent.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation twilight sunset "
                      "event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.twilightsunsetevent.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "twilight sunset event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.automation.calculatednoonevent.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable flag for VSCP daemon automation calculated "
                      "noon event."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.automation.calculatednoonevent.last"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
    variable.setNote(("Date and time for last sent VSCP daemon automation "
                      "calculated noon event."));
    addStockVariable(variable);

    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable the VSCP server web interface."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.document_root"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Root folder for VSCP server web interface."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.listening_ports"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("VSCP server web interface listening ports."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.index_files"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("VSCP server web interface index files."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.authentication_domain"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Authentication domain for VSCP server web interface."),
                     true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.enable_auth_domain_check"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable the VSCP server web interface authentication "
                      "domain check."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_certificat"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to SSL certification for VSCP server web "
                      "interface."),
                     true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_certificat_chain"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("T.B.D."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_verify_peer"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(
      ("Enable client's certificate verification by the server."), true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_ca_path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(
      ("Name of a directory containing trusted CA certificates."), true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_ca_file"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to a .pem file containing trusted certificates."),
                     true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_verify_depth"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Sets maximum depth of certificate chain."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_default_verify_paths"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Loads default trusted certificates locations set at "
                      "openssl compile time."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_cipher_list"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("List of ciphers to present to the client."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_protocol_version"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Sets the minimal accepted version of accepted SSL/TLS"
                      " protocol."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssl_short_trust"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enables the use of short lived certificates."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.cgi_interpreter"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to an executable to be used use as an interpreter "
                      "for all CGI scripts regardless script extension. "),
                     true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.cgi_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All files that match cgi_patterns are treated as CGI "
                      "files."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.cgi_environment"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Set extra variables to be passed to the CGI script "
                      "in addition to the standard environment variables."),
                     true);
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.protect_uri"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("This is a comma separated list of URI=PATH pairs, "
                      "specifying that given URIs must be protected with "
                      "password files specified by PATH."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.trottle"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Limit download speed for clients."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.enable_directory_listing"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable directory listing."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.enable_keep_alive"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable connection keep alive."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.keep_alive_timeout_ms"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Idle timeout between two requests in one keep-alive "
                      "connection."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.access_control_list"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Set which remote hosts are allowed to connect."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.extra_mime_types"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Extra mime types to recognize."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.num_threads"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of worker threads."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.run_as_user"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Switch to given user credentials after startup."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.hide_file_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("A pattern for the files to hide."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.url_rewrite_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("URL rewrites for VSCP daemon web interface."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.global_auth_file"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to global authentication file. Leave blank for no "
                      "authentication."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.per_directory_auth_file"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote((" To enable directory protection with authentication, "
                      "set this to “.htpasswd”."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.url_rewrite_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Comma-separated list of URL rewrites in the form of "
                      "uri_pattern=file_or_directory_path."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.ssi_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All files that match ssi_pattern are treated as Server "
                      "Side Includes (SSI)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.request_timeout_ms"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Timeout for network read and network write operations, "
                      "in milliseconds."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.linger_timeout_ms"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Set TCP socket linger timeout before closing sockets "
                      "(SO_LINGER option)."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.decode_url"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(
      ("Enable URL encodeding of request strings in the server."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.access_control_allow_origin"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Access-Control-Allow-Origin header field."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.access_control_allow_methods"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Access-Control-Allow-Methods header field."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.access_control_allow_headers"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Access-Control-Allow-Methods header field."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.error_pages"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("This option may be used to specify a directory for "
                      "user defined error pages."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName("vscp." VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY);
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(
      ("Enable TCP_NODELAY socket option on client connections."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.static_file_max_age"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Set the maximum time (in seconds) a cache may store "
                      "a static files."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.strict_transport_security_max_age"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Set the Strict-Transport-Security header, and set "
                      "the max-age value."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.allow_sendfile_call"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("This option can be used to enable or disable the "
                      "use of the Linux sendfile system call."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.additional_header"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Send additional HTTP response header line for "
                      "every request."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.max_request_size"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Maximum request size URL."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.allow_index_script_resource"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Index scripts (like index.cgi or index.lua) may have "
                      "script handled resources."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.duktape_script_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("A pattern for files that are interpreted as "
                      "JavaScripts by the server."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_preload_file"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("This configuration option can be used to specify a "
                      "Lua script file, which is executed before the actual "
                      "web page script."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_script_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("A pattern for files that are interpreted as Lua "
                      "scripts by the server."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_server_page_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Files matching this pattern are treated as Lua "
                      "server pages."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_websocket_patterns"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote((""));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_background_script"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Run a Lua script in the background, independent from "
                      "any connection."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websrv.lua_background_script_params"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Used to add dynamic parameters to background script."));
    addStockVariable(variable);

    // *************************************************************************
    //                              Websockets
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websockets.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Enable websocket functionality."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websockets.document_root"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Rootfolder for websockets."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.websockets.timeout_ms"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Websocket timeout in milliseconds."));
    addStockVariable(variable);

    // *************************************************************************
    //                            Decision Matrix
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.loglevel"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("DM log level 'normal' or 'debug'."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.db.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to decision matrix database."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.xml.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to decision matrix XML file to load at startup."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.allow.xml.save"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(
      ("Allow write of decision matrix XML file after edit/delete."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Total number of decision matrix rows."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.dm.count.active"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Number of active decision matrix rows."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.dm"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Full decision matrix (all rows)."));
    addStockVariable(variable);

    // *************************************************************************
    //                               Debug
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags1"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 1."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags2"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 2."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags3"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 3."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags4"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 4."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags5"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 5."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags6"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 6."));
    addStockVariable(variable);
    variable.init();

    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags7"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 7."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.debug.flags8"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("VSCP debug flags 8."));
    addStockVariable(variable);

    // *************************************************************************
    //                             Variables
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.variable.xml.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to XML file to load variables from on startup."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.variable.db.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to variable database."));
    addStockVariable(variable);

    // *************************************************************************
    //                              Log files
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.syslog.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Flag to enable syslog."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.database.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Flag to enable logging to database."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.database.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to logging database."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.log.database.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
    variable.setNote(("Number of records in log database."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.database.sql"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_SQL);
    variable.setNote(("SQL select for log database retrieval (Default: 'ORDER "
                      "BY date DESC LIMIT 500) ."));
    // SELECT * FROM log WHERE date BETWEEN '2016-12-01' AND '2011-11-02';
    // SELECT * FROM log WHERE strftime('%H',date)  BETWEEN '12' AND '13';
    variable.setValue(("ORDER BY date DESC LIMIT 500"));
    addStockVariable(variable);

    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.log.database.search"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Log database search result."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.general.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Flag to enable general logging to a text file."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.general.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to general logging text file."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.access.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Flag to enable access logging to a text file."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.access.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to access logging text file."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.security.enable"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Flag to enable security logging to a text file."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.log.security.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to security logging text file."));
    addStockVariable(variable);

    // *************************************************************************
    //                             Databases
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.database.vscpdata.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to VSCP data database."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ | PERMISSON_OWNER_WRITE);
    variable.setName(("vscp.database.vscpdconfig.path"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Path to VSCP daemon main database."));
    addStockVariable(variable);

    // *************************************************************************
    //                              Drivers
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of loaded drivers."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.level1.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of Level I drivers."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.level2.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of level II drivers."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.add"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Add a driver."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.delete"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Delete a driver."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.start"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Start a driver."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.pause"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Pause a driver."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver.stop"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
    variable.setNote(("Stop a driver."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.driver"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All driver info."));
    addStockVariable(variable);

    // *************************************************************************
    //                             Interfaces
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.interface.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of active interfaces."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.interface"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All interfaces."));
    addStockVariable(variable);

    // *************************************************************************
    //                              Discovery
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.discovery.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of discovered units."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.discovery"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All discovered units."));
    addStockVariable(variable);

    // *************************************************************************
    //                                Users
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setOwnerID(0);
    variable.setName(("vscp.user.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of defined users."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_OWNER_ALL);
    variable.setOwnerID(0);
    variable.setName(("vscp.user"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All defined users."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setOwnerID(0);
    variable.setName(("vscp.user.names"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All defined users names."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_OWNER_WRITE);
    variable.setOwnerID(0);
    variable.setStockVariable();
    variable.setName(("vscp.user.add"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Add a new user."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_OWNER_WRITE);
    variable.setOwnerID(0);
    variable.setStockVariable();
    variable.setName(("vscp.user.delete"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("Delete a user."));
    addStockVariable(variable);

    // *************************************************************************
    //                                Tables
    // *************************************************************************

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.table.count"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
    variable.setNote(("Number of defined user tables."));
    addStockVariable(variable);

    variable.init();
    variable.setAccessRights(PERMISSON_ALL_READ);
    variable.setName(("vscp.table"));
    variable.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
    variable.setNote(("All user tables."));
    addStockVariable(variable);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// exist
//

uint32_t
CVariableStorage::exist(const std::string &name)
{
    CVSCPVariable variable;

    return find(name, variable);
}

///////////////////////////////////////////////////////////////////////////////
// find
//

uint32_t
CVariableStorage::find(const std::string &name, CVSCPVariable &variable)
{
    uint32_t id         = 0;
    std::string lc_name = vscp_lower(name);

    // Look for stock variables
    if (vscp_startsWith(lc_name, "vscp.")) {
        return getStockVariable(name, variable);
    } else {

        // Search for non-persistent
        if (id = findNonPersistentVariable(name, variable)) {
            return id;
        } else {
            return findPersistentVariable(name, variable);
        }
    }

    return 0; // Not found
}

///////////////////////////////////////////////////////////////////////////////
// getStockVariable
//

uint32_t
CVariableStorage::getStockVariable(const std::string &name,
                                   CVSCPVariable &var,
                                   CUserItem *pUser)
{
    std::string str;
    std::string strToken;
    std::string strrest;
    uint32_t varid = 0;

    var.init();
    var.setStockVariable();

    std::string lcname;
    vscp_lower(lcname);

    std::deque<std::string> tokens;
    vscp_split(tokens, lcname, ".");

    if (tokens.empty()) return false;
    strToken = tokens.front();
    tokens.pop_front();

    // Make sure it starts with ".vscp"
    if ("vscp" != strToken) return false;

    // Get persistent variable
    varid = findNonPersistentVariable(name, var);
    // if ( 0 == varid ) return 0;

    // *************************************************************************
    //                            Debug flags
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.debug")) {

        if (vscp_startsWith(lcname, "vscp.debug.flags1")) {
            var.setValue((long)gpobj->m_debugFlags1);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags2")) {
            var.setValue((long)gpobj->m_debugFlags2);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags3")) {
            var.setValue((long)gpobj->m_debugFlags3);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags4")) {
            var.setValue((long)gpobj->m_debugFlags4);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags5")) {
            var.setValue((long)gpobj->m_debugFlags5);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags6")) {
            var.setValue((long)gpobj->m_debugFlags6);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags7")) {
            var.setValue((long)gpobj->m_debugFlags7);
            return var.getID();
        }

        if (vscp_startsWith(lcname, "vscp.debug.flags8")) {
            var.setValue((long)gpobj->m_debugFlags8);
            return var.getID();
        }
    }

    // *************************************************************************
    //                            VSCP version info.
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.version.major")) {
        var.setValue(vscp_str_format("%d", VSCPD_MAJOR_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.version.minor")) {
        var.setValue(vscp_str_format("%d", VSCPD_MINOR_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.version.release")) {
        var.setValue(vscp_str_format("%d", VSCPD_RELEASE_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.version.build")) {
        var.setValue(vscp_str_format("%d", VSCPD_BUILD_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.version.str")) {
        var.setValue(vscp_str_format("%s", VSCPD_DISPLAY_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.copyright")) {
        var.setValue(vscp_str_format("%s", VSCPD_COPYRIGHT));
        return var.getID();
    }

    // *************************************************************************
    //                                SQLite
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.sqlite.version.major")) {
        int major, minor, sub, build;
        sscanf(SQLITE_VERSION, "%d.%d.%d.%d", &major, &minor, &sub, &build);
        var.setValue(vscp_str_format("%d", major));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.minor")) {
        int major, minor, sub, build;
        sscanf(SQLITE_VERSION, "%d.%d.%d.%d", &major, &minor, &sub, &build);
        var.setValue(vscp_str_format("%d", minor));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.release")) {
        int major, minor, sub, build;
        sscanf(SQLITE_VERSION, "%d.%d.%d.%d", &major, &minor, &sub, &build);
        var.setValue(vscp_str_format("%d", sub));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.build")) {
        int major, minor, sub, build;
        sscanf(SQLITE_VERSION, "%d.%d.%d.%d", &major, &minor, &sub, &build);
        var.setValue(vscp_str_format("%d", build));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.str")) {
        var.setValue((SQLITE_VERSION));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.copyright")) {
        var.setValue(vscp_str_format("%s", "SQLite Is Public Domain"));
        return var.getID();
    }

    // *************************************************************************
    //                               OPENSSL
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.openssl.version.str")) {
#ifdef WIN32
        var.setValue((OPENSSL_VERSION_TEXT));
#else
        // http://linux.die.net/man/3/ssleay
        // var.setValue( ("OPENSSL_VERSION_NUMBER") );
        var.setValue(vscp_str_format("%s", SSLeay_version(SSLEAY_VERSION)));
        return var.getID();
#endif
    }

    if (vscp_startsWith(lcname, "vscp.openssl.copyright")) {
        var.setValue("Copyright © 1999-2018, OpenSSL Software Foundation. ");
        return var.getID();
    }

    // *************************************************************************
    //                                 LUA
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.lua.version.major")) {
        var.setValue(LUA_VERSION_MAJOR);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.minor")) {
        var.setValue(LUA_VERSION_MINOR);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.release")) {
        var.setValue(LUA_VERSION_RELEASE);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.str")) {
        var.setValue(vscp_str_format("%s", LUA_RELEASE));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.lua.copyright")) {
        var.setValue(vscp_str_format("%s", LUA_COPYRIGHT));
        return var.getID();
    }

    // *************************************************************************
    //                                 Duktape
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.duktape.version.major")) {
        int dukmajor = DUK_VERSION / 10000;
        var.setValue(DUK_VERSION / 10000);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.duktape.version.minor")) {
        var.setValue((DUK_VERSION - (DUK_VERSION / 10000)) / 100);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.duktape.version.release")) {
        var.setValue(DUK_VERSION - (DUK_VERSION / 100));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.duktape.version.str")) {
        var.setValue(vscp_str_format("%s", DUK_GIT_DESCRIBE));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.duktape.copyright")) {
        var.setValue("Duktape copyrights are held by its authors. "
                     "(https://github.com/grodansparadis/vscp/blob/"
                     "master/src/common/duktape.h)",
                     true);
        return var.getID();
    }

    // *************************************************************************
    //                                Civitweb
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.civetweb.copyright")) {
        var.setValue(
          ("Copyright (c) 2004-2013 Sergey Lyubka, Copyright (c) "
           "2013-2018 the Civetweb developers, (mods.) Copyright (c) 2017-2018 "
           "Ake Hedman, Grodans Paradis AB"),
          true);
        return var.getID();
    }

    // *************************************************************************
    //                               System
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.os.str")) {
        // TODO var.setValue( xxGetOsDescription() );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.width.str")) {
        // TODO var.setValue( xxIsPlatform64Bit() ? ("64") : ("32") );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.width.is64bit")) {
        // TODO var.setValue( xxIsPlatform64Bit() ? true : false );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.width.is32bit")) {
        // TODO var.setValue( !xxIsPlatform64Bit() ? true : false );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.width")) {
        // TODO var.setValue( xxIsPlatform64Bit() ? 64 : 32 );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.endiness.str")) {
        var.setValue(vscp_isLittleEndian() ? ("Little endian")
                                           : ("Big endian"));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.os.endiness.islittleendian")) {
        var.setValue(vscp_isLittleEndian() ? true : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.name")) {
        // TODO var.setValue( xxGetFullHostName() );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.ip")) {
        cguid guid;

        if (!gpobj->getIPAddress(guid)) {
            guid.clear();
        }

        var.setValue(vscp_str_format("%d.%d.%d.%d",
                                        guid.getAt(11),
                                        guid.getAt(10),
                                        guid.getAt(9),
                                        guid.getAt(8)));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.mac")) {
        cguid guid;

        if (!gpobj->getMacAddress(guid)) {
            guid.clear();
        }

        var.setValue(vscp_str_format("%02X:%02X:%02X:%02X:%02X:%02X",
                                        guid.getAt(13),
                                        guid.getAt(12),
                                        guid.getAt(11),
                                        guid.getAt(10),
                                        guid.getAt(9),
                                        guid.getAt(8)));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.userid")) {
        // TODO var.setValue( xxGetUserId() );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.username")) {
        // TODO var.setValue( xxGetUserName() );
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.guid")) {
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.loglevel.str")) {
        switch (gpobj->m_logLevel) {
            case DAEMON_LOGMSG_NONE:
                str = ("None");
                break;
            case DAEMON_LOGMSG_NORMAL:
                str = ("Normal");
                break;
            case DAEMON_LOGMSG_DEBUG:
                str = ("Debug");
                break;
            default:
                str = ("Unknown");
                break;
        }
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.loglevel")) {
        var.setValue(vscp_str_format("%d ", gpobj->m_logLevel));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.client.receivequeue.max")) {
        var.setValue(
          vscp_str_format("%d", gpobj->m_maxItemsInClientReceiveQueue));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.host.root.path")) {
        var.setValue(gpobj->m_rootFolder);
        return var.getID();
    }

    // *************************************************************************
    //                                TCP/IP
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.tcpip.address")) {
        var.setValue(gpobj->m_strTcpInterfaceAddress);
        return var.getID();
    }

    // *************************************************************************
    //                                WEB-Server
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.websrv.enable")) {
        // Boolean
        var.setValue(gpobj->m_web_document_root);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.document_root")) {
        // String
        var.setValue(gpobj->m_web_document_root);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.listening_ports")) {
        // String
        var.setValue(gpobj->m_web_listening_ports);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.index_files")) {
        // String
        var.setValue(gpobj->m_web_index_files);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.authentication_domain")) {
        // String
        var.setValue(gpobj->m_web_authentication_domain);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_auth_domain_check")) {
        // Boolean
        var.setValue(gpobj->m_enable_auth_domain_check);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_certificat")) {
        // String
        var.setValue(gpobj->m_web_ssl_certificate);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.certificat_chain")) {
        // String
        var.setValue(gpobj->m_web_ssl_certificate);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_verify_peer")) {
        // Boolean
        var.setValue(gpobj->m_web_ssl_verify_peer);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_ca_path")) {
        // String
        var.setValue(gpobj->m_web_ssl_ca_path);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_ca_file")) {
        // String
        var.setValue(gpobj->m_web_ssl_ca_file);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_verify_depth")) {
        // Integer
        var.setValue(gpobj->m_web_ssl_verify_depth);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_default_verify_paths")) {
        // Boolean
        var.setValue(gpobj->m_web_ssl_default_verify_paths);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_cipher_list")) {
        // String
        var.setValue(gpobj->m_web_ssl_cipher_list);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_protocol_version")) {
        // Integer
        var.setValue(gpobj->m_web_ssl_protocol_version);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_short_trust")) {
        // Boolean
        var.setValue(gpobj->m_web_ssl_short_trust);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_interpreter")) {
        // String
        var.setValue(gpobj->m_web_cgi_interpreter);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_patterns")) {
        // String
        var.setValue(gpobj->m_web_cgi_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_environment")) {
        // String
        var.setValue(gpobj->m_web_cgi_environment);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.protect_uri")) {
        // String
        var.setValue(gpobj->m_web_protect_uri);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.trottle")) {
        // String
        var.setValue(gpobj->m_web_trottle);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_directory_listing")) {
        // Boolean
        var.setValue(gpobj->m_web_enable_directory_listing);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_keep_alive")) {
        // Boolean
        var.setValue(gpobj->m_web_enable_keep_alive);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.keep_alive_timeout_ms")) {
        // Long
        var.setValue(gpobj->m_web_keep_alive_timeout_ms);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_list")) {
        // String
        var.setValue(gpobj->m_web_access_control_list);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.extra_mime_types")) {
        // String
        var.setValue(gpobj->m_web_extra_mime_types);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.num_threads")) {
        // Integer
        var.setValue(gpobj->m_web_num_threads);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.run_as_user")) {
        // String
        var.setValue(gpobj->m_web_run_as_user);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.url_rewrite_patterns")) {
        // String
        var.setValue(gpobj->m_web_url_rewrite_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.hide_file_patterns")) {
        // String
        var.setValue(gpobj->m_web_hide_file_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.request_timeout_ms")) {
        // Long
        var.setValue(gpobj->m_web_request_timeout_ms);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.linger_timeout_ms")) {
        // Long
        var.setValue(gpobj->m_web_linger_timeout_ms);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.decode_url")) {
        // Boolean
        var.setValue(gpobj->m_web_decode_url);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.global_auth_file")) {
        // String
        var.setValue(gpobj->m_web_global_auth_file);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.per_directory_auth_file")) {
        // String
        var.setValue(gpobj->m_web_per_directory_auth_file);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssi_patterns")) {
        // String
        var.setValue(gpobj->m_web_ssi_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_origin")) {
        // String
        var.setValue(gpobj->m_web_access_control_allow_origin);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_methods")) {
        // String
        var.setValue(gpobj->m_web_access_control_allow_methods);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_headers")) {
        // String
        var.setValue(gpobj->m_web_access_control_allow_headers);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.error_pages")) {
        // String
        var.setValue(gpobj->m_web_error_pages);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.tcp_nodelay")) {
        // Long
        var.setValue(gpobj->m_web_tcp_nodelay);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.static_file_max_age")) {
        // Long
        var.setValue(gpobj->m_web_static_file_max_age);
        return var.getID();
    }

    if (vscp_startsWith(lcname,
                        "vscp.websrv.strict_transport_security_max_age")) {
        // Long
        var.setValue(gpobj->m_web_strict_transport_security_max_age);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.allow_sendfile_call")) {
        // Boolean
        var.setValue(gpobj->m_web_allow_sendfile_call);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.additional_header")) {
        // String
        var.setValue(gpobj->m_web_additional_header);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.max_request_size")) {
        // Long
        var.setValue(gpobj->m_web_max_request_size);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.allow_index_script_resource")) {
        // Boolean
        var.setValue(gpobj->m_web_allow_index_script_resource);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.duktape_script_patterns")) {
        // String
        var.setValue(gpobj->m_web_duktape_script_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_preload_file")) {
        // String
        var.setValue(gpobj->m_web_lua_preload_file);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_script_patterns")) {
        // String
        var.setValue(gpobj->m_web_lua_script_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_server_page_patterns")) {
        // String
        var.setValue(gpobj->m_web_lua_server_page_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_websocket_patterns")) {
        // String
        var.setValue(gpobj->m_web_lua_websocket_patterns);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_background_script")) {
        // String
        var.setValue(gpobj->m_web_lua_background_script);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_background_script_params")) {
        // String
        var.setValue(gpobj->m_web_lua_background_script_params);
        return var.getID();
    }

    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.websockets.enable")) {
        // Boolean
        var.setValue(gpobj->m_bWebsocketsEnable);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websockets.document_root")) {
        // String
        if (0 != gpobj->m_websocket_document_root.length()) {
            var.setValue(gpobj->m_websocket_document_root);
        } else {
            // If not set webserver uses web document root
            var.setValue(gpobj->m_web_document_root);
        }
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.websockets.timeout_ms")) {
        // Long
        var.setValue((long)gpobj->m_websocket_timeout_ms);
        return var.getID();
    }

    // *************************************************************************
    //                              Discovery
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.discovery.enable")) {
        var.setValue(true); // TODO
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.discovery.count")) {
        var.setValue(1); // TODO
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.discovery")) {
        var.setValue(("Functionality has not been implemented yet")); // TODO
        return var.getID();
    }

    // *************************************************************************
    //                             Multicast
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.multicast.address")) {
        var.setValue(gpobj->m_strMulticastAnnounceAddress);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.multicast.ttl")) {
        var.setValue(gpobj->m_ttlMultiCastAnnounce);
        return var.getID();
    }

    // *************************************************************************
    //                                UDP
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.udp.enable")) {
        var.setValue(gpobj->m_udpSrvObj.m_bEnable ? true : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.udp.address")) {
        var.setValue(gpobj->m_udpSrvObj.m_interface);
        return var.getID();
    }

    // *************************************************************************
    //                              AUTOMATION
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.automation.calc.last")) {
        str = gpobj->m_automation.getLastCalculation().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.longitude")) {
        var.setValue(
          vscp_str_format("%f", gpobj->m_automation.getLongitude()));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.latitude")) {
        var.setValue(
          vscp_str_format("%f", gpobj->m_automation.getLatitude()));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.enable")) {
        var.setValue(gpobj->m_automation.isSendHeartbeat() ? true : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.period")) {
        var.setValue(vscp_str_format(
          "%ld", gpobj->m_automation.getIntervalHeartbeat()));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.last")) {
        str = gpobj->m_automation.getHeartbeatSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl-heartbeat.enable")) {
        var.setValue(gpobj->m_automation.isSendSegmentControllerHeartbeat()
                       ? true
                       : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl-heartbeat.period")) {
        var.setValue(vscp_str_format(
          "%ld", gpobj->m_automation.getIntervalSegmentControllerHeartbeat()));
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl-heartbeat.last")) {
        str = gpobj->m_automation.getSegmentControllerHeartbeatSent()
                .getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.daylength")) {
        int hour, minute;
        double daylength = gpobj->m_automation.getDayLength();
        gpobj->m_automation.convert2HourMinute(daylength, &hour, &minute);
        vscp_str_format("%02d:%02d", hour, minute);
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.declination")) {
        double declination = gpobj->m_automation.getDeclination();
        vscp_str_format("%f", declination);
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.sun.max.altitude")) {
        double maxalt = gpobj->m_automation.getSunMaxAltitude();
        vscp_str_format("%f", maxalt);
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname,
                        "vscp.automation.twilightsunriseevent.enable")) {
        var.setValue(gpobj->m_automation.isSendSunriseTwilightEvent() ? true
                                                                      : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunriseevent.last")) {
        str = gpobj->m_automation.getSegmentControllerHeartbeatSent()
                .getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunriseevent.enable")) {
        var.setValue(gpobj->m_automation.isSendSunriseEvent() ? true : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunriseevent.last")) {
        str = gpobj->m_automation.getSunriseEventSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunsetevent.enable")) {
        var.setValue(gpobj->m_automation.isSendSunsetEvent() ? true : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunsetevent.last")) {
        str = gpobj->m_automation.getSunsetEventSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunsetevent.enable")) {
        var.setValue(gpobj->m_automation.isSendSunsetTwilightEvent() ? true
                                                                     : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunsetevent.last")) {
        str = gpobj->m_automation.getSunsetTwilightEventSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.calculatednoonevent.enable")) {
        var.setValue(gpobj->m_automation.isSendCalculatedNoonEvent() ? true
                                                                     : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.calculatednoonevent.last")) {
        str = gpobj->m_automation.getCalculatedNoonEventSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunsetevent.enable")) {
        var.setValue(gpobj->m_automation.isSendSunsetTwilightEvent() ? true
                                                                     : false);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunsetevent.last")) {
        str = gpobj->m_automation.getSunsetTwilightEventSent().getISODateTime();
        var.setValue(str);
        return var.getID();
    }

    // *************************************************************************
    //                            Decision Matrix (DM)
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.dm.db.path")) {
        var.setValue(gpobj->m_dm.m_path_db_vscp_dm);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm.xml.path")) {
        var.setValue(gpobj->m_dm.m_staticXMLPath);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm.allow.xml.save")) {
        var.setValue(gpobj->m_dm.bAllowXMLsave);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm.loglevel")) {
        if (gpobj->m_debugFlags1 & VSCP_DEBUG1_DM) {
            var.setValue("debug");
        } else {
            var.setValue("normal");
        }
        return var.getID();
    }

    // *************************************************************************
    //                             Variables
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.variable.db.path")) {
        var.setValue(gpobj->m_variables.m_dbFilename);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.variable.xml.path")) {
        var.setValue(gpobj->m_variables.m_xmlPath);
        return var.getID();
    }

    // *************************************************************************
    //                              Log files
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.log.access.enable")) {
        // Boolean
        var.setValue(false); // TODO
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.log.access.path")) {
        // String
        var.setValue(""); // TODO
        return var.getID();
    }

    // *************************************************************************
    //                             Databases
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.database.vscpdata.path")) {
        var.setValue(gpobj->m_path_db_vscp_data);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.database.vscpdconfig.path")) {
        var.setValue(gpobj->m_path_db_vscp_daemon);
        return var.getID();
    }

    // *************************************************************************
    //                             Decision Matrix
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.dm.count.active")) {
        var.setValue(gpobj->m_dm.getMemoryElementCount());
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm.count")) {
        var.setValue((long)gpobj->m_dm.getDatabaseRecordCount());
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm")) {
        var.setStockVariable();
        var.setPersistent(false);
        var.setAccessRights(PERMISSON_OWNER_READ);
        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
        gpobj->m_dm.getAllRows(str);
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.dm.")) {

        int pos;
        std::string str;
        str = vscp_str_right(name, name.length() - 8); // remove "vscp.dm."
        if (0 == vscp_strcasecmp(str.c_str(), ".")) {

            // this is a sub variable.
            std::string strID;
            unsigned long id;

            // Get id
            id = vscp_readStringValue(vscp_str_left(str, pos));

            // Get the remaining sub string
            str = vscp_str_right(str, str.length() - pos - 1);

            dmElement *pElement = NULL;
            if (!gpobj->m_dm.getDatabaseRecord(id, pElement)) return false;

            var.setStockVariable();
            var.setPersistent(false);

            if (vscp_startsWith(str, "id")) {
                var.setAccessRights(PERMISSON_OWNER_READ);
                ;
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "bEnable")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue(pElement->m_bEnable);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "groupid")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(pElement->m_strGroupID);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "mask.priority")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue(pElement->m_vscpfilter.mask_priority);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "mask.class")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(pElement->m_vscpfilter.mask_class);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "mask.type")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(pElement->m_vscpfilter.mask_type);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "mask.guid")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
                var.setValue(pElement->m_vscpfilter.mask_GUID);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "filter.priority")) {
                cguid guid;
                guid.getFromArray(pElement->m_vscpfilter.mask_GUID);
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
                var.setValue(guid.getAsString());
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "filter.class")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(pElement->m_vscpfilter.filter_class);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "filter.type")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(pElement->m_vscpfilter.filter_type);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "filter.guid")) {
                cguid guid;
                guid.getFromArray(pElement->m_vscpfilter.filter_GUID);
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID);
                var.setValue(guid.getAsString());
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.start")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.end")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_DATETIME);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.monday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.tuesday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.wednessday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.thursday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.friday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.saturday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.sunday")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "allowed.time")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "bCheckIndex")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "index")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "bCheckZone")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "zone")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "bCheckSubZone")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_BOOLEAN);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "subzone")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "measurement.value")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_DOUBLE);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "measurement.unit")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "measurement.compare")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "measurement.compare.string")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "measurement.comment")) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "count.trigger")) {
                var.setAccessRights(PERMISSON_OWNER_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(str, "count.error")) {
                var.setAccessRights(PERMISSON_OWNER_READ);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            } else if ((vscp_startsWith(str, "error")) ||
                       (vscp_startsWith(str, "error.string"))) {
                var.setAccessRights(PERMISSON_OWNER_READ |
                                    PERMISSON_OWNER_WRITE);
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue((long)pElement->m_id);
                return (UINT_MAX); // Fake variable id
            }

        } else {
            // A full DM row is requested ("vscp.dm.35" for row with id = 35)
            //???  TODO
        }
    }

    // *************************************************************************
    //                              Drivers
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.driver")) {

        if (vscp_startsWith(lcname, "vscp.driver.level1.count")) {
            // Integer
            var.setValue(0); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.level2.count")) {
            // Integer
            var.setValue(0); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.count")) {
            // Integer
            var.setValue(0); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.add")) {
            // Boolean
            var.setValue(false); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.delete")) {
            // Boolean
            var.setValue(false); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.start")) {
            // Boolean
            var.setValue(false); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.pause")) {
            // Boolean
            var.setValue(false); // TODO
            return var.getID();
        } else if (vscp_startsWith(lcname, "vscp.driver.stop")) {
            // Boolean
            var.setValue(false); // TODO
            return var.getID();
        }
        // "vscp.driver"
        else {
            // String
            var.setValue(("")); // TODO
            return var.getID();
        }
    }

    // *************************************************************************
    //                             Interfaces
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.interface")) {

        if (vscp_startsWith(lcname, "vscp.interface.count")) {
            // Integer
            var.setValue(0); // TODO
            return var.getID();
        }
        // "vscp.interface"
        else {
            // String
            var.setValue(("")); // TODO
            return var.getID();
        }
    }

    // *************************************************************************
    //                                Users
    // *************************************************************************
    if (vscp_startsWith(lcname, "vscp.user.count")) {
        // long
        var.setValue((long)gpobj->m_userList.getUserCount());
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.user.names")) {
        std::deque<std::string> array;
        str.clear();

        gpobj->m_userList.getAllUsers(array);
        for (int i = 0; i < array.size(); i++) {
            str += array[i];
            if (i != (array.size() - 1)) str += (",");
        }
        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
        var.setValue(str);
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.user.add")) {
        // holder for write only variable
        return var.getID();
    }

    if (vscp_startsWith(lcname, "vscp.user.delete")) {
        // holder for write only variable
        return var.getID();
    }

    // Individual user  (vscp.user.1 or vscp.user.1.name... )
    if (vscp_startsWith(lcname, "vscp.user.", &strrest)) {

        int pos;
        unsigned long idx;
        std::deque<std::string> tokens;
        vscp_split(tokens, strrest, ".");
        if (tokens.empty()) return 0;

        std::string strToken = tokens.front();
        tokens.pop_front(); // Get idx
        idx = stol(strToken);

        if (!tokens.empty()) {
            // vscp.user.n - return user record for index n
            if (!gpobj->m_userList.getUserAsString(idx, str)) return 0;
            var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
            var.setValue(str);
            return (UINT_MAX); // Fake variable id
        } else {
            // vscp.user.n.field
            CUserItem *pUserItem;
            if (!gpobj->m_userList.getUserAsString(idx, str))
                return 0; // Get name
            if (NULL ==
                (pUserItem = gpobj->m_userList.getUserItemFromOrdinal(idx)))
                return 0; // UserItem
            strToken = tokens.front();
            tokens.pop_front(); // Get field
            if (vscp_startsWith(strToken, "userid")) {
                str = vscp_str_format("%ld", pUserItem->getUserID());
                var.setType(VSCP_DAEMON_VARIABLE_CODE_LONG);
                var.setValue(str);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(strToken, "name")) {
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(pUserItem->getUserName());
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(strToken, "password")) {
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(pUserItem->getPassword());
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(strToken, "fullname")) {
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(pUserItem->getFullname());
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(strToken, "filter")) {
                var.setType(VSCP_DAEMON_VARIABLE_CODE_FILTER);
                var.setValue(str);
                return (UINT_MAX); // Fake variable id
            } else if (vscp_startsWith(strToken, "rights")) {
                // rights or rights.0..7
                if (tokens.empty()) {
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pUserItem->getUserRightsAsString());
                    return (UINT_MAX); // Fake variable id
                } else {
                    strToken = tokens.front();
                    tokens.pop_front(); // 0..7
                    idx = stol(strToken);
                    if (idx > 7) return false;
                    vscp_str_format("%d", idx);
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_INTEGER);
                    var.setValue(str);
                    return (UINT_MAX); // Fake variable id
                }
            } else if (vscp_startsWith(strToken, "allowed")) {
                // allowed.events or allowed.remotes
                if (tokens.empty()) return 0;
                strToken = tokens.front();
                tokens.pop_front(); // events/remotes
                if (("events") == strToken) {
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pUserItem->getAllowedEventsAsString());
                    return (UINT_MAX); // Fake variable id
                } else if (vscp_startsWith(strToken, "remotes")) {
                    var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                    var.setValue(pUserItem->getAllowedRemotesAsString());
                    return (UINT_MAX); // Fake variable id
                }
            } else if (vscp_startsWith(strToken, "note")) {
                var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
                var.setValue(pUserItem->getNote());
                return (UINT_MAX); // Fake variable id
            }
        }
    }

    if (vscp_startsWith(lcname, "vscp.user")) {
        gpobj->m_userList.getAllUsers(str);
        var.setType(VSCP_DAEMON_VARIABLE_CODE_STRING);
        var.setValue(str);
        return var.getID();
    }

    // *************************************************************************
    //                                Tables
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.table", &strrest)) {

        if (vscp_startsWith(lcname, "vscp.table.count")) {
            // Integer
            var.setValue(0); // TODO
            return var.getID();
        }
        // "vscp.table[....]"
        else {
            // String
            var.setValue(("")); // TODO
            return var.getID();
        }
    }

    // ----------------------------- Not Found ---------------------------------

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// putStockVariable
//

bool
CVariableStorage::putStockVariable(CVSCPVariable &var, CUserItem *pUser)
{
    std::string str;
    std::string lcname = vscp_lower(var.getName());

    // Make sure it starts with ".vscp"
    if (!vscp_startsWith(lcname, "vscp.")) return false;

    ///////////////////////////////////////////////////////////////////////////
    //                               DEBUG
    ///////////////////////////////////////////////////////////////////////////

    if (vscp_startsWith(lcname, "vscp.debug.flags1")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags1 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags2")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags2 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags3")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags3 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags4")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags4 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags5")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags5 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags6")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags6 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags7")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags7 = (uint32_t)value;
        return true; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.debug.flags8")) {
        long value;
        var.getValue(&value);
        gpobj->m_debugFlags8 = (uint32_t)value;
        return true; // None writable
    }

    ///////////////////////////////////////////////////////////////////////////
    //                              VERSIONS
    ///////////////////////////////////////////////////////////////////////////

    if (vscp_startsWith(lcname, "vscp.version.major")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.version.minor")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.version.build")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.version.str")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.copyright.vscp")) {
        return false; // None writable
    }

    // *************************************************************************
    //                                 LUA
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.lua.copyright")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.major")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.minor")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.release")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.lua.version.str")) {
        return false; // None writable
    }

    // *************************************************************************
    //                                SQLite
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.sqlite.version.major")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.minor")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.release")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.build")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.sqlite.version.str")) {
        return false; // None writable
    }

    // *************************************************************************
    //                                OPENSSL
    // *************************************************************************

#ifndef WIN32
    if (vscp_startsWith(lcname, "vscp.openssl.version.str")) {
        return false; // None writable
    }
#endif

    // *************************************************************************
    //                                 OS
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.os.str")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.wordwidth")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.wordwidth.str")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.width.is64bit")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.width.is32bit")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.endiness.str")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.os.endiness.islittleendian")) {
        return false; // None writable
    }

    // *************************************************************************
    //                                 HOST
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.host.rootpath")) {
        return false;
    }

    if (vscp_startsWith(lcname, "vscp.host.name")) {
        gpobj->m_strServerName = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_servername"),
          vscp_str_format("%s", gpobj->m_strServerName.c_str()));
    }

    if (vscp_startsWith(lcname, "vscp.host.ip")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.host.mac")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.host.userid")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.host.username")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.host.guid")) {
        gpobj->m_guid.getFromString(var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.loglevel")) {
        int val;
        var.getValue(&val);
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem(("vscpd_LogLevel"),
                                                    val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.loglevel.str")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.client.receivequeue.max")) {
        int val;
        var.getValue(&val);
        gpobj->m_maxItemsInClientReceiveQueue = val;
        return gpobj->updateConfigurationRecordItem(
          ("vscpd.maxqueue"), vscp_str_format("%d", val));
    }

    if (vscp_startsWith(lcname, "vscp.tcpip.address")) {
        std::string strval;
        strval                          = var.getValue();
        gpobj->m_strTcpInterfaceAddress = strval;
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_TcpipInterface_Address"), strval);
    }

    if (vscp_startsWith(lcname, "vscp.udp.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_udpSrvObj.m_bEnable = val;
        return gpobj->updateConfigurationRecordItem(
          "vscpd_UdpSimpleInterface_Enable", val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.udp.address")) {
        std::string strval;
        strval = var.getValue();
        gpobj->m_udpSrvObj.m_interface = strval;
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_UdpSimpleInterface_Address"), strval);
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.enable")) {
        bool val;
        var.getValue(&val);
        val ? gpobj->m_automation.enableHeartbeatEvent()
            : gpobj->m_automation.disableHeartbeatEvent();
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_HeartbeatEvent_Enable"), val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.period")) {
        int val;
        var.getValue(&val);
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem(
          "vscpd_Automation_HeartbeatEvent_Interval",
          vscp_str_format("%d", val));
    }

    if (vscp_startsWith(lcname, "vscp.automation.heartbeat.last")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl-heartbeat.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_logLevel = val;
        return gpobj->updateConfigurationRecordItem(("vscpd_LogLevel"),
                                                    val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl.heartbeat.period")) {
        long val;
        var.getValue(&val);
        gpobj->m_automation.setSegmentControllerHeartbeatInterval(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_SegmentControllerEvent_Interval"),
          vscp_str_format("%ld", val));
    }

    if (vscp_startsWith(lcname, "vscp.automation.segctrl.heartbeat.last")) {
        return false; // None writable
    }

    if (vscp_startsWith(lcname, "vscp.automation.longitude")) {
        double val;
        var.getValue(&val);
        gpobj->m_automation.setLongitude(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_Longitude"),
          vscp_str_format("%f", gpobj->m_automation.getLongitude()));
    }

    if (vscp_startsWith(lcname, "vscp.automation.latitude")) {
        double val;
        var.getValue(&val);
        gpobj->m_automation.setLatitude(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_Latitude"),
          vscp_str_format("%f", gpobj->m_automation.getLatitude()));
    }

    if (vscp_startsWith(lcname,
                        "vscp.automation.twilightsunriseevent.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_automation.enableSunRiseTwilightEvent(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_SunriseTwilight_Enable"), val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.automation.twilightsunsetevent.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_automation.enableSunSetTwilightEvent(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_SunsetTwilight_Enable"), val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunriseevent.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_automation.enableSunRiseEvent(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_Sunrise_Enable"), val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.automation.sunsetevent.enable")) {
        int val;
        var.getValue(&val);
        gpobj->m_automation.enableSunSetEvent(val);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Automation_Sunset_Enable"), val ? ("1") : ("0"));
    }

    if (vscp_startsWith(lcname, "vscp.workingfolder")) {
        return false; // Not writable
    }

    // *************************************************************************
    //                              WEB-Server
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.websrv.enable")) {
        var.getValue(&gpobj->m_web_bEnable);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ENABLE), gpobj->m_web_bEnable ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.document_root")) {
        gpobj->m_web_document_root = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.listening_ports")) {
        gpobj->m_web_listening_ports = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.index_files")) {
        gpobj->m_web_index_files = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_INDEX_FILES), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.authentication_domain")) {
        gpobj->m_web_authentication_domain = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_auth_domain_check")) {
        var.getValue(&gpobj->m_enable_auth_domain_check);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK),
          gpobj->m_enable_auth_domain_check ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_certificat")) {
        gpobj->m_web_ssl_certificate = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_certificat_chain")) {
        gpobj->m_web_ssl_certificate_chain = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_verify_peer")) {
        var.getValue(&gpobj->m_web_ssl_verify_peer);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER),
          gpobj->m_web_ssl_verify_peer ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_ca_path")) {
        gpobj->m_web_ssl_ca_path = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_ca_file")) {
        gpobj->m_web_ssl_ca_file = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_verify_depth")) {
        gpobj->m_web_ssl_verify_depth =
          atoi((const char *)var.getValue().c_str());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_default_verify_paths")) {
        var.getValue(&gpobj->m_web_ssl_default_verify_paths);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS),
          gpobj->m_web_ssl_default_verify_paths ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_cipher_list")) {
        gpobj->m_web_ssl_cipher_list = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_protocol_version")) {
        gpobj->m_web_ssl_protocol_version =
          atoi((const char *)var.getValue().c_str());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssl_short_trust")) {
        var.getValue(&gpobj->m_web_ssl_short_trust);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST),
          gpobj->m_web_ssl_short_trust ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_interpreter")) {
        gpobj->m_web_cgi_interpreter = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_patterns")) {
        gpobj->m_web_cgi_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.cgi_environment")) {
        gpobj->m_web_cgi_environment = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.protect_uri")) {
        gpobj->m_web_protect_uri = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_PROTECT_URI), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.trottle")) {
        gpobj->m_web_trottle = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_TROTTLE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_directory_listing")) {
        var.getValue(&gpobj->m_web_enable_directory_listing);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING),
          gpobj->m_web_enable_directory_listing ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.enable_keep_alive")) {
        var.getValue(&gpobj->m_web_enable_keep_alive);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE),
          gpobj->m_web_enable_keep_alive ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.keep_alive_timeout_ms")) {
        gpobj->m_web_keep_alive_timeout_ms =
          std::stol((const char *)var.getValue().c_str());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_list")) {
        gpobj->m_web_access_control_list = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.extra_mime_types")) {
        gpobj->m_web_extra_mime_types = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.num_threads")) {
        gpobj->m_web_num_threads = atoi((const char *)var.getValue().c_str());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_NUM_THREADS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.hide_file_patterns")) {
        gpobj->m_web_url_rewrite_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.run_as_user")) {
        gpobj->m_web_run_as_user = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.url_rewrite_patterns")) {
        gpobj->m_web_url_rewrite_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.request_timeout_ms")) {
        gpobj->m_web_request_timeout_ms = std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.linger_timeout_ms")) {
        gpobj->m_web_linger_timeout_ms = std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.decode_url")) {
        var.getValue(&gpobj->m_web_decode_url);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_DECODE_URL),
          gpobj->m_web_decode_url ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.global_authfile")) {
        gpobj->m_web_global_auth_file = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.per_directory_auth_file")) {
        gpobj->m_web_per_directory_auth_file = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.ssi_patterns")) {
        gpobj->m_web_ssi_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_origin")) {
        gpobj->m_web_access_control_allow_origin = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_methods")) {
        gpobj->m_web_access_control_allow_methods = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS),
          var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.access_control_allow_headers")) {
        gpobj->m_web_access_control_allow_headers = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS),
          var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.error_pages")) {
        gpobj->m_web_error_pages = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES), var.getValue());
    }

    if (vscp_startsWith(lcname, VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY)) {
        gpobj->m_web_tcp_nodelay = std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.static_file_max_age")) {
        gpobj->m_web_static_file_max_age = std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE), var.getValue());
    }

    if (vscp_startsWith(lcname,
                        "vscp.websrv.strict_transport_security_max_age")) {
        gpobj->m_web_strict_transport_security_max_age =
          std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE),
          var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.allow_sendfile_call")) {
        var.getValue(&gpobj->m_web_allow_sendfile_call);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL),
          gpobj->m_web_allow_sendfile_call ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.additional_headers")) {
        gpobj->m_web_additional_header = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.") +
        (VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE + 4)) {
        gpobj->m_web_max_request_size = std::stol(var.getValue());
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.allow_index_script_resource")) {
        var.getValue(&gpobj->m_web_allow_index_script_resource);
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE),
          gpobj->m_web_allow_index_script_resource ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websrv.duktape_script_patterns")) {
        gpobj->m_web_duktape_script_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_preload_file")) {
        gpobj->m_web_lua_preload_file = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_script_patterns")) {
        gpobj->m_web_lua_script_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_server_page_patterns")) {
        gpobj->m_web_lua_server_page_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_websockets_patterns")) {
        gpobj->m_web_lua_websocket_patterns = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_background_script")) {
        gpobj->m_web_lua_background_script = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT), var.getValue());
    }

    if (vscp_startsWith(lcname, "vscp.websrv.lua_background_script_params")) {
        gpobj->m_web_lua_background_script_params = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          (VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS),
          var.getValue());
    }

    // *************************************************************************
    //                            Websocket-Server
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.websockets.enable")) {
        var.getValue(&gpobj->m_bWebsocketsEnable);
        return gpobj->updateConfigurationRecordItem(
          ("websocket_enable"), gpobj->m_bWebsocketsEnable ? "1" : "0");
    }

    if (vscp_startsWith(lcname, "vscp.websockets.enable")) {
        gpobj->m_websocket_document_root = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          ("websocket_document_root"), gpobj->m_websocket_document_root);
    }

    if (vscp_startsWith(lcname, "vscp.websockets.enable")) {
        gpobj->m_websocket_timeout_ms =
          std::stol((const char *)var.getValue().c_str());
        return gpobj->updateConfigurationRecordItem(("websocket_timeout_ms"),
                                                    var.getValue());
    }

    // *************************************************************************
    //                            Decision Matrix (DM)
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.dm.xml.path")) {
        gpobj->m_dm.m_staticXMLPath = var.getValue();
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_DM_XML_Path"), gpobj->m_dm.m_staticXMLPath);
    }

    if (vscp_startsWith(lcname, "vscp.dm.allow.xml.save")) {
        var.getValue(&gpobj->m_dm.bAllowXMLsave);
        return gpobj->updateConfigurationRecordItem(("vscpd_DM_DB_Path"),
                                                    var.getAsString());
    }

    // *************************************************************************
    //                             Variables
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.variable.db.path")) {
        std::string strval;
        strval = var.getValue();
        var.setValue(gpobj->m_variables.m_dbFilename);
        return gpobj->updateConfigurationRecordItem(("vscpd_Variables_DB_Path"),
                                                    strval);
    }

    if (vscp_startsWith(lcname, "vscp.variable.xml.path")) {
        std::string strval;
        strval = var.getValue();
        var.setValue(gpobj->m_variables.m_xmlPath);
        return gpobj->updateConfigurationRecordItem(
          ("vscpd_Variables_XML_Path"), strval);
    }

    // *************************************************************************
    //                              Log files
    // *************************************************************************

    // *************************************************************************
    //                             Databases
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.database.vscpdata.path")) {
        std::string strval;
        strval                     = var.getValue();
        gpobj->m_path_db_vscp_data = strval;
        return gpobj->updateConfigurationRecordItem(("vscpd_db_data_path"),
                                                    strval);
    }

    if (vscp_startsWith(lcname, "vscp.database.vscpdconfig.path")) {
        std::string strval;
        strval                       = var.getValue();
        gpobj->m_path_db_vscp_daemon = strval;
        return gpobj->updateConfigurationRecordItem(("vscpd_db_vscpconf_path"),
                                                    strval);
    }

    // *************************************************************************
    //                             Decision Matrix
    // *************************************************************************
    if (vscp_startsWith(lcname, "vscp.dm.count")) {
        return false; // None writable
    }

    else if (vscp_startsWith(lcname, "vscp.dm.count.active")) {
        return false; // None writable
    }

    else if (vscp_startsWith(lcname, "vscp.dm.logging.enable")) {
        return false; // None writable
    }

    else if (vscp_startsWith(lcname, "vscp.dm.logging.path")) {
        return false; // None writable
    }

    else if (0 == vscp_strcasecmp(lcname.c_str(), "vscp.dm.")) {

        int pos;
        short row;
        dmElement dmDB;
        std::string str;
        str = vscp_str_right(str, lcname.length() - 8); // remove "vscp.dm."
        if (0 == vscp_strcasecmp(str.c_str(), ".")) {

            // this is a sub variable.
            std::string strID;
            unsigned long id;

            // Get id
            id = vscp_readStringValue(vscp_str_left(str, pos));

            // get the sub string
            str = vscp_str_left(str, str.length() - pos - 1);

            // Get the DM db record
            if (!gpobj->m_dm.getDatabaseRecord(id, &dmDB)) return false;

            // Get a pointer to memory DM row (if any)
            dmElement *pdm =
              gpobj->m_dm.getMemoryElementFromId(var.getID(), &row);

            if (vscp_startsWith(str, "id")) {
                return false; // None writable
            }

            else if (vscp_startsWith(str, "bEnable")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);

                // Update database record
                bVal ? strValue = "1" : "0";
                if (!gpobj->m_dm.updateDatabaseRecordItem(
                      id, "bEnable", strValue)) {
                    return false;
                }

                // if the record is in memory
                if (NULL != pdm) {

                    pdm->m_bEnable = bVal;

                    // if value is false the database row should be removed
                    if (!bVal) {
                        gpobj->m_dm.removeMemoryElement(row);
                    }

                }

                // If true and not in memory the record should be added
                else {
                    dmElement *pdmnew = new dmElement();
                    if (NULL != pdmnew) return false;
                    gpobj->m_dm.getDatabaseRecord(dmDB.m_id, pdmnew);
                    gpobj->m_dm.addMemoryElement(pdmnew);
                }

            } else if (vscp_startsWith(str, "groupid")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_strGroupID = strValue;
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("groupid"), strValue);

            } else if (vscp_startsWith(str, "mask.priority")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.mask_priority = val;
                }
                strValue = vscp_str_format("%d", val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("maskPriority"), strValue);
            } else if (vscp_startsWith(str, "mask.class")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.mask_class = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("maskClass"), strValue);
            } else if (vscp_startsWith(str, "mask.type")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.mask_type = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("maskType"), strValue);
            } else if (vscp_startsWith(str, "mask.guid")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    cguid guid;
                    guid.getFromString(strValue);
                    memcpy(pdm->m_vscpfilter.mask_GUID, guid.getGUID(), 16);
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("maskGUID"), strValue);
            } else if (vscp_startsWith(str, "filter.priority")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.filter_priority = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("filterPriority"), strValue);
            } else if (vscp_startsWith(str, "filter.class")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.filter_class = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("filterClass"), strValue);
            } else if (vscp_startsWith(str, "filter.type")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_vscpfilter.filter_type = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("filterType"), strValue);
            } else if (vscp_startsWith(str, "filter.guid")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    cguid guid;
                    guid.getFromString(strValue);
                    memcpy(pdm->m_vscpfilter.filter_GUID, guid.getGUID(), 16);
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("filterGUID"), strValue);
            } else if (vscp_startsWith(str, "allowed.start")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.setFromTime(strValue);
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedStart"), strValue);
            } else if (vscp_startsWith(str, "allowed.end")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.setEndTime(strValue);
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedEnd"), strValue);
            } else if (vscp_startsWith(str, "allowed.monday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowMonday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedMonday"), strValue);
            } else if (vscp_startsWith(str, "allowed.tuesday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowTuesday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedTuesday"), strValue);
            } else if (vscp_startsWith(str, "allowed.wednessday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowWednesday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedWednessday"), strValue);
            } else if (vscp_startsWith(str, "allowed.thursday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowThursday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedThursday"), strValue);
            } else if (vscp_startsWith(str, "allowed.friday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowFriday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedFriday"), strValue);
            } else if (vscp_startsWith(str, "allowed.saturday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowSaturday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedSaturday"), strValue);
            } else if (vscp_startsWith(str, "allowed.sunday")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.allowSunday(bVal);
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedSunday"), strValue);
            } else if (vscp_startsWith(str, "allowed.time")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_timeAllow.parseActionTime(strValue);
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("allowedTime"), strValue);
            } else if (vscp_startsWith(str, "bCheckIndex")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_bCheckIndex = bVal;
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("bCheckIndex"), strValue);
            } else if (vscp_startsWith(str, "index")) {

                int val;

                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_index = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("index"), strValue);
            } else if (vscp_startsWith(str, "bCheckZone")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("bCheckZone"), strValue);
            } else if (vscp_startsWith(str, "zone")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_zone = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("zone"), strValue);
            } else if (vscp_startsWith(str, "bCheckSubZone")) {

                bool bVal;
                std::string strValue;

                var.getValue(&bVal);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_bCheckZone = bVal;
                }
                bVal ? strValue = ("1") : ("0");
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("bCheckSubZone"), strValue);
            } else if (vscp_startsWith(str, "subzone")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_subzone = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("subzone"), strValue);
            } else if (vscp_startsWith(str, "measurement.value")) {

                double val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_measurementValue = val;
                }
                strValue = vscp_str_format(("%d"), val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, ("measurementValue"), strValue);
            } else if (vscp_startsWith(str, "measurement.unit")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_measurementUnit = val;
                }
                strValue = vscp_str_format("%d", val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, "measurementUnit", strValue);
            } else if (vscp_startsWith(str, "measurement.compare")) {

                int val;
                std::string strValue;

                var.getValue(&val);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_measurementUnit = val;
                }
                strValue = vscp_str_format("%d", val);
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, "measurementCompare", strValue);
            } else if (vscp_startsWith(str, "measurement.compare.string")) {
                return false;
            } else if (vscp_startsWith(str, "comment")) {

                std::string strValue;

                var.getValue(&strValue);
                // Change in memory record
                if (NULL != pdm) {
                    pdm->m_comment = strValue;
                }
                return gpobj->m_dm.updateDatabaseRecordItem(
                  id, "comment", strValue);
            } else if (vscp_startsWith(str, "count.trigger")) {
                if (NULL != pdm) {
                    pdm->m_triggCounter = 0;
                }
            } else if (vscp_startsWith(str, "count.error")) {
                if (NULL != pdm) {
                    pdm->m_errorCounter = 0;
                }
            } else if ((vscp_startsWith(str, "error")) ||
                       (vscp_startsWith(str, "error.string"))) {
                return false; // None writable
            }

        } else {
            // A full DM row is written ("vscp.dm.35" for row with id = 35)
            // a new row have id set to zero
        }
    }

    // *************************************************************************
    //                              Drivers
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.driver.")) {
    }

    // *************************************************************************
    //                             Interfaces
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.interface.")) {
        ;
    }

    // *************************************************************************
    //                              Discovery
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.discovery.")) {
        ;
    }

    // *************************************************************************
    //                                 Log
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.log.")) {
        ;
    }

    // *************************************************************************
    //                                Tables
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.table.")) {
        ;
    }

    // *************************************************************************
    //                                Users
    // *************************************************************************

    if (vscp_startsWith(lcname, "vscp.user.")) {

        std::deque<std::string> tokens;
        vscp_split(tokens, lcname, ".");

        std::string strToken;
        strToken = tokens.front();
        tokens.pop_front(); // vscp
        strToken = tokens.front();
        tokens.pop_front(); // user

        if (tokens.empty()) return false;
        strToken = tokens.front();
        tokens.pop_front(); // vscp

        vscp_trim(strToken);

        // Add new user record
        if (vscp_startsWith(strToken, "add")) {

            // String is BASE64 encoded so it must be decoded before it
            // can be used
            str = var.getValue();

            return gpobj->m_userList.addUser(str);
        }
        // Delete a user record (value = userid)
        if (vscp_startsWith(strToken, "delete")) {

            // String is BASE64 encoded so it must be decoded before it
            // can be used
            str = var.getValue();

            return gpobj->m_userList.deleteUser(str);
        } else {

            // Token should be a user ordinal number

            unsigned long idx = vscp_readStringValue(strToken);
            CUserItem *pUserItem =
              gpobj->m_userList.getUserItemFromOrdinal(idx);
            if (NULL == pUserItem) return false;

            if (!tokens.empty()) {

                // write
                str = var.getValue();

                if (pUserItem->setFromString(str)) {
                    pUserItem->saveToDatabase();
                    return true;
                }
            }

            strToken = tokens.front();
            tokens.pop_front();

            if (vscp_startsWith(strToken, "write")) {
                pUserItem->setFromString(var.getValue());
            } else if (vscp_startsWith(strToken, "name")) {
                pUserItem->setUserName(var.getValue());
            } else if (vscp_startsWith(strToken, "password")) {
                pUserItem->setPassword(var.getValue());
            } else if (vscp_startsWith(strToken, "fullname")) {
                pUserItem->setFullname(var.getValue());
            } else if (vscp_startsWith(strToken, "filter")) {
                pUserItem->setFilterFromString(var.getValue());
            } else if (vscp_startsWith(strToken, "rights")) {
                if (tokens.empty()) {
                    pUserItem->setUserRightsFromString(var.getValue());
                } else {
                    unsigned long nRight, value;
                    strToken = tokens.front();
                    tokens.pop_front(); // id
                    nRight = stol(strToken);
                    if (nRight > 7) return false;
                    value = stoul(var.getValue());
                    pUserItem->setUserRights(nRight, value);
                }
            } else if (vscp_startsWith(strToken, "allowed")) {
                if (tokens.empty()) return false;
                strToken = tokens.front();
                tokens.pop_front();
                if (("events") == strToken) {
                    pUserItem->setAllowedEventsFromString(var.getValue());
                } else if (vscp_startsWith(strToken, "remotes")) {
                    pUserItem->setAllowedRemotesFromString(var.getValue());
                } else {
                    return false;
                }
            } else if (vscp_startsWith(strToken, "note")) {
                pUserItem->setNote(var.getValue());
            } else {
                return false;
            }
        }
    }

    // ----------------------------- Not Found
    // -------------------------------------

    return false; // Not a stock variable
}

///////////////////////////////////////////////////////////////////////////////
// setVariableFromDbRecord
//

bool
CVariableStorage::setVariableFromDbRecord(sqlite3_stmt *ppStmt,
                                          CVSCPVariable &variable)
{
    // Check pointer
    if (NULL == ppStmt) return false;

    // ID
    variable.setID(sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_ID));

    // Last changed
    variable.m_lastChanged.getISODateTime((const char *)sqlite3_column_text(
      ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE));

    // Name
    variable.setName(std::string(
      (const char *)sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME)));

    // Variable type
    variable.setType(sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE));

    // Value
    variable.setValue(std::string((const char *)sqlite3_column_text(
      ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE)));

    // Persistence
    variable.setPersistent(
      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT) ? true
                                                                     : false);

    // Owner
    variable.setOwnerID(
      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER));

    // Access rights
    variable.setAccessRights(
      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION));

    // Note
    variable.setNote(std::string(
      (const char *)sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE)));

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// findNonPersistentVariable
//

uint32_t
CVariableStorage::findNonPersistentVariable(const std::string &name,
                                            CVSCPVariable &variable)
{
    sqlite3_stmt *ppStmt;
    char psql[8192];

    variable.setID(0); // Invalid id

    sprintf(psql,
            VSCPDB_VARIABLE_FIND_FROM_NAME,
            (const char *)vscp_upper(name).c_str());

    if (NULL == m_db_vscp_internal_variable) {
        return 0;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(
                       m_db_vscp_internal_variable, psql, -1, &ppStmt, NULL)) {
        return 0;
    }

    // Get the data for the variable
    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        setVariableFromDbRecord(ppStmt, variable);
        /*
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID )
        ); variable.m_lastChanged.getISODateTime( (const char
        *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) ); variable.setType(
        sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) ); variable.setPersistent(
        false ); variable.setUserID( sqlite3_column_int( ppStmt,
        VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); variable.setAccessRights(
        sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
        */
    }

    sqlite3_finalize(ppStmt);

    return variable.getID();
}

///////////////////////////////////////////////////////////////////////////////
// findPersistentVariable
//

uint32_t
CVariableStorage::findPersistentVariable(const std::string &name,
                                         CVSCPVariable &variable)
{
    sqlite3_stmt *ppStmt;
    char psql[512];

    variable.setID(0); // Invalid id

    sprintf(psql, VSCPDB_VARIABLE_FIND_FROM_NAME, vscp_upper(name).c_str());

    if (NULL == m_db_vscp_external_variable) {
        return 0;
    }

    if (SQLITE_OK != sqlite3_prepare_v2(
                       m_db_vscp_external_variable, psql, -1, &ppStmt, NULL)) {
        return 0;
    }

    // Get the result
    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        setVariableFromDbRecord(ppStmt, variable);
        /*
        variable.setID( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_ID )
        ); variable.m_lastChanged.getISODateTime( (const char
        *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE ) );
        variable.setName( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME ) ) ); variable.setType(
        sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE ) );
        variable.setValue( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE ) ) ); variable.setPersistent(
        sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERSISTENT ) ? true
        : false ); variable.setUserID( sqlite3_column_int( ppStmt,
        VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER ) ); variable.setAccessRights(
        sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION ) );
        variable.setNote( std::string( (const char *)sqlite3_column_text(
        ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE ) ) );
        */
    }

    sqlite3_finalize(ppStmt);

    return variable.getID();
}

///////////////////////////////////////////////////////////////////////////////
// getVarlistFromRegExp
//

bool
CVariableStorage::getVarlistFromRegExp(std::deque<std::string> &nameArray,
                                       const std::string &regex,
                                       const int type,
                                       bool bClear)
{
    std::string varname;
    sqlite3_stmt *ppStmt;
    std::string regexlocal;

    // Clear array if set
    if (bClear) nameArray.clear();

    vscp_trim(regexlocal);
    if (0 == regexlocal.length()) regexlocal = ("(.*)"); // List all if empty

    try {

        if (SQLITE_OK != sqlite3_prepare_v2(m_db_vscp_internal_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL)) {
            return false;
        }

        while (SQLITE_ROW == sqlite3_step(ppStmt)) {

            const unsigned char *p =
              sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME);
            varname = std::string((const char *)p);
            if (std::regex_match(varname, std::regex(regexlocal))) {
                nameArray.push_back(varname);
            }
        }

        sqlite3_finalize(ppStmt);

        if (SQLITE_OK != sqlite3_prepare_v2(m_db_vscp_external_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL)) {
            sqlite3_finalize(ppStmt);
            return false;
        }

        while (SQLITE_ROW == sqlite3_step(ppStmt)) {

            varname = std::string((const char *)sqlite3_column_text(
              ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME));
            if (std::regex_match(varname, std::regex(regexlocal))) {
                nameArray.push_back(varname);
            }
        }

        sqlite3_finalize(ppStmt);

        // Sort the array
        // nameArray.sort();  TODO sort in db instead

    } catch (...) {
        syslog(LOG_ERR,
               "[getVarlistFromRegExp] Invalid "
               "regular expression [%s]",
               (const char *)regexlocal.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// listEnumerationFromRegExp
//

bool
CVariableStorage::listEnumerationFromRegExp(varQuery *pq,
                                            const std::string &regex,
                                            const int type)
{
    std::string varname;
    std::string regexlocal;
    vscp_makeUpper(regexlocal);

    // Must be a valid pointer
    if (NULL == pq) return false;

    vscp_trim(regexlocal);
    if (0 == regexlocal.length()) regexlocal = ("(.*)"); // List all if empty
    /* TODO xxRegEx xxregex( regexlocal );
    if ( !xxregex.isValid() ) {

        syslog( LOG_ERR,  vscp_str_format("[getVarlistFromRegExp] Invalid "
                                           "regular expression [%s]"),
                            (const char *)regexlocal.c_str() ) );
        return false;

    }*/

    if ((VARIABLE_INTERNAL & pq->tableType) ||
        (VARIABLE_STOCK & pq->tableType)) {

        if (SQLITE_OK != sqlite3_prepare_v2(m_db_vscp_internal_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &pq->ppStmt,
                                            NULL)) {
            return false;
        }

    } else if (VARIABLE_EXTERNAL & pq->tableType) {

        if (SQLITE_OK != sqlite3_prepare_v2(m_db_vscp_external_variable,
                                            VSCPDB_VARIABLE_FIND_ALL,
                                            -1,
                                            &pq->ppStmt,
                                            NULL)) {
            return false;
        }

    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// listItem
//

bool
CVariableStorage::listItem(varQuery *pq, CVSCPVariable &variable)
{
    // Must be a valid pointer
    if (NULL == pq) return false;

try_again:

    if ((VARIABLE_INTERNAL & pq->tableType) ||
        (VARIABLE_STOCK & pq->tableType)) {

        if (SQLITE_ROW != sqlite3_step(pq->ppStmt)) return false;

        setVariableFromDbRecord(pq->ppStmt, variable);

        // If stock variable is requested and it is not try again
        if ((VARIABLE_STOCK == pq->tableType) &&
            (!variable.isStockVariable())) {
            goto try_again;
        }

    } else if (VARIABLE_EXTERNAL & pq->tableType) {

        if (SQLITE_ROW != sqlite3_step(pq->ppStmt)) return false;

        setVariableFromDbRecord(pq->ppStmt, variable);

    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// listFinalize
//

void
CVariableStorage::listFinalize(varQuery *pq)
{
    // Must be a valid pointer
    if (NULL == pq) return;

    if (VARIABLE_STOCK == pq->tableType) {
        // Nothing to do
    } else if (VARIABLE_INTERNAL == pq->tableType) {
        if (NULL != pq->ppStmt) sqlite3_finalize(pq->ppStmt);
    } else if (VARIABLE_EXTERNAL == pq->tableType) {
        if (NULL != pq->ppStmt) sqlite3_finalize(pq->ppStmt);
    }

    delete pq;
}

///////////////////////////////////////////////////////////////////////////////
// addStockVariable
//

bool
CVariableStorage::addStockVariable(CVSCPVariable &var)
{
    uint32_t id   = 0;
    char *zErrMsg = 0;

    var.setStockVariable(); // Make sure it's marked as a stock variable

    char *sql = sqlite3_mprintf(
      VSCPDB_VARIABLE_INSERT,
      (const char *)var.m_lastChanged.Now().getISODateTime().c_str(),
      (const char *)var.getName().c_str(),
      var.getType(),
      (const char *)var.getValue().c_str(),
      0, // not persistent
      0, // admin
      var.getAccessRights(),
      (const char *)var.getNote().c_str());

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_internal_variable, sql, NULL, NULL, &zErrMsg)) {
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// add
//

bool
CVariableStorage::add(CVSCPVariable &var)
{
    uint32_t id   = 0;
    char *zErrMsg = 0;

    // Must have a valid name
    if (var.getName().empty()) {
        syslog(LOG_ERR,
               "[Add variable] Variable "
               "name can't be empty.");
        return false;
    }

    // Update last changed timestamp
    var.setLastChangedToNow();

    // Stock variables
    if (var.isStockVariable()) {
        return addStockVariable(var);
    }

    if ((id = exist(var.getName()))) {

        char *sql = sqlite3_mprintf(
          VSCPDB_VARIABLE_UPDATE,
          (const char *)var.m_lastChanged.getISODateTime().c_str(),
          (const char *)var.getName().c_str(),
          var.getType(),
          (const char *)var.getValue().c_str(),
          var.isPersistent() ? 1 : 0,
          var.getOwnerID(),
          var.getAccessRights(),
          (const char *)var.getNote().c_str(),
          id);

        if (SQLITE_OK != sqlite3_exec(var.isPersistent()
                                        ? m_db_vscp_external_variable
                                        : m_db_vscp_internal_variable,
                                      sql,
                                      NULL,
                                      NULL,
                                      &zErrMsg)) {
            syslog(LOG_ERR,
                   "[Add variable] Unable to update "
                   "variable in db. [%s] Err=%s",
                   sql,
                   zErrMsg);
            sqlite3_free(sql);
            return false;
        }

        sqlite3_free(sql);

    } else {
        char *sql = sqlite3_mprintf(
          VSCPDB_VARIABLE_INSERT,
          (const char *)var.m_lastChanged.Now().getISODateTime().c_str(),
          (const char *)var.getName().c_str(),
          var.getType(),
          (const char *)var.getValue().c_str(),
          var.isPersistent() ? 1 : 0,
          var.getOwnerID(),
          var.getAccessRights(),
          (const char *)var.getNote().c_str());

        if (SQLITE_OK !=
            sqlite3_exec((var.isPersistent() ? m_db_vscp_external_variable
                                             : m_db_vscp_internal_variable),
                         sql,
                         NULL,
                         NULL,
                         &zErrMsg)) {
            syslog(LOG_ERR,
                   "[Add variable] "
                   "Unable to add variable in db. [%s] Err=%s",
                   sql,
                   zErrMsg);
            sqlite3_free(sql);
            return false;
        }

        sqlite3_free(sql);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// add
//

bool
CVariableStorage::add(const std::string &name,
                      const std::string &value,
                      const uint16_t type,
                      const uint32_t userid,
                      const bool bPersistent,
                      const uint32_t accessRights,
                      const std::string &note)
{
    CVSCPVariable variable;

    variable.setName(name);
    variable.setType(type);
    variable.setValueFromString(type, value);
    variable.setPersistent(bPersistent);
    variable.setOwnerID(userid);
    variable.setAccessRights(accessRights);
    variable.setNote(note);

    return add(variable);
}

///////////////////////////////////////////////////////////////////////////////
// addWithStringType
//

bool
CVariableStorage::add(const std::string &varName,
                      const std::string &value,
                      const std::string &strType,
                      const uint32_t userid,
                      bool bPersistent,
                      const uint32_t accessRights,
                      const std::string &note)
{
    uint8_t type = CVSCPVariable::getVariableTypeFromString(strType);
    return add(varName, value, type, userid, bPersistent, accessRights, note);
}

///////////////////////////////////////////////////////////////////////////////
// update
//

bool
CVariableStorage::update(CVSCPVariable &var)
{
    long id       = 0;
    char *zErrMsg = 0;

    // Must exists
    if (0 == (id = exist(var.getName()))) {
        return false;
    }

    char *sql =
      sqlite3_mprintf(VSCPDB_VARIABLE_UPDATE,
                      (const char *)var.m_lastChanged.getISODateTime().c_str(),
                      (const char *)var.getName().c_str(),
                      var.getType(),
                      (const char *)var.getValue().c_str(),
                      var.isPersistent() ? 1 : 0,
                      var.getOwnerID(),
                      var.getAccessRights(),
                      (const char *)var.getNote().c_str(),
                      id);

    if (SQLITE_OK !=
        sqlite3_exec((var.isPersistent() ? m_db_vscp_external_variable
                                         : m_db_vscp_internal_variable),
                     sql,
                     NULL,
                     NULL,
                     &zErrMsg)) {
        syslog(LOG_ERR,
               "[Add variable] Unable to update "
               "variable in db. [%s] Err=%s",
               sql,
               zErrMsg);
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    // If stock variables update locally
    if (var.isStockVariable()) {
        putStockVariable(var);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool
CVariableStorage::remove(std::string &name)
{
    char *zErrMsg = 0;
    CVSCPVariable variable;

    if (!find(name, variable)) {
        return false;
    }

    // If stock variable it can't be removed
    if (variable.isStockVariable()) {
        return false;
    }

    if (variable.isPersistent()) {
        char *sql = sqlite3_mprintf(VSCPDB_VARIABLE_WITH_ID, variable.getID());
        if (SQLITE_OK !=
            sqlite3_exec(
              m_db_vscp_external_variable, sql, NULL, NULL, &zErrMsg)) {
            sqlite3_free(sql);
            syslog(LOG_ERR,
                   "[Delete variable] Unable to delete "
                   "variable in db. [%s] Err=%s",
                   sql,
                   zErrMsg);
            return false;
        }

        sqlite3_free(sql);
    } else {
        char *sql = sqlite3_mprintf(VSCPDB_VARIABLE_WITH_ID, variable.getID());
        if (SQLITE_OK !=
            sqlite3_exec(
              m_db_vscp_internal_variable, sql, NULL, NULL, &zErrMsg)) {
            sqlite3_free(sql);
            syslog(LOG_ERR,
                   "[Delete variable] Unable to delete "
                   "variable in db. [%s] Err=%s",
                   sql,
                   zErrMsg);
            return false;
        }

        sqlite3_free(sql);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool
CVariableStorage::remove(CVSCPVariable &variable)
{
    std::string str = variable.getName();
    return remove(str);
}

///////////////////////////////////////////////////////////////////////////////
// doCreateExternalVariableTable
//
// Create the external variable database
//
//

bool
CVariableStorage::doCreateExternalVariableTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_VARIABLE_CREATE;

    // Check if database is open
    if (NULL == m_db_vscp_external_variable) return false;

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_external_variable, psql, NULL, NULL, NULL)) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateInternalVariableTable
//
// Create the internal variable database
//
//

bool
CVariableStorage::doCreateInternalVariableTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_VARIABLE_CREATE;

    // Check if database is open
    if (NULL == m_db_vscp_internal_variable) return false;

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_internal_variable, psql, NULL, NULL, NULL)) {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------

static int depth_load_var_parser   = 0;

static void
startLoadVarParser(void *data, const char *name, const char **attr)
{
    CVariableStorage *pStorage = (CVariableStorage *)data;
    if (NULL == data) return;

    if (0 == depth_load_var_parser) {

        if ((0 == vscp_strcasecmp(name, "variables"))) {
            // We just increase the depth
        }
    } else if (1 == depth_load_var_parser) {

        if (0 == vscp_strcasecmp(name, "variable")) {

            CVSCPVariable var;

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == vscp_strcasecmp(attr[i], "type")) {
                    var.setType(attribute);
                } else if (0 == vscp_strcasecmp(attr[i], "name")) {
                    var.setName(attribute);
                } else if (0 == vscp_strcasecmp(attr[i], "persistent")) {
                    vscp_trim(attribute);
                    if (0 == strcasecmp(attribute.c_str(), "true")) {
                        var.setPersistent(true);
                    } else {
                        var.setPersistent(false);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "user")) {
                    uint32_t id = stoul(attribute);
                    var.setOwnerId(id);
                } else if (0 == vscp_strcasecmp(attr[i], "access-rights")) {
                    uint32_t ar = vscp_readStringValue(attribute);
                    var.setAccessRights(ar);
                } else if (0 == vscp_strcasecmp(attr[i], "value")) {
                    var.setValue(attribute);
                } else if (0 == vscp_strcasecmp(attr[i], "note")) {
                    var.setNote(attribute);
                }
            }

            if (pStorage->exist(var.getName())) {
                if (pStorage->update(var)) {
                    if (gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE) {
                        syslog(LOG_DEBUG,
                               "[Loading XML file] Update variable %s.",
                               (const char *)var.getName().c_str());
                    }
                } else {
                    syslog(LOG_ERR,
                           "[Loading XML file] Failed to update variable %s.\n",
                           (const char *)var.getName().c_str());
                }

            } else {
                if (pStorage->add(var)) {
                    if (gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE) {
                        syslog(LOG_DEBUG,
                               "[Loading XML file] Added variable %s.",
                               (const char *)var.getName().c_str());
                    }
                } else {
                    syslog(LOG_ERR,
                           "[Loading XML file] Failed to add variable %s.",
                           (const char *)var.getName().c_str());
                }
            }
        }
    }

    depth_load_var_parser++;
}



static void
endLoadVarParser(void *data, const char *name)
{
    CVariableStorage *pStorage = (CVariableStorage *)data;
    if (NULL == data) return;

    depth_load_var_parser--;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// loadFromXML
//
// Read persistent variables from file
//

bool
CVariableStorage::loadFromXML(const std::string &path)
{
    std::string str;
    std::string xmlpath;
    unsigned long lval;

    // If a null path is supplied use the configured path
    if (path.empty()) {
        xmlpath = m_xmlPath;
        if (gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE) {
            syslog(LOG_ERR,
                   "[loadFromXML] Loading variable XML file from %s.",
                   m_xmlPath.c_str());
        }

    } else {
        xmlpath = path;
        if (gpobj->m_debugFlags1 & VSCP_DEBUG1_VARIABLE) {
            syslog(LOG_ERR,
                   "[loadFromXML] Loading variable XML file from %s.",
                   (const char *)path.c_str());
        }
    }

    FILE *fp;
    fp = fopen(xmlpath.c_str(), "r");
    if (NULL == fp) {
        syslog(
          LOG_CRIT, "Failed to open variable file [%s]", m_xmlPath.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser, startLoadVarParser, endLoadVarParser);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_CRIT,
               "Failed to allocate buffer for XML parser file [%s]",
               m_xmlPath.c_str());
        return false;
    }

    size_t file_size = 0;
    file_size        = fread(buf, sizeof(char), XML_BUFF_SIZE, fp);

    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0)) {
        syslog(LOG_ERR, "Failed parse XML configuration file.");
        fclose(fp);
        XML_ParserFree(xmlParser);
        return false;
    }

    fclose(fp);
    XML_ParserFree(xmlParser);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// save
//
//

bool
CVariableStorage::save()
{

    // Make a copy before we save
    // TODOxxCopyFile( m_xmlPath, m_xmlPath + ("~") );

    return save(m_xmlPath);
}

///////////////////////////////////////////////////////////////////////////////
// save
//
//

bool
CVariableStorage::save(std::string &path, uint8_t whatToSave)
{
    std::string str;

    try {

        std::ofstream of(path);

        if (!of.is_open()) {
            syslog(LOG_ERR,
                   "Failed to open file to "
                   "write variables to. path=%s",
                   path.c_str());
        } else {

            of << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n";

            // VSCP variables
            of << "<variables>\n\n";

            if ((NULL == m_db_vscp_internal_variable) &&
                (whatToSave & VARIABLE_INTERNAL)) {

                char *pErrMsg = 0;
                sqlite3_stmt *ppStmt;
                const char *psql = "SELECT * from variable";

                if (SQLITE_OK !=
                    sqlite3_prepare_v2(
                      m_db_vscp_internal_variable, psql, -1, &ppStmt, NULL)) {
                    return false;
                }

                while (SQLITE_ROW == sqlite3_step(ppStmt)) {

                    CVSCPVariable variable;

                    // id
                    variable.setID(
                      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_ID));
                    // Last change date
                    variable.m_lastChanged.getISODateTime(
                      (const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE));
                    // name
                    variable.setName(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME)));
                    // type
                    variable.setType(
                      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE));
                    // value
                    variable.setValue(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE)));
                    // peristence
                    variable.setPersistent(false);
                    // owner id
                    variable.setOwnerID(sqlite3_column_int(
                      ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER));
                    // access rights
                    variable.setAccessRights(sqlite3_column_int(
                      ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION));
                    // note
                    variable.setNote(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE)));

                    // Write out one variable
                    writeVariableToXmlFile(of, variable);
                }

                sqlite3_finalize(ppStmt);
            }

            if ((NULL == m_db_vscp_external_variable) &&
                (whatToSave & VARIABLE_EXTERNAL)) {

                char *pErrMsg = 0;
                sqlite3_stmt *ppStmt;
                const char *psql = "SELECT * from variable";

                if (SQLITE_OK !=
                    sqlite3_prepare_v2(
                      m_db_vscp_external_variable, psql, -1, &ppStmt, NULL)) {
                    return false;
                }

                while (SQLITE_ROW == sqlite3_step(ppStmt)) {

                    CVSCPVariable variable;

                    // id
                    variable.setID(
                      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_ID));
                    // last change
                    variable.m_lastChanged.getISODateTime(
                      (const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_LASTCHANGE));
                    // name
                    variable.setName(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_NAME)));
                    // type
                    variable.setType(
                      sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_VARIABLE_TYPE));
                    // value
                    variable.setValue(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_VALUE)));
                    // persistence
                    variable.setPersistent(
                      sqlite3_column_int(ppStmt,
                                         VSCPDB_ORDINAL_VARIABLE_PERSISTENT)
                        ? true
                        : false);
                    // owner id
                    variable.setOwnerID(sqlite3_column_int(
                      ppStmt, VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER));
                    // access rights
                    variable.setAccessRights(sqlite3_column_int(
                      ppStmt, VSCPDB_ORDINAL_VARIABLE_PERMISSION));
                    // note
                    variable.setNote(
                      std::string((const char *)sqlite3_column_text(
                        ppStmt, VSCPDB_ORDINAL_VARIABLE_NOTE)));

                    // Write out one variable
                    writeVariableToXmlFile(
                      of,
                      variable); // Write the variable out on the stream
                }

                sqlite3_finalize(ppStmt);
            }

            // DM matrix information end
            of << "</variables>\n";

            // Close the file
            of.close();

            // Variable saved
            m_lastSaveTime = vscpdatetime::Now();
        }
    } catch (...) {
        syslog(LOG_ERR,
               "Error when writing out variables to file. path=%s",
               path.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeVariableToXmlFile
//
// Create the external variable database
//
//

bool
CVariableStorage::writeVariableToXmlFile(std::ofstream &of,
                                         CVSCPVariable &variable)
{
    std::string str, strtemp;
    std::string name = variable.getName();

    try {
        of << vscp_str_format("  <variable type=\"%d\" ",
                                 variable.getType());
        of << vscp_str_format(" name=\"%s\" ", (const char *)name.c_str());
        if (variable.isPersistent()) {
            of << " persistent=\"true\" ";
        } else {
            of << " persistent=\"false\" ";
        }
        of << vscp_str_format(" user=\"%d\" ", variable.getOwnerID());
        of << vscp_str_format(" access-rights=\"%d\" ",
                                 variable.getAccessRights());
        variable.writeValueToString(strtemp);
        of << vscp_str_format(" value=\"%s\" ",
                                 (const char *)strtemp.c_str());
        // Always save on base64 encoded form
        of << vscp_str_format(" note=\"%s\" ",
                                 (const char *)variable.getNote().c_str());
        of << " />\n\n";
    } catch (...) {
        syslog(LOG_ERR,
               "Failed to write out variable name=%s",
               variable.getName().c_str());
    }

    return true;
}
