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
#include "stockvariable.h"

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

CVariable::CVariable(void)
{
    init();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVariable::~CVariable(void)
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
CVariable::init(void)
{
    m_id = 0;
    m_name.clear();
    m_type         = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    m_bPersistent  = false;                // Not persistent by default
    m_accessRights = PERMISSION_OWNER_ALL; // Owner can do everything.
    m_userid       = USER_ID_ADMIN;        // Admin owns variable by default
    m_lastChanged  = vscpdatetime::Now();
    m_bStock       = false; // This is not a stock variable
}

///////////////////////////////////////////////////////////////////////////////
// fixName
//

void
CVariable::fixName(void)
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
CVariable::makeAccessRightString(uint32_t accessrights,
                                 std::string &strAccessRights)
{
    strAccessRights.clear();

    // other
    strAccessRights = (accessrights & PERMISSION_OTHER_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_OTHER_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_OTHER_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;

    // group
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = (accessrights & PERMISSION_GROUP_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_GROUP_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_GROUP_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;

    // owner
    strAccessRights = ' ' + strAccessRights;
    strAccessRights = (accessrights & PERMISSION_OWNER_EXECUTE)
                        ? 'x' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_OWNER_WRITE)
                        ? 'w' + strAccessRights
                        : '-' + strAccessRights;

    strAccessRights = (accessrights & PERMISSION_OWNER_READ)
                        ? 'r' + strAccessRights
                        : '-' + strAccessRights;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableType
//

uint16_t
CVariable::getVariableTypeFromString(const std::string &strVariableType)
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
            type = VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventguid")) {
            type = VSCP_DAEMON_VARIABLE_CODE_GUID;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventdata")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventclass")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventtype")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "eventtimestamp")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP;
        } else if (0 == vscp_strcasecmp(str.c_str(), "event")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT;
        } else if (0 == vscp_strcasecmp(str.c_str(), "guid")) {
            type = VSCP_DAEMON_VARIABLE_CODE_GUID;
        } else if (0 == vscp_strcasecmp(str.c_str(), "data")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA;
        } else if (0 == vscp_strcasecmp(str.c_str(), "vscpclass")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS;
        } else if (0 == vscp_strcasecmp(str.c_str(), "vscptype")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE;
        } else if (0 == vscp_strcasecmp(str.c_str(), "timestamp")) {
            type = VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP;
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
CVariable::getVariableTypeAsString(int type)
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

        case VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT:
            return "Measurement";

        case VSCP_DAEMON_VARIABLE_CODE_EVENT:
            return "VscpEvent";

        case VSCP_DAEMON_VARIABLE_CODE_GUID:
            return "VscpGuid";

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA:
            return "VscpData";

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS:
            return "VscpClass";

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE:
            return "VscpType";

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP:
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
CVariable::isNumerical(void)
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
CVariable::setType(const std::string &strType)
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
// setType
//

bool
CVariable::setType(uint16_t type)
{
    switch (type) {

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
        case VSCP_DAEMON_VARIABLE_CODE_STRING:
        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
        case VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT:
        case VSCP_DAEMON_VARIABLE_CODE_EVENT:
        case VSCP_DAEMON_VARIABLE_CODE_GUID:
        case VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA:
        case VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS:
        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE:
        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP:
        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
        case VSCP_DAEMON_VARIABLE_CODE_BLOB:
        case VSCP_DAEMON_VARIABLE_CODE_DATE:
        case VSCP_DAEMON_VARIABLE_CODE_TIME:
        case VSCP_DAEMON_VARIABLE_CODE_MIME:
        case VSCP_DAEMON_VARIABLE_CODE_HTML:
        case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
        case VSCP_DAEMON_VARIABLE_CODE_JSON:
        case VSCP_DAEMON_VARIABLE_CODE_XML:
        case VSCP_DAEMON_VARIABLE_CODE_SQL:
        case VSCP_DAEMON_VARIABLE_CODE_LUA:
        case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
        case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
        case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
        case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
        case VSCP_DAEMON_VARIABLE_CODE_USER:
        case VSCP_DAEMON_VARIABLE_CODE_FILTER:
            m_type = type;
            break;

        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//

std::string
CVariable::getAsString(bool bShort)
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
CVariable::getAsJSON(std::string &strVariable)
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
CVariable::setFromJSON(std::string &strVariable)
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
CVariable::getAsXML(std::string &strVariable)
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
CVariable::setFromXML(std::string &strVariable)
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
CVariable::setName(const std::string &strName)
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
CVariable::setOwnerId(uint32_t userid)
{
    // Check for admin user
    if (0 == userid) {
        m_userid = USER_ID_ADMIN;
        return true;
    }

    // Check if id is defined
    if (!CUserItem::isUserInDB(userid)) return false;

    // Assign userid
    m_userid = userid;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isUserAllowRead
//

bool CVariable::isUserAllowRead(CUserItem *pUser)
{
    if ( NULL == pUser ) {
        if ( isUserReadable() ) return true;
    }
    else {
        // Admin user can read all variables
        if ( 0 == pUser->getUserID() ) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// isUserAllowWrite
//

bool CVariable::isUserAllowWrite(CUserItem *pUser)
{
    if ( NULL == pUser ) {
        if ( isUserWritable() ) return true;
    }
    else {
        // Check if owner can write
        if ( ( m_userid == pUser->getUserID() ) && isOwnerWritable() ) {
            return true;
        }
        else if ( isUserWritable() ) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// isUserAllowExecute
//

bool CVariable::isUserAllowExecute(CUserItem *pUser)
{
    if ( NULL == pUser ) {
        if ( isUserExecutable() ) return true;
    }
    else {
        // Check if owner can execute
        if ( ( m_userid == pUser->getUserID() ) && isOwnerExecutable() ) {
            return true;
        }
        else if ( isUserExecutable() ) {
            return true;
        }
    }

    return false;
}



///////////////////////////////////////////////////////////////////////////////
// setOwnerIdFromUserName
//

bool
CVariable::setOwnerIdFromUserName(std::string &strUser)
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
CVariable::writeValueToString(std::string &strValueOut, bool bBase64)
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
CVariable::getValue(bool bBase64)
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
CVariable::getValue(std::string *pval, bool bBase64)
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
CVariable::setValue(bool val)
{
    vscp_str_format(m_strValue, "%s", val ? "true" : "false");
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVariable::getValue(bool *pValue)
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
CVariable::setValue(int val)
{
    vscp_str_format(m_strValue, "%d", val);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::getValue(int *pValue)
{
    *pValue = (int)vscp_readStringValue(m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::setValue(long val)
{
    vscp_str_format(m_strValue, "%ld", val);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::getValue(long *pValue)
{
    *pValue = vscp_readStringValue(m_strValue);
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::setValue(double val)
{
    vscp_str_format(m_strValue, "%lf", val);
}

///////////////////////////////////////////////////////////////////////////////
// getValue
//

void
CVariable::getValue(double *pValue)
{
    *pValue = stod(m_strValue);
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void
CVariable::setValue(cguid &guid)
{
    m_strValue = guid.getAsString();
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void
CVariable::setValue(vscpEvent &event)
{
    vscp_writeVscpEventToString(&event, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void
CVariable::getValue(vscpEvent *pEvent)
{
    if (NULL != pEvent) return;
    vscp_setVscpEventFromString(pEvent, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//
void
CVariable::setValue(vscpEventEx &eventex)
{
    vscp_writeVscpEventExToString(&eventex, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// getValue
//
void
CVariable::getValue(vscpEventEx *pEventEx)
{
    if (NULL != pEventEx) return;
    vscp_setVscpEventExFromString(pEventEx, m_strValue);
};

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::getValue(vscpdatetime *pValue)
{
    m_strValue = pValue->getISODateTime();
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::setValue(vscpdatetime &val)
{
    if (val.isValid()) {
        m_strValue = val.getISODateTime();
    } else {
        m_strValue = ("00-00-00T00:00:00");
    }
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::getValue(vscpEventFilter *pValue)
{
    vscp_readFilterMaskFromString( pValue, m_strValue);
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

void
CVariable::setValue(vscpEventFilter &val)
{
    std::string strFilter, strMask;

    vscp_writeFilterToString( &val, strFilter );
    vscp_writeMaskToString( &val, strMask );
    m_strValue = strFilter;
    m_strValue += ",";
    m_strValue += strMask;
}

///////////////////////////////////////////////////////////////////////////////
// setValueFromString
//

bool
CVariable::setValueFromString(int type,
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

        case VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_EVENT:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_GUID:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE:
            m_strValue = str;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP:
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
CVariable::getNote(bool bBase64)
{
    std::string strNote = m_note;
    if (bBase64) vscp_base64_std_encode(strNote);

    return strNote;
}

///////////////////////////////////////////////////////////////////////////////
// getNote
//

bool
CVariable::getNote(std::string &strNote, bool bBase64)
{
    strNote = m_note;

    if (bBase64) vscp_base64_std_encode(strNote);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setFromString
//
// Format is:
// "variable;name";"type";"persistence";"user";"rights";"value";"note"
// value and note is always BASE64 encoded.
//

bool
CVariable::setFromString(const std::string &strVariable,
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
        setValueFromString(m_type, value);
    } else {
        return false;
    }

    // Get the note (if any)
    // Get the value of the variable
    if (!tokens.empty()) {

        std::string note = tokens.front();
        tokens.pop_front();
        setNote(note);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setValue
//

bool
CVariable::setValue(const std::string &strValue, bool bBase64)
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
CVariable::setNote(const std::string &strNote, bool bBase64)
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
CVariable::reset(void)
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

            case VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT:
                m_strValue =
                  ("0,0,0,255,255"); // value,unit,sensor-index,zone,subzone
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_GUID:
                m_strValue =
                  ("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA:
                m_strValue = ("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE:
                m_strValue = ("0");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP:
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
CVariable::isTrue(void)
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
CVariable::setTrue(void)
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
CVariable::setFalse(void)
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

    // Set the default dm configuration paths
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
    CVariable variable;

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

    // Get admin user
    CUserItem *pUserItem = gpobj->m_userList.getUser(0);

    // Initialize stock variables
    if (NULL != pUserItem) {
        initStockVariables(pUserItem);
    } else {
        syslog(LOG_ERR,
               "Init remote variables but failed to get admin user for stock "
               "variable init.");
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// exist
//

uint32_t
CVariableStorage::exist(const std::string &name)
{
    CVariable variable;

    // Get admin user
    CUserItem *pUserItem = gpobj->m_userList.getUser(0);

    return find(name, pUserItem, variable);
}

///////////////////////////////////////////////////////////////////////////////
// find
//

uint32_t
CVariableStorage::find(const std::string &name,
                       CUserItem *pUser,
                       CVariable &variable)
{
    uint32_t id         = 0;
    std::string lc_name = vscp_lower(name);

    // Check pointer
    if (NULL == pUser) {
        syslog(LOG_ERR,
               "find remote variable %s but user is undefined",
               name.c_str());
        return 0;
    }

    // Look for stock variables
    if (vscp_startsWith(lc_name, "vscp.")) {
        if (handleStockVariable(name, STOCKVAR_READ, variable, pUser)) {
            return variable.getID();
        } else {
            return 0;
        }
    } else {

        // Search for non-persistent
        if (id = findNonPersistentVariable(name, pUser, variable)) {
            return id;
        } else {
            return findPersistentVariable(name, pUser, variable);
        }
    }

    return 0; // Not found
}

///////////////////////////////////////////////////////////////////////////////
// setVariableFromDbRecord
//

bool
CVariableStorage::setVariableFromDbRecord(sqlite3_stmt *ppStmt,
                                          CVariable &variable)
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
                                            CUserItem *pUser,
                                            CVariable &variable)
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
        /*   TODO
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
                                         CUserItem *pUser,
                                         CVariable &variable)
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
        /*   TODO ????
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
CVariableStorage::listItem(varQuery *pq, CVariable &variable)
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
CVariableStorage::addStockVariable(CVariable &var)
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
CVariableStorage::add(CVariable &var)
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
    CVariable variable;

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
    uint8_t type = CVariable::getVariableTypeFromString(strType);
    return add(varName, value, type, userid, bPersistent, accessRights, note);
}

///////////////////////////////////////////////////////////////////////////////
// update
//

bool
CVariableStorage::update(CVariable &var, CUserItem *pUser)
{
    long id       = 0;
    char *zErrMsg = 0;

    // Must exists
    if (0 == (id = exist(var.getName()))) {
        return false;
    }

    // Check if user is allowed to do this
    if ( !var.isUserAllowWrite( pUser ) ) {
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
        return handleStockVariable(var.getName(),
                                      STOCKVAR_WRITE,
                                      var,
                                      pUser);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool
CVariableStorage::remove(std::string &name, CUserItem *pUser)
{
    char *zErrMsg = 0;
    CVariable variable;

    if (!find(name, pUser, variable)) {
        syslog(
          LOG_ERR, "Trying to remove variable %s but variable not found.", name.c_str());
        return false;
    }

    // If stock variable it can't be removed
    if (variable.isStockVariable()) {
        syslog(LOG_ERR,
               "Trying to remove stockvariable %s not allowed.",
               name.c_str());
        return false;
    }

    // Check if user is allowed to do this
    if ( !variable.isUserAllowWrite( pUser ) ) {
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
CVariableStorage::remove(CVariable &variable, CUserItem *pUser)
{
    std::string str = variable.getName();
    return remove(str, pUser);
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

static int depth_load_var_parser = 0;

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

            CVariable var;

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
                    // Always BASE64 encoded on file
                    var.setValue(attribute, true);
                } else if (0 == vscp_strcasecmp(attr[i], "note")) {
                    //  Allways BASE64 encoded on file
                    var.setNote(attribute, true);
                }
            }

            if (pStorage->exist(var.getName())) {
                if (pStorage->update(var, NULL)) {
                    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_VARIABLE) {
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
                    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_VARIABLE) {
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
        if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_VARIABLE) {
            syslog(LOG_ERR,
                   "[loadFromXML] Loading variable XML file from %s.",
                   m_xmlPath.c_str());
        }

    } else {
        xmlpath = path;
        if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_VARIABLE) {
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

                    CVariable variable;

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

                    CVariable variable;

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
CVariableStorage::writeVariableToXmlFile(std::ofstream &of, CVariable &variable)
{
    std::string str, strtemp;
    std::string name = variable.getName();

    try {
        of << vscp_str_format("  <variable type=\"%d\" ", variable.getType());
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
        of << vscp_str_format(" value=\"%s\" ", (const char *)strtemp.c_str());
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
