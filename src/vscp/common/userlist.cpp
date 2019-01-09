// userlist.cpp
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

#ifdef __GNUG__
//#pragma implementation
#endif

#include <deque>
#include <map>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <sqlite3.h>

#include "userlist.h"
#include <controlobject.h>
#include <vscpdb.h>
#include <vscphelper.h>

// Forward declarations
void
vscp_md5(char *digest, const unsigned char *buf, size_t len);

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CUserItem::CUserItem(void)
{
    m_userID = VSCP_ADD_USER_UNINITIALISED;

    // Accept all events
    vscp_clearVSCPFilter(&m_filterVSCP);

    // No user rights
    memset(m_userRights, 0, sizeof(m_userRights));
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserItem::~CUserItem(void)
{
    m_listAllowedIPV4Remotes.clear();
    m_listAllowedIPV6Remotes.clear();
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
    for (int i = 0; i < m_user.length(); i++) {
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
CUserItem::setFromString(std::string userSettings)
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
CUserItem::getAsString(std::string &strUser)
{
    std::string str;
    strUser.clear();

    strUser += vscp_str_format("%ld;", getUserID());
    strUser += getUserName();
    strUser += ";";
    // Protect password
    str = getPassword();
    for (int i = 0; i < str.length(); i++) {
        strUser += "*";
    }
    // strUser += getPassword();
    strUser += ";";
    strUser += getFullname();
    strUser += ";";
    vscp_writeFilterToString(getFilter(), str);
    strUser += str;
    strUser += ";";
    vscp_writeMaskToString(getFilter(), str);
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
// setUserRightsFromString
//

bool
CUserItem::setUserRightsFromString(const std::string &strRights)
{
    // Privileges
    if (strRights.length()) {

        std::deque<std::string> tokens;
        vscp_split(tokens, strRights, "/");

        int idx = 0;
        while (!tokens.empty()) {

            std::string str = tokens.front();
            tokens.pop_front();

            if (0 == strcasecmp(str.c_str(), "admin")) {
                // All rights
                // memset( m_userRights, 0xff, sizeof( m_userRights ) );
                // All rights
                for (int i = 0; i < USER_PRIVILEGE_BYTES; i++) {
                    setUserRights(i, 0xff);
                }
            } else if (0 == strcasecmp(str.c_str(), "user")) {
                // A standard user
                setUserRights(0, 0x06);
            } else if (0 == strcasecmp(str.c_str(), "driver")) {
                // A standard driver
                setUserRights(0, 0x0f);
            } else {
                // Numerical
                unsigned long lval = stoul(str);
                setUserRights(idx++, (uint8_t)lval);
            }

            if (idx >= sizeof(m_userRights)) break;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedEventsFromString
//

bool
CUserItem::setAllowedEventsFromString(const std::string &strEvents)
{
    std::string str;

    // Privileges
    if (strEvents.length()) {

        m_listAllowedEvents.clear();

        std::deque<std::string> tokens;
        vscp_split(tokens, strEvents, "/");

        while (!tokens.empty()) {
            str = tokens.front();
            tokens.pop_front();
            m_listAllowedEvents.push_back(str);
        };
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setAllowedRemotesFromString
//

bool
CUserItem::setAllowedRemotesFromString(const std::string &strConnect)
{
    // Privileges
    if (strConnect.length()) {

        m_listAllowedIPV4Remotes.clear();
        m_listAllowedIPV6Remotes.clear();

        std::deque<std::string> tokens;
        vscp_split(tokens, strConnect, ",");

        while (!tokens.empty()) {
            std::string remote = tokens.front();
            tokens.pop_front();
            vscp_trim(remote);
            addAllowedRemote(remote);
        }

        // TODO handel Ipv6

        /*do {
            std::string remote = tokens.front();
            tokens.pop_front();
            if ( !remote.empty() ) {
                vscp_trim( remote ;
                if ( ( "*"  == remote ) || ( "*.*.*.*" == remote ) ) {
                    // All is allowed to connect
                    clearAllowedRemoteList();
                }
                else {
                    addAllowedRemote( remote );
                }
            }
        } while ( !tokes.empty() );*/
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUserRightsAsString
//

std::string
CUserItem::getUserRightsAsString(void)
{
    std::string strRights;

    for (int i = 0; i < USER_PRIVILEGE_BYTES; i++) {
        strRights += vscp_str_format("%d", m_userRights[i]);
        if (i != (USER_PRIVILEGE_BYTES - 1)) {
            strRights += "/";
        }
    }

    return strRights;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedEventsAsString
//

std::string
CUserItem::getAllowedEventsAsString(void)
{
    std::string strAllowedEvents;

    for (int i = 0; i < m_listAllowedEvents.size(); i++) {
        strAllowedEvents += m_listAllowedEvents[i];
        if (i != (m_listAllowedEvents.size() - 1)) strAllowedEvents += "/";
    }

    return strAllowedEvents;
}

///////////////////////////////////////////////////////////////////////////////
// getAllowedRemotesAsString
//

std::string
CUserItem::getAllowedRemotesAsString(void)
{
    int i;
    std::string strAllowedRemotes;

    for (i = 0; i < m_listAllowedIPV4Remotes.size(); i++) {
        strAllowedRemotes += m_listAllowedIPV4Remotes[i];
        if (i != (m_listAllowedIPV4Remotes.size() - 1))
            strAllowedRemotes += "/";
    }

    for (i = 0; i < m_listAllowedIPV6Remotes.size(); i++) {
        strAllowedRemotes += m_listAllowedIPV6Remotes[i];
        if (i != (m_listAllowedIPV6Remotes.size() - 1))
            strAllowedRemotes += "/";
    }

    return strAllowedRemotes;
}

///////////////////////////////////////////////////////////////////////////////
// isUserInDB
//

bool
CUserItem::isUserInDB(const unsigned long userid)
{
    bool rv = false;
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char psql[512];

    sprintf(psql,
            VSCPDB_USER_CHECK_USER_ID,
            (unsigned long)(userid - VSCP_LOCAL_USER_OFFSET));

    // Database must be open
    if (NULL != gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR, "isUserInDB: VSCP daemon database is not open.");
        return false;
    }

    pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_prepare(gpobj->m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
        return false;
    }

    // Get the result
    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        rv = true;
    }

    sqlite3_finalize(ppStmt);

    pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// isUserInDB
//

bool
CUserItem::isUserInDB(const std::string &user, long *pid)
{
    bool rv       = false;
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;

    // Database must be open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR, "isUserInDB: VSCP daemon database is not open.");
        return false;
    }

    // Search username
    char *sql =
      sqlite3_mprintf(VSCPDB_USER_CHECK_USER, (const char *)user.c_str());

    pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_prepare(gpobj->m_db_vscp_daemon, sql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "isUserInDB: Failed to read user database - prepare query.");
        sqlite3_free(sql);
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {
        rv = true;
        if (NULL != pid) {
            *pid = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_USER_ID);
        }
    }

    sqlite3_free(sql);
    sqlite3_finalize(ppStmt);

    pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// saveToDatabase
//

bool
CUserItem::saveToDatabase(void)
{
    char *zErrMsg = 0;

    // Database must be open
    if (NULL == gpobj->m_db_vscp_daemon) return false;

    // Internal records can't be saved to db
    // driver users is an example on users that are only local
    if (m_userID < 0) return false;

    std::string strFilter, strMask, strBoth;
    strBoth = strFilter + "," + strMask;
    vscp_writeFilterToString(&m_filterVSCP, strFilter);
    vscp_writeMaskToString(&m_filterVSCP, strMask);
    strBoth = strFilter + "," + strMask;

    if (0 == m_userID) {

        // Add
        char *sql =
          sqlite3_mprintf(VSCPDB_USER_INSERT,
                          (const char *)m_user.c_str(),
                          (const char *)m_password.c_str(),
                          (const char *)m_fullName.c_str(),
                          (const char *)strBoth.c_str(),
                          (const char *)getUserRightsAsString().c_str(),
                          (const char *)getAllowedEventsAsString().c_str(),
                          (const char *)getAllowedRemotesAsString().c_str(),
                          (const char *)m_note.c_str());

        pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

        if (SQLITE_OK !=
            sqlite3_exec(gpobj->m_db_vscp_daemon, sql, NULL, NULL, &zErrMsg)) {
            sqlite3_free(sql);
            pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
            return false;
        }

        sqlite3_free(sql);

        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);

        // The database item now have a id which we need to read back
        return readBackIndexFromDatabase();

    } else {

        // Update
        char *sql =
          sqlite3_mprintf(VSCPDB_USER_UPDATE,
                          (const char *)m_user.c_str(),
                          (const char *)m_password.c_str(),
                          (const char *)m_fullName.c_str(),
                          (const char *)strBoth.c_str(),
                          (const char *)getUserRightsAsString().c_str(),
                          (const char *)getAllowedEventsAsString().c_str(),
                          (const char *)getAllowedRemotesAsString().c_str(),
                          (const char *)m_note.c_str(),
                          m_userID);

        pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

        if (SQLITE_OK !=
            sqlite3_exec(gpobj->m_db_vscp_daemon, sql, NULL, NULL, &zErrMsg)) {
            syslog(LOG_ERR,
                   "Failed to update user database. Err=%s  SQL=%s",
                   zErrMsg,
                   sql);
            sqlite3_free(sql);
            pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
            return false;
        }

        sqlite3_free(sql);

        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readBackIndexFromDatabase
//

bool
CUserItem::readBackIndexFromDatabase(void)
{
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;
    char *sql =
      sqlite3_mprintf(VSCPDB_USER_CHECK_USER, (const char *)m_user.c_str());

    pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_prepare(gpobj->m_db_vscp_daemon, sql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR, "Failed to get index for user. SQL=%s", sql);
        sqlite3_free(sql);
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
        return false;
    }

    if (SQLITE_ROW != sqlite3_step(ppStmt)) {
        syslog(LOG_ERR, "Failed to get index result  for user. SQL=%s", sql);
        sqlite3_free(sql);
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
    }

    // Get index (offset to local user)
    m_userID = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_USER_ID) +
               VSCP_LOCAL_USER_OFFSET;

    sqlite3_free(sql);
    pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isAllowedToConnect
//

/*bool CUserItem::isAllowedToConnect(const std::string& remote)
{
    unsigned int i;
    std::string str;
    xxIPV4address ipaddr;
    if ( !ipaddr.Hostname( remote ) ) return false;

    // If empty all host allowed, This is "*.*.*.*" or "*"
    if ( m_listAllowedIPV4Remotes.IsEmpty() ) return true;

    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        xxLogDebug(m_listAllowedIPV4Remotes[ i ]);
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(remote)) return true;
    }

    std::deque<std::string> tokens;
    vscp_split( tokens, attribute, "." );
    std::string ip1 = tokens.front();
    tokens.pop_front();
    std::string ip2 = tokens.front();
    tokens.pop_front();
    std::string ip3 = tokens.front();
    tokens.pop_front();
    std::string ip4 = tokens.front();
    tokens.pop_front();

    // test wildcard a.b.c.*
    str.Printf("%s.%s.%s.*", ip1.c_str(), ip2.c_str(), ip3.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(str)) return true;
    }

    // test wildcard a.b.*.*
    str.Printf("%s.%s.*.*", ip1.c_str(), ip2.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(str)) return true;
    }

    // test wildcard a.*.*.*
    str.Printf("%s.*.*.*", ip1.c_str());
    for (i = 0; i < m_listAllowedIPV4Remotes.GetCount(); i++) {
        if (m_listAllowedIPV4Remotes[ i ].IsSameAs(str)) return true;
    }

    return false;
}*/

////////////////////////////////////////////////////////////////////////////////
// check_acl
//
//

int
CUserItem::isAllowedToConnect(uint32_t remote_ip)
{
    int allowed, flag;
    uint32_t net, mask;

    remote_ip = htonl(remote_ip);

    // If the list is empty - allow all
    allowed = (0 == m_listAllowedIPV4Remotes.size()) ? '+' : '-';

    for (int i = 0; i < m_listAllowedIPV4Remotes.size(); i++) {

        flag = m_listAllowedIPV4Remotes[i].at(0); // vec.ptr[0];
        if ((flag != '+' && flag != '-') ||
            (0 ==
             vscp_parse_ipv4_addr(
               m_listAllowedIPV4Remotes[i].substr(1).c_str(), &net, &mask))) {
            return -1;
        }

        if (net == (remote_ip & mask)) {
            allowed = flag;
        }
    }

    return (allowed == '+');
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
    if (m_listAllowedEvents.empty()) return true;

    // test wildcard *.*
    str = "*:*";
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str) return true;
    }

    str = vscp_str_format("%04X:%04X", vscp_class, vscp_type);
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str) return true;
    }

    // test wildcard class.*
    str = vscp_str_format("%04X:*", vscp_class);
    for (i = 0; i < m_listAllowedEvents.size(); i++) {
        if (m_listAllowedEvents[i] == str) return true;
    }

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

    // xxLogDebug( "Read Configuration: VSCPEnable=%s",
    //              ( m_bVSCPEnable ? "true" : "false" )  );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CUserList::~CUserList(void)
{
    {
        for (std::map<std::string, CGroupItem *>::iterator it =
               m_grouphashmap.begin();
             it != m_grouphashmap.end();
             ++it) {
            CGroupItem *pItem = it->second;
            if (NULL != pItem) delete pItem;
        }
    }

    m_grouphashmap.clear();

    {
        for (std::map<std::string, CUserItem *>::iterator it =
               m_userhashmap.begin();
             it != m_userhashmap.end();
             ++it) {
            CUserItem *pItem = it->second;
            if (NULL != pItem) delete pItem;
        }
    }

    m_userhashmap.clear();
}

///////////////////////////////////////////////////////////////////////////////
// loadUsers
//

bool
CUserList::loadUsers(void)
{
    // Check if user is already in the database
    char *pErrMsg = 0;
    sqlite3_stmt *ppStmt;
    const char *psql = VSCPDB_USER_ALL;

    // Check if database is open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "loadUsers: Failed to read VSCP settings database - "
               "database not open.");
        return false;
    }

    pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_prepare(gpobj->m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        syslog(
          LOG_ERR,
          "loadUsers: Failed to read VSCP settings database - prepare query.");
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        // New user item
        CUserItem *pItem = new CUserItem;
        if (NULL != pItem) {

            const unsigned char *p;

            // id (offset from local users)
            pItem->setUserID(
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_USER_ID) +
              VSCP_LOCAL_USER_OFFSET);

            // User
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_USERNAME);
            if (NULL != p) {
                pItem->setUserName(std::string((const char *)p));
            }

            // Password
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_PASSWORD);
            if (NULL != p) {
                pItem->setPassword(std::string((const char *)p));
            }

            // Fullname
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_FULLNAME);
            if (NULL != p) {
                pItem->setFullname(std::string((const char *)p));
            }

            // Event filter
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_FILTER);
            if (NULL != p) {
                std::string str((const char *)p);
                pItem->setFilterFromString(str);
            }

            // Rights
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_RIGHTS);
            if (NULL != p) {
                pItem->setUserRightsFromString((const char *)p);
            }

            // Allowed events
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_ALLOWED_EVENTS);
            if (NULL != p) {
                pItem->setAllowedEventsFromString((const char *)p);
            }

            // Allowed remotes
            p =
              sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_ALLOWED_REMOTES);
            if (NULL != p) {
                pItem->setAllowedRemotesFromString((const char *)p);
            }

            // Note
            p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_USER_NOTE);
            if (NULL != p) {
                pItem->setNote((const char *)p);
            }

            m_userhashmap[pItem->getUserName()] = pItem;

        } else {
            syslog(LOG_ERR, "Unable to allocate memory for new user.");
        }
    }

    sqlite3_finalize(ppStmt);
    pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//

bool
CUserList::addSuperUser(const std::string &user,
                        const std::string &password,
                        const std::string &allowedRemotes,
                        uint32_t bFlags)
{
    char buf[512];
    char *pErrMsg = 0;

    // Cant add user with name that is already defined.
    if (NULL != m_userhashmap[user]) {
        return false;
    }

    // New user item
    CUserItem *pItem = new CUserItem;
    if (NULL == pItem) return false;

    pItem->setUserID(0); // Super user is always at id = 0

    std::string driverhash = user;
    driverhash += ":";
    driverhash += std::string(gpobj->m_web_authentication_domain);
    driverhash += ":";
    driverhash += password;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, (const char *)driverhash.c_str(), driverhash.length());

    char digest[33];
    vscp_md5(digest, (const unsigned char *)buf, strlen(buf));

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
CUserList::addUser(const std::string &user,
                   const std::string &password,
                   const std::string &fullname,
                   const std::string &strNote,
                   const vscpEventFilter *pFilter,
                   const std::string &userRights,
                   const std::string &allowedRemotes,
                   const std::string &allowedEvents,
                   uint32_t bFlags)
{
    char buf[512];
    char *pErrMsg = 0;

    // Cant add user with name that is already defined.
    if (NULL != m_userhashmap[user]) {
        return false;
    }

    // Check if database is open
    if (!(bFlags & VSCP_ADD_USER_FLAG_LOCAL) &&
        (NULL == gpobj->m_db_vscp_daemon)) {
        syslog(LOG_ERR,
               "addUser: Failed to read VSCP settings database - "
               "database not open.");
        return false;
    }

    // New user item
    CUserItem *pItem = new CUserItem;
    if (NULL == pItem) return false;

    // Local user
    if (VSCP_ADD_USER_FLAG_LOCAL & bFlags) {
    }

    pItem->setUserID(m_cntLocaluser);
    m_cntLocaluser++; // Update local user id counter

    // Check if user is defined already
    if (!(bFlags & VSCP_ADD_USER_FLAG_LOCAL) && pItem->isUserInDB(user)) {
        delete pItem;
        return false;
    }

    // MD5 Token
    std::string driverhash = user;
    driverhash += ":";
    driverhash += gpobj->m_web_authentication_domain;
    driverhash += ":";
    driverhash += password;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, (const char *)driverhash.c_str(), driverhash.length());

    char digest[33];
    vscp_md5(digest, (const unsigned char *)buf, strlen(buf));

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

    // Save to database
    if (!(VSCP_ADD_USER_FLAG_LOCAL & bFlags)) {
        pItem->saveToDatabase();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addUser
//
// name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::addUser(const std::string &strUser, bool bUnpackNote)
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
                   &filter,
                   userRights,
                   allowedRemotes,
                   allowedEvents);
}

///////////////////////////////////////////////////////////////////////////////
// deleteUser
//

bool
CUserList::deleteUser(const std::string &user)
{
    char *zErrMsg    = 0;
    CUserItem *pUser = getUser(user);
    if (NULL == pUser) return false;

    // Internal users can't be deleted
    if (pUser->getUserID() < VSCP_LOCAL_USER_OFFSET) return false;

    // Check if database is open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "deleteUser: Failed to read VSCP "
               "settings database - database not open.");
        return false;
    }

    pthread_mutex_lock(&gpobj->m_db_vscp_configMutex);

    char *sql =
      sqlite3_mprintf(VSCPDB_USER_DELETE_USERNAME, (const char *)user.c_str());
    if (SQLITE_OK !=
        sqlite3_exec(gpobj->m_db_vscp_daemon, sql, NULL, NULL, &zErrMsg)) {
        sqlite3_free(sql);
        syslog(LOG_ERR,
               "Delete user: Unable to delete "
               "user in db. [%s] Err=%s\n",
               sql,
               zErrMsg);
        pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
        return false;
    }

    pthread_mutex_unlock(&gpobj->m_db_vscp_configMutex);
    sqlite3_free(sql);

    // Remove also from internal table
    m_userhashmap.erase(user);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem *
CUserList::getUser(const std::string &user)
{
    return m_userhashmap[user];
}

///////////////////////////////////////////////////////////////////////////////
// getUser
//

CUserItem *
CUserList::getUser(const long userid)
{
    std::map<std::string, CUserItem *>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key      = it->first;
        CUserItem *pUserItem = it->second;
        if (userid == pUserItem->getUserID()) {
            return pUserItem;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// validateUser
//

CUserItem *
CUserList::validateUser(const std::string &user, const std::string &password)
{
    CUserItem *pUserItem;

    pUserItem = m_userhashmap[user];
    if (NULL == pUserItem) return NULL;

    if (!vscp_isPasswordValid(pUserItem->getPassword(), password)) {
        return NULL;
    }

    return pUserItem;
}

///////////////////////////////////////////////////////////////////////////////
// validateUserDomain
//

CUserItem *
CUserList::validateUserDomain(const std::string &user,
                              const std::string &md5password)
{
    CUserItem *pUserItem;

    pUserItem = m_userhashmap[user];
    if (NULL == pUserItem) return NULL;

    // Check password
    if (pUserItem->getPasswordDomain() != md5password) return NULL;

    return pUserItem;
}

///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::getUserAsString(CUserItem *pUserItem, std::string &strUser)
{
    std::string str;
    strUser.clear();

    // Check pointer
    if (NULL == pUserItem) return false;

    return pUserItem->getAsString(strUser);
}

///////////////////////////////////////////////////////////////////////////////
// getUserAsString
//
// userid;name;password;fullname;filter;mask;rights;remotes;events;note
//

bool
CUserList::getUserAsString(uint32_t idx, std::string &strUser)
{
    std::string str;
    uint32_t i = 0;

    std::map<std::string, CUserItem *>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {

        if (i == idx) {
            std::string key      = it->first;
            CUserItem *pUserItem = it->second;
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
CUserList::getAllUsers(std::string &strAllusers)
{
    std::string str;
    strAllusers.clear();

    std::map<std::string, CUserItem *>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key      = it->first;
        CUserItem *pUserItem = it->second;
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
CUserList::getAllUsers(std::deque<std::string> &arrayUsers)
{
    std::string str;

    std::map<std::string, CUserItem *>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {
        std::string key      = it->first;
        CUserItem *pUserItem = it->second;
        arrayUsers.push_back(key);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getUserItemFromOrdinal
//
//

CUserItem *
CUserList::getUserItemFromOrdinal(uint32_t idx)
{
    std::string str;
    uint32_t i = 0;

    std::map<std::string, CUserItem *>::iterator it;
    for (it = m_userhashmap.begin(); it != m_userhashmap.end(); ++it) {

        if (i == idx) {
            std::string key      = it->first;
            CUserItem *pUserItem = it->second;
            return pUserItem;
        }

        i++;
    }

    return NULL;
}