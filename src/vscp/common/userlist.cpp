// userlist.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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

#ifdef __GNUG__
//#pragma implementation
#endif

#include <deque>
#include <map>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <vscp_aes.h>
#include <controlobject.h>
#include <vscpdb.h>
#include <vscphelper.h>
#include "userlist.h"

// Forward declarations
void
vscp_md5(char* digest, const unsigned char* buf, size_t len);

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject* gpobj;

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserItem::CUserItem(void)
{
    m_userID = VSCP_ADD_USER_UNINITIALISED;

    // Accept all events
    vscp_clearVSCPFilter(&m_filterVSCP);

    // No user rights
    m_userRights = 0x00000000;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserItem::~CUserItem(void)
{
    m_listAllowedRemotes.clear();
    m_listAllowedEvents.clear();
}

///////////////////////////////////////////////////////////////////////////////
// fixName
//

void
CUserItem::fixName(void)
{
    vscp_trim(m_user);

    // Works only for ASCII names. Should be fixed so
    // UTF8 names can be used TODO
    for (size_t i = 0; i < m_user.length(); i++) {
        switch ((const char)m_user[i]) {
            case ';':
            case '\'':
            case '\"':
            case ',':
            case ' ':
                m_user[i] = '_';
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// setFromString
//
// name;password;fullname;filtermask;rights;remotes;events;note
//

bool
CUserItem::setFromString(const std::string& userSettings)
{
    std::string strToken;
    std::deque<std::string> tokens;
    vscp_split(tokens, userSettings, ";");

    // name
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setUserName(strToken);
            fixName();
        }
    }

    // password
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setPassword(strToken);
        }
    }

    // fullname
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setFullname(strToken);
        }
    }

    // filter
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setFilterFromString(strToken);
        }
    }

    // mask
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        setFilterFromString(strToken);
    }

    // rights
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setUserRightsFromString(strToken);
        }
    }

    // remotes
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        setAllowedRemotesFromString(strToken);
    }

    // events
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            setAllowedEventsFromString(strToken);
        }
    }

    // note
    if (!tokens.empty()) {
        strToken = tokens.front();
        tokens.pop_front();
        vscp_trim(strToken);
        if (strToken.length()) {
            vscp_base64_std_decode(strToken);
            setNote(strToken);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserItem::getAsString(std::string& strUser)
{
    std::string str;
    strUser.clear();

    strUser += vscp_str_format("%ld;", getUserID());
    strUser += getUserName();
    strUser += ";";
    // Protect password
    str = getPassword();
    for (size_t i = 0; i < str.length(); i++) {
        strUser += "*";
    }
    // strUser += getPassword();
    strUser += ";";
    strUser += getFullname();
    strUser += ";";
    vscp_writeFilterToString(str, getUserFilter());
    strUser += str;
    strUser += ";";
    vscp_writeMaskToString(str, getUserFilter());
    strUser += str;
    strUser += ";";
    strUser += getUserRightsAsString();
    strUser += ";";
    strUser += getAllowedRemotesAsString();
    strUser += ";";
    strUser += getAllowedEventsAsString();
    strUser += ";";

    str = getNote();
    vscp_base64_std_encode(str);
    strUser += str;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAsMap
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserItem::getAsMap(std::map<std::string, std::string>& mapUser)
{
    std::string str, wstr;

    mapUser["userid"] = vscp_str_format("%ld;", getUserID());
    mapUser["name"]   = getUserName();

    // Protect password
    wstr = "";
    str  = getPassword();
    for (size_t i = 0; i < str.length(); i++) {
        wstr += "*";
    }
    mapUser["password"] = wstr;
    mapUser["fullname"] = getFullname();

    vscp_writeFilterToString(str, getUserFilter());
    mapUser["filter"] = str;

    vscp_writeMaskToString(str, getUserFilter());
    mapUser["mask"]    = str;
    mapUser["rights"]  = getUserRightsAsString();
    mapUser["remotes"] = getAllowedRemotesAsString();
    mapUser["events"]  = getAllowedEventsAsString();

    str = getNote();
    vscp_base64_std_encode(str);
    mapUser["note"] = str;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setUserRightsFromString
//

bool
CUserItem::setUserRightsFromString(const std::string& strRights)
{
    // Privileges
    if (strRights.length()) {

        m_userRights = 0;

        std::deque<std::string> tokens;
        vscp_split(tokens, strRights, "/");

        while (!tokens.empty()) {

            std::string str = tokens.front();
            tokens.pop_front();

            if (0 == strcasecmp(str.c_str(), "admin")) {
                // All rights
                m_userRights |= VSCP_ADMIN_DEFAULT_RIGHTS;
            } else if (0 == strcasecmp(str.c_str(), "user")) {
                // A standard user
                m_userRights |= VSCP_USER_DEFAULT_RIGHTS;
            } else if (0 == strcasecmp(str.c_str(), "driver")) {
                // A standard driver
                m_userRights |= VSCP_DRIVER_DEFAULT_RIGHTS;
            } else if (0 == strcasecmp(str.c_str(), "send-events")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_SEND_EVENT;
            } else if (0 == strcasecmp(str.c_str(), "receive-events")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_RCV_EVENT;
            } else if (0 == strcasecmp(str.c_str(), "l1ctrl-events")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_SEND_L1CTRL_EVENT;
            } else if (0 == strcasecmp(str.c_str(), "l2ctrl-events")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_SEND_L2CTRL_EVENT;
            } else if (0 == strcasecmp(str.c_str(), "hlo-events")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_SEND_HLO_EVENT;
            } else if (0 == strcasecmp(str.c_str(), "shutdown")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_SHUTDOWN;
            } else if (0 == strcasecmp(str.c_str(), "restart")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_RESTART;
            } else if (0 == strcasecmp(str.c_str(), "interface")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_INTERFACE;
            } else if (0 == strcasecmp(str.c_str(), "test")) {
                m_userRights |= VSCP_USER_RIGHT_ALLOW_TEST;
            } else {
                // Numerical
                uint32_t val = vscp_readStringValue(str);
                m_userRights |= val;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedEvent
//

bool
CUserItem::getAllowedEvent(size_t n, std::string& event)
{
    if (!m_listAllowedEvents.size()) {
        return false;
    }

    if (n > (m_listAllowedEvents.size() - 1)) {
        return false;
    }

    event = m_listAllowedEvents[n];
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedEvent
//

bool
CUserItem::setAllowedEvent(size_t n, std::string& event)
{
    if (!m_listAllowedEvents.size()) {
        return false;
    }

    if (n > (m_listAllowedEvents.size() - 1)) {
        return false;
    }

    m_listAllowedEvents[n] = event;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addAllowedEvent
//

bool
CUserItem::addAllowedEvent(const std::string& strEvent)
{
    std::string str     = strEvent;
    uint16_t vscp_class = 0;
    uint16_t vscp_type  = 0;

    vscp_trim(str);

    // We want to store in standard for "%04X:%04X" so we
    // need to extract the values or wildcards
    if ("*:*" == str) {
        m_listAllowedEvents.push_back(str);
        return true;
    }

    // Left wildcard
    if ('*' == strEvent[0]) {
        str       = vscp_str_right(str, str.length() - 2);
        vscp_type = vscp_readStringValue(str);
        str       = vscp_str_format("*:%04X", vscp_type);
        m_listAllowedEvents.push_back(str);
        return true;
    }

    // Right wildcard
    if ('*' == str[str.length() - 1]) {
        str        = vscp_str_left(str, str.length() - 2);
        vscp_class = vscp_readStringValue(str);
        str        = vscp_str_format("%04X:*", vscp_class);
        m_listAllowedEvents.push_back(str);
        return true;
    }

    // class:type
    vscp_class = vscp_readStringValue(str);
    size_t pos;
    if (std::string::npos != (pos = str.find(':'))) {
        str       = vscp_str_right(str, str.length() - pos - 1);
        vscp_type = vscp_readStringValue(str);
        str       = vscp_str_format("%04X:%04X", vscp_class, vscp_type);
        m_listAllowedEvents.push_back(str);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedEventsFromString
//

bool
CUserItem::setAllowedEventsFromString(const std::string& strEvents, bool bClear)
{
    std::string str;

    // Privileges
    if (strEvents.length()) {

        if (bClear) {
            m_listAllowedEvents.clear();
        }

        std::deque<std::string> tokens;
        vscp_split(tokens, strEvents, ",");

        while (!tokens.empty()) {
            str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);

            addAllowedEvent(str);
        };
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedEventsAsString
//

std::string
CUserItem::getAllowedEventsAsString(void)
{
    std::string strAllowedEvents;

    for (size_t i = 0; i < m_listAllowedEvents.size(); i++) {

        strAllowedEvents += m_listAllowedEvents[i];

        if (i != (m_listAllowedEvents.size() - 1)) {
            strAllowedEvents += "/";
        }
    }

    return strAllowedEvents;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedRemotesFromString
//

bool
CUserItem::setAllowedRemotesFromString(const std::string& strConnect)
{
    // Privileges
    if (strConnect.length()) {

        m_listAllowedRemotes.clear();

        std::deque<std::string> tokens;
        vscp_split(tokens, strConnect, ",");

        while (!tokens.empty()) {
            std::string remote = tokens.front();
            tokens.pop_front();
            vscp_trim(remote);
            addAllowedRemote(remote);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedRemotesAsString
//

std::string
CUserItem::getAllowedRemotesAsString(void)
{
    size_t i;
    std::string strAllowedRemotes;

    for (i = 0; i < m_listAllowedRemotes.size(); i++) {

        strAllowedRemotes += m_listAllowedRemotes[i];

        if (i != (m_listAllowedRemotes.size() - 1)) {
            strAllowedRemotes += ",";
        }
    }

    return strAllowedRemotes;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedRemote
//

bool
CUserItem::getAllowedRemote(size_t n, std::string& remote)
{
    if (!m_listAllowedRemotes.size()) {
        return false;
    }

    if (n > (m_listAllowedRemotes.size() - 1)) {
        return false;
    }

    remote = m_listAllowedRemotes[n];
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedRemote
//

bool
CUserItem::setAllowedRemote(size_t n, std::string& remote)
{
    if (!m_listAllowedRemotes.size()) {
        return false;
    }

    if (n > (m_listAllowedRemotes.size() - 1)) {
        return false;
    }

    m_listAllowedRemotes[n] = remote;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUserRightsAsString
//

std::string
CUserItem::getUserRightsAsString(void)
{
    std::string strRights;

    for (int i = 0; i < 32; i++) {
        strRights += vscp_str_format("%d", (m_userRights & (2^i)) ? 1 : 0);
    }

    std::reverse(strRights.begin(), strRights.end());
    return strRights;
}

////////////////////////////////////////////////////////////////////////////////
// isAllowedToConnect
//
//

int
CUserItem::isAllowedToConnect(uint32_t remote_ip)
{
    int allowed = '+';
    int flag;
    uint32_t net, mask;

    remote_ip = htonl(remote_ip);

    // If the list is empty - allow all
    // if (0 == m_listAllowedRemotes.size()) return 1;

    for (size_t i = 0; i < m_listAllowedRemotes.size(); i++) {

        flag = m_listAllowedRemotes[i].at(0); // vec.ptr[0];
        if ((flag != '+' && flag != '-') ||
            (0 ==
             vscp_parse_ipv4_addr(m_listAllowedRemotes[i].substr(1).c_str(),
                                  &net,
                                  &mask))) {
            return -1;
        }

        if (net == (remote_ip & mask)) {
            allowed = flag;
        }
    }

    return ('+' == allowed) ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
// isUserAllowedToSendEvent
//

bool
CUserItem::isUserAllowedToSendEvent(const uint32_t vscp_class,
                                    const uint32_t vscp_type)
{
    unsigned int i;
    std::string str;

    // If empty all events allowed
    if (m_listAllowedEvents.empty())
        return true;

    // test wildcard *.*
    str = "*:*";
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str)
            return true;
    }

    str = vscp_str_format("%04X:%04X", vscp_class, vscp_type);
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str)
            return true;
    }

    // test wildcard class.*
    str = vscp_str_format("%04X:*", vscp_class);
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str)
            return true;
    }

    syslog(LOG_ERR, "isUserAllowedToSendEvent: Not allowed to send event - ");

    return false;
}

//*****************************************************************************
//                              CUserList
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserList::CUserList(void)
{
    // First local user except the super user has id 1
    m_cntLocaluser = 1;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserList::~CUserList(void)
{
    {
        for (std::map<std::string, CGroupItem*>::iterator it =
               m_grouphashmap.begin();
             it != m_grouphashmap.end();
             ++it) {
            CGroupItem* pItem = it->second;
            if (NULL != pItem) {
                delete pItem;
            }
        }
    }

    m_grouphashmap.clear();

    {
        for (std::map<std::string, CUserItem*>::iterator it =
               m_userhashmap.begin();
             it != m_userhashmap.end();
             ++it) {
            CUserItem* pItem = it->second;
            if (NULL != pItem) {
                delete pItem;
            }
        }
    }

    m_userhashmap.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool
CUserList::addSuperUser(const std::string& user,
                        const std::string& password,
                        const std::string& strDomain,
                        const std::string& allowedRemotes,
                        uint32_t bFlags)
{
    //char buf[512];

    // Cant add user with username that is already defined.
    if (NULL != m_userhashmap[user]) {
        return false;
    }

    // New user item
    CUserItem* pItem = new CUserItem;
    if (NULL == pItem) {
        syslog(LOG_ERR,
               "addSuperUser: Failed to delete user - "
               "User is not defined.");
        return false;
    }

    // ----

    char buf[2048], secret[2048];
    uint8_t iv[16];
    std::string strIV;
    std::string strCrypto;

    std::deque<std::string> tokens;
    vscp_split(tokens, password, ";");
    strIV = tokens.front();
    tokens.pop_front();
    strCrypto = tokens.front();

    strIV = "5a475c082c80dcdf7f2dfbd976253b24";
    strCrypto ="69b1180d2f4809d39be34e19c750107f";
    if (0 == vscp_hexStr2ByteArray(iv, 16, (const char*)strIV.c_str())) {
        syslog(LOG_ERR,
               "[addSuperUser] Authentication: No room "
               "for iv block. ");
        return false; // Not enough room in buffer
    }

    size_t len;
    if (0 == (len = vscp_hexStr2ByteArray((uint8_t *)secret,
                                          strCrypto.length(),
                                          (const char*)strCrypto.c_str()))) {
        syslog(LOG_ERR,
               "[addSuperUser] Authentication: No room "
               "for crypto block. ");
        return false; // Not enough room in buffer
    }

    memset(buf, 0, sizeof(buf));
    AES_CBC_decrypt_buffer(AES128, (uint8_t *)buf, (uint8_t *)secret, len, gpobj->m_systemKey, iv);

    // std::string str = std::string((const char*)buf);
    // std::deque<std::string> tokens;
    // vscp_split(tokens, str, ":");

    // ----

    pItem->setUserID(0); // Super user is always at id = 0

    std::string driverhash = user;
    driverhash += ":";
    driverhash += strDomain;
    driverhash += ":";
    driverhash += password;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, (const char*)driverhash.c_str(), driverhash.length());

    char digest[33];
    vscp_md5(digest, (const unsigned char*)buf, strlen(buf));

    pItem->setPasswordDomain(std::string(digest));

    pItem->setUserName(user);
    pItem->fixName();
    pItem->setPassword(password);
    pItem->setFullname("Admin user");
    pItem->setNote("Admin user");
    pItem->setFilter(NULL);
    pItem->setUserRightsFromString("admin");
    pItem->setAllowedRemotesFromString(allowedRemotes);

    // Add to the map
    m_userhashmap[user] = pItem;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool
CUserList::addUser(const std::string& user,
                   const std::string& password,
                   const std::string& fullname,
                   const std::string& strNote,
                   const std::string& strDomain,
                   const vscpEventFilter* pFilter,
                   const std::string& userRights,
                   const std::string& allowedRemotes,
                   const std::string& allowedEvents,
                   uint32_t bFlags)
{
    char buf[512];

    // Cant add user with name that is already defined.
    if (NULL != m_userhashmap[user]) {
        syslog(LOG_ERR,
               "addUser: Failed to add user - "
               "user is already defined.");
        return false;
    }

    // // New user item
    CUserItem* pItem = new CUserItem;
    if (NULL == pItem) {
        syslog(LOG_ERR,
               "addUser: Failed to add user - "
               "Memory problem (CUserItem).");
        return false;
    }

    pItem->setUserID(m_cntLocaluser);
    m_cntLocaluser++; // Update local user id counter

    // MD5 Token
    std::string driverhash = user;
    driverhash += ":";
    driverhash += strDomain;
    driverhash += ":";
    driverhash += password;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, (const char*)driverhash.c_str(), driverhash.length());

    char digest[33];
    vscp_md5(digest, (const unsigned char*)buf, strlen(buf));

    pItem->setPasswordDomain(std::string(digest));

    pItem->setUserName(user);
    pItem->fixName();
    pItem->setPassword(password);
    pItem->setFullname(fullname);
    pItem->setNote(strNote);
    pItem->setFilter(pFilter);
    pItem->setUserRightsFromString(userRights);
    pItem->setAllowedRemotesFromString(allowedRemotes);
    pItem->setAllowedEventsFromString(allowedEvents);

    // Add to the map
    m_userhashmap[user] = pItem;

    // Set filter filter
    if (NULL != pFilter) {
        pItem->setFilter(pFilter);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//
// name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::addUser(const std::string& strUser,
                   const std::string& strDomain,
                   bool bUnpackNote)
{
    std::string strToken;
    std::string user;
    std::string password;
    std::string fullname;
    std::string strNote;
    vscpEventFilter filter;
    std::string userRights;
    std::string allowedRemotes;
    std::string allowedEvents;

    std::deque<std::string> tokens;
    vscp_split(tokens, strUser, ";");

    // user
    if (!tokens.empty()) {
        user = tokens.front();
        tokens.pop_front();
    }

    // password
    if (!tokens.empty()) {
        password = tokens.front();
        tokens.pop_front();
    }

    // fullname
    if (!tokens.empty()) {
        fullname = tokens.front();
        tokens.pop_front();
    }

    // filter
    if (!tokens.empty()) {
        vscp_readFilterFromString(&filter, tokens.front());
        tokens.pop_front();
    }

    // mask
    if (!tokens.empty()) {
        vscp_readMaskFromString(&filter, tokens.front());
        tokens.pop_front();
    }

    // user rights
    if (!tokens.empty()) {
        userRights = tokens.front();
        tokens.pop_front();
    }

    // allowed remotes
    if (!tokens.empty()) {
        allowedRemotes = tokens.front();
        tokens.pop_front();
    }

    // allowed events
    if (!tokens.empty()) {
        allowedEvents = tokens.front();
        tokens.pop_front();
    }

    // note
    if (!tokens.empty()) {
        if (bUnpackNote) {
            strNote = tokens.front();
            tokens.pop_front();
            vscp_base64_std_decode(strNote);
        } else {
            strNote = tokens.front();
            tokens.pop_front();
        }
    }

    return addUser(user,
                   password,
                   fullname,
                   strNote,
                   strDomain,
                   &filter,
                   userRights,
                   allowedRemotes,
                   allowedEvents);
}

///////////////////////////////////////////////////////////////////////////////
// deleteUser
//

bool
CUserList::deleteUser(const std::string& user)
{
    CUserItem* pUser = getUser(user);
    if (NULL == pUser) {
        syslog(LOG_ERR,
               "deleteUser: Failed to delete user - "
               "User is not defined.");
        return false;
    }

    // Remove also from internal table
    m_userhashmap.erase(user);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// deleteUser
//

bool
CUserList::deleteUser(const long userid)
{
    CUserItem* pUser = getUser(userid);
    if (NULL == pUser) {
        syslog(LOG_ERR,
               "deleteUser: Failed to delete user - "
               "User is not defined.");
        return false;
    }

    return deleteUser(pUser->getUserName());
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem*
CUserList::getUser(const std::string& user)
{
    return m_userhashmap[user];
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem*
CUserList::getUser(const long userid)
{
    std::map<std::string, CUserItem*>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key      = it->first;
        CUserItem* pUserItem = it->second;
        if (userid == pUserItem->getUserID()) {
            return pUserItem;
        }
    }

    syslog(LOG_ERR,
           "getUser: Failed to get user - "
           "User is not found.");

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// validateUser
//

CUserItem*
CUserList::validateUser(const std::string& user, const std::string& password)
{
    CUserItem* pUserItem;

    pUserItem = m_userhashmap[user];
    if (NULL == pUserItem) {
        syslog(LOG_ERR,
               "validateUser: Failed to validate user - "
               "User is not defined.");
        return NULL;
    }

    if (!vscp_isPasswordValid(pUserItem->getPassword(), password)) {
        syslog(LOG_INFO,
               "validateUser: Failed to validate user - "
               "Check username/password.");
        return NULL;
    }

    return pUserItem;
}

///////////////////////////////////////////////////////////////////////////////
// validateUserDomain
//

CUserItem*
CUserList::validateUserDomain(const std::string& user,
                              const std::string& md5password)
{
    CUserItem* pUserItem;

    pUserItem = m_userhashmap[user];
    if (NULL == pUserItem) {
        syslog(LOG_ERR,
               "validateUserDomain: Failed to validate user - "
               "User is not defined.");
        return NULL;
    }

    // Check password
    if (pUserItem->getPasswordDomain() != md5password) {
        syslog(LOG_INFO,
               "validateUserDomain: Failed to validate user - "
               "Check username/password.");
        return NULL;
    }

    return pUserItem;
}

///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::getUserAsString(CUserItem* pUserItem, std::string& strUser)
{
    std::string str;
    strUser.clear();

    // Check pointer
    if (NULL == pUserItem) {
        syslog(LOG_ERR,
               "getUserAsString: Failed to get user - "
               "IOnvalid user item.");
        return false;
    }

    return pUserItem->getAsString(strUser);
}

///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::getUserAsString(uint32_t idx, std::string& strUser)
{
    std::string str;
    uint32_t i = 0;

    std::map<std::string, CUserItem*>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {

        if (i == idx) {
            std::string key      = it->first;
            CUserItem* pUserItem = it->second;
            if (getUserAsString(pUserItem, strUser)) {
                return true;
            } else {
                return false;
            }
        }

        i++;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// getAllUsers
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::getAllUsers(std::string& strAllusers)
{
    std::string str;
    strAllusers.clear();

    std::map<std::string, CUserItem*>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key      = it->first;
        CUserItem* pUserItem = it->second;
        if (getUserAsString(pUserItem, str)) {
            strAllusers += str;
            strAllusers += "\r\n";
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getAllUsers
//
//

bool
CUserList::getAllUsers(std::deque<std::string>& arrayUsers)
{
    std::string str;

    std::map<std::string, CUserItem*>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key = it->first;
        // CUserItem* pUserItem = it->second;
        arrayUsers.push_back(key);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUserItemFromOrdinal
//
//

CUserItem*
CUserList::getUserItemFromOrdinal(uint32_t idx)
{
    std::string str;
    uint32_t i = 0;

    std::map<std::string, CUserItem*>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {

        if (i == idx) {
            std::string key      = it->first;
            CUserItem* pUserItem = it->second;
            return pUserItem;
        }

        i++;
    }

    return NULL;
}