// userlist.h
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

#if !defined(USERLIST__INCLUDED_)
#define USERLIST__INCLUDED_

#include <iostream>
#include <map>

#include <vscp.h>
#include <vscphelper.h>

// permission bits for an object (variables)
// uuugggooo
// uuu = user(owner) (rwx)
// ggg = group       (rwx)  // Currently not used
// ooo = other       (rwx)
// First user rights is checked. If user have rights to do the operation it is
// allowed. If not group rights are checked and if the user is member of a group
// that is allowed to do the operation it is allowed. Last other rights are
// checked and if the the other rights allow the user to do the operation it is
// allowed.

// User rights bit array
// "admin" has all rights.
// "user" standard user rights
// "driver" can send and receive events and log in to tcp/ip through local host

// Rights byte 7
#define VSCP_USER_RIGHT_ALLOW_RESTART 0x80000000

// Rights byte 6
#define VSCP_USER_RIGHT_ALLOW_VARIABLE_SAVE 0x10000000

// Rights byte 5
#define VSCP_USER_RIGHT_ALLOW_DM_EDIT 0x08000000
#define VSCP_USER_RIGHT_ALLOW_DM_LOAD 0x04000000
#define VSCP_USER_RIGHT_ALLOW_DM_SAVE 0x02000000
// Undefined

#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_LOAD 0x00800000
#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_UNLOAD 0x00400000

// Rights byte 4

// Rights byte 3

// Rights byte 2
#define VSCP_USER_RIGHT_ALLOW_SEND_EVENT 0x00010000

// Rights byte 1
#define VSCP_USER_RIGHT_ALLOW_UDP 0x00000200
#define VSCP_USER_RIGHT_ALLOW_COAP 0x00000100

// Rights byte 0
#define VSCP_USER_RIGHT_ALLOW_MQTT 0x00000080
#define VSCP_USER_RIGHT_ALLOW_WEB 0x00000040
#define VSCP_USER_RIGHT_ALLOW_WEBSOCKET 0x00000020
#define VSCP_USER_RIGHT_ALLOW_TCPIP 0x00000010

// Some interfaces has a privilege level for commands
// Higher privileges is "better"
#define VSCP_USER_RIGHT_PRIORITY3 0x00000008
#define VSCP_USER_RIGHT_PRIORITY2 0x00000004
#define VSCP_USER_RIGHT_PRIORITY1 0x00000002
#define VSCP_USER_RIGHT_PRIORITY0 0x00000001

#define VSCP_ADD_USER_UNINITIALISED -1

// Local users have an id below this value
#define VSCP_LOCAL_USER_OFFSET 0x10000

// Add user flags
#define VSCP_ADD_USER_FLAG_LOCAL 0x00000001 // local user

// Users not in db is local
//#define USER_IS_LOCAL                                -1           // Never
// saved to db #define USER_IS_UNSAVED                               0 // Should
// be saved to db

#define USER_PRIVILEGE_MASK 0x0f
#define USER_PRIVILEGE_BYTES 8

#define USER_ID_ADMIN 0x00 // The one and only admin user

class CGroupItem
{

  public:
    /// Constructor
    CGroupItem(void);

    /// Destructor
    virtual ~CGroupItem(void);

  private:
};

class CUserItem
{

  public:
    /// Constructor
    CUserItem(void);

    /// Destructor
    virtual ~CUserItem(void);

    /*!
     *  Make sure user name is a valid name
     */
    void fixName(void);

    /*!
        Check if a remote client is allowed to connect.
        First full ip address is checked against hash set (a.b.c.d)
        Next LSB byte is replaced with a star and tested. (a.b.c.*)
        Next the lsb-1 is also replaced with a star and tested. (a.b.*.*)
        Last the lsb-2 is replaced with a star and tested.(a.*.*.*)
        @param remote ip-address for remote machine.
        @return true if the remote machine is allowed to connect.
    */
    // bool isAllowedToConnect( const std::string& remote);

    /*
        IP ACL. By default, empty or non defined, meaning all IPs are allowed to
        connect.

        An Access Control List (ACL) allows restrictions to be put on the list
        of IP addresses which have access to the web server. The ACL is a comma
        separated list of IP subnets, where each subnet is pre-pended by either
        a - or a + sign. A plus sign means allow, where a minus sign means deny.
        If a subnet mask is omitted, such as -1.2.3.4, this means to deny only
        that single IP address.

        Subnet masks may vary from 0 to 32, inclusive. The default setting is to
        allow all accesses. On each request the full list is traversed, and the
        last match wins. Examples:

        "-0.0.0.0/0,+192.168/16" deny all accesses, only allow 192.168/16 subnet

        Verify given socket address against the ACL.

        @param remote_ip Remote ip address on network order.
        @return -1 if ACL is malformed, 0 if address is disallowed, 1 if
       allowed.

     */
    int isAllowedToConnect(uint32_t remote_ip);

    /*!
        Check if use is allowed to send event.
        First check "*.*"
        Next check "class:type"
        Next check "class:*"
        @param vscp_class VSCP class to test.
        @param vscp_type VSCP type to test.
        @return true if the client is allowed to send event.
        */
    bool isUserAllowedToSendEvent(const uint32_t vscp_class,
                                  const uint32_t vscp_type);

    /*!
     * Set user rights from a comma separated string. The string can have
     * up to eight comma separated bit field octets.
     *
     * As an alternative one can use mnemonics
     *
     * admin    - user get full access.
     * user     - user get standard user rights.
     * driver   - user get standard driver rights.
     */
    bool setUserRightsFromString(const std::string &strRights);

    /*!
     * Set allowed remote addresses for string
     * Comma separated list if IP v4 or IP v6 addresses. Wildcards can be used
     * on any position ('*').
     */
    bool setAllowedRemotesFromString(const std::string &strConnect);

    /*!
     * Save record to database
     * @return true on success.
     */
    bool saveToDatabase(void);

    /*!
     * Read record to database
     * @return true on success.
     */
    bool readBackIndexFromDatabase(void);

    /*!
     * Check if a user is with a specific userid is available in the db
     * @param userid userid to look for
     * @return true on success
     */
    static bool isUserInDB(const unsigned long userid);

    /*!
     * Check if a user is with a specific username is available in the db
     * @param user username to look for.
     * @param pid  Optional Pointer to integer that receives id for the record
     * if the user is found.
     * @return true on success
     */
    static bool isUserInDB(const std::string &user, long *pid = NULL);

    /*!
        Check password for user
        @param password Password to check
        @return true If password is correct
    */
    bool checkPassword(const std::string &password)
    {
        return (getPassword() == password);
    };

    /*!
        Check combined password domain for user
        @param passworddomain Password domain to check
        @return true If password domain is correct
    */
    bool checkPasswordDomain(const std::string &md5password)
    {
        return (getPasswordDomain() == md5password);
    };

    // * * * Getters/Setters * * *

    // UserID
    long getUserID(void) { return m_userID; };
    void setUserID(const long id) { m_userID = id; };

    // Username
    std::string getUserName(void) { return m_user; };
    void setUserName(const std::string &strUser) { m_user = strUser; };

    // Password
    std::string getPassword(void) { return m_password; };
    void setPassword(const std::string &strPassword)
    {
        m_password = strPassword;
    };

    // PasswordDomain
    std::string getPasswordDomain(void)
    {
        return vscp_lower(m_md5PasswordDomain);
    };

    void setPasswordDomain(const std::string &strPassword)
    {
        m_md5PasswordDomain = vscp_lower(strPassword);
    };

    // Full name
    std::string getFullname(void) { return m_fullName; };
    void setFullname(const std::string &strUser) { m_fullName = strUser; };

    // Note
    std::string getNote(void) { return m_note; };
    void setNote(const std::string &note) { m_note = note; };

    // User rights
    uint8_t getUserRights(const uint8_t pos)
    {
        return m_userRights[pos & 0x07];
    };
    void setUserRights(const uint8_t pos, const uint8_t right)
    {
        m_userRights[pos & 0x07] = right;
    };
    std::string getUserRightsAsString(void);

    // -------------------------------- 
    // * * * Allowed events list * * *
    // -------------------------------- 

    /*!
        Clear the allowed event list
    */
    void clearAllowedEventList(void) { m_listAllowedEvents.clear(); };

    /*!
        Add one allowed event
        @param strEvent Event to add on the form "class:type" where
        any of class/type can be a wildcard '*'
    */
    bool addAllowedEvent(const std::string &strEvent);

    /*!
        Get allowed event

        @param n Ordinal for event.
        @param event [out] Allowed event
        @return True on success, false on failure
    */
    bool getAllowedEvent(size_t n, std::string& event );

    /*!
        Set allowed event

        @param n Ordinal for event.
        @param event Allowed event if success.
        @return True on success, false on failure
    */
    bool setAllowedEvent(size_t n, std::string& event );

    /*!
        Get all allowed events as a comma separated string
    */
    std::string getAllowedEventsAsString(void);

    /*!
     * Set allowed event for string.
     * @param strEvents Comma separated string where each item is "class:type"
     * where either class or type or both can be a wildcard '*'
     * @param bClear If true clear the list before adding new entries.
     * @return true on succes, false otherwise
     */
    bool setAllowedEventsFromString(const std::string &strEvents,
                                    bool bClear = true);

    /*!
        Get Number of allowed events a user can send. Zero events returned means
       user can send any event.
        @return Number of events
    */
    size_t getAllowedEventsCount(void) { return m_listAllowedEvents.size(); };

    // -----------------------
    // * * * Remote list * * *
    // -----------------------

    /*!
        Return total number of allowed hosts. An empty list means
       all remote hosts can connect.
        @return Number of allowed hosts
    */
    size_t getAllowedRemotesCount(void)
    {
        return (m_listAllowedRemotes.size());
    };

    /*!
        Clear allowed hosts list. An empty list means
       all remote hosts can connect.
    */
    void clearAllowedRemoteList(void)
    {
        m_listAllowedRemotes.clear();
    };

    /*!
        Add allowed remote
        @param strRemote Address (ip.v4/ip.v6) for remote host to add
    */
    void addAllowedRemote(const std::string &strRemote)
    {
        m_listAllowedRemotes.push_back(strRemote);
    };

    /*!
        Get all allowed remotes as a comma separated string. An empty list means
       all remote hosts can connect.
        @return Return list of allowed remotes as a comma separated list of
       strings.
    */
    std::string getAllowedRemotesAsString(void);

    /*!
        Get specific remote host item
        @param n Item index
        @param remote [out] Remote host on success
        @return true on success
    */
    bool getAllowedRemote( size_t n, std::string& remote );

    /*!
        Set specific remote host item
        @param n Item index
        @param remote Remote host to set
        @return true on success
    */
    bool setAllowedRemote( size_t n, std::string& remote );

    /*!
        Get receive filter
    */
    const vscpEventFilter *getUserFilter(void) { return &m_filterVSCP; };

    /*!
        Set receive filter
    */
    void setFilter(const vscpEventFilter *pFilter)
    {
        if (NULL != pFilter)
            memcpy(&m_filterVSCP, pFilter, sizeof(vscpEventFilter));
    };

    /*!
        Set receive filter from string
    */
    bool setFilterFromString(const std::string &strFilter)
    {
        return vscp_readFilterFromString(&m_filterVSCP, strFilter);
    };

    /*!
        Set receive mask from string
    */
    bool setMaskFromString(const std::string &strMask)
    {
        return vscp_readMaskFromString(&m_filterVSCP, strMask);
    };

    std::string getFilter(void)
    {
        std::string str;
        vscp_writeFilterToString(&m_filterVSCP, str);
        return str;
    };

    std::string getMask(void)
    {
        std::string str;
        vscp_writeMaskToString(&m_filterVSCP, str);
        return str;
    };

    /*!
     * Set user settings from string
     * @param userSettings User settings in a semicolon separated list in the
     * form. leave fields blank if they should not be updated.
     * @return true on success, false on failure.
     */
    bool setFromString(const std::string &userSettings);

    /*!
     * Get user settings as string
     * @param userSettings String that will get user settings
     * @return true on success, false on failure.
     */
    bool getAsString(std::string &userSettings);

  protected:
    // System assigned ID for user (-1 -  for system users (not in DB), 0 for
    // admin user )
    long m_userID;

    /// Username
    std::string m_user;

    /// Password
    std::string m_password;

    /// MD5 of user:domain:password (h1)
    std::string m_md5PasswordDomain;

    /// Full name
    std::string m_fullName;

    /// note
    std::string m_note;

    /*!
        Bit array (64-bits) with user rights i.e. tells what
        this user is allowed to do.
    */
    uint8_t m_userRights[USER_PRIVILEGE_BYTES];

    /*!
        This list holds allowed events that user can send.
        Wildcards can be used and the default is all events
        allowed. Form is class:type where either or both of
        class and type can use wildcard '*'
    */
    std::deque<std::string> m_listAllowedEvents;

    /*!
        This list holds ip-addresses for remote
        computers that are allowed to connect to this
        machine. ip.v4 and ip.v6 on standard form.
    */
    std::deque<std::string> m_listAllowedRemotes;

    /*!
        Filter associated with this user
    */
    vscpEventFilter m_filterVSCP;
};

class CUserList
{
  public:
    /// Constructor
    CUserList(void);

    /// Destructor
    virtual ~CUserList(void);

    /*!
     * Load users from database
     * @return true on success
     */
    bool loadUsers(void);

    /*!
     * Add the super (admin) user. This can only be the user setup in the
     * configuration file..
     * @param user Username for user.
     * @param password Password.
     * @param List of allowed remote locations from which the
     *          super user is allowed to connect to this system. If empty
     *          the super user can connect form all remote locations.
     * @param bSystemUser If true this user is a user that should not be saved
     to the DB
       @return true on success. false on failure.
     */
    bool addSuperUser(const std::string &user,
                      const std::string &password,
                      const std::string &allowedRemotes = "",
                      uint32_t bFlags                   = 0);

    /*!
        Add a user to the in memory list. Must saved to database to make
       persistent. The configuration set username is not a valid username.
        @param user Username for user.
        @param password Password.
        @param fullname Fullname for user.
        @param note An arbitrary note about the user
        @param Pointer to a VSCP filter associated with this user.
        @param userRights list with user rights on the form
       right1,right2,right3.... admin - all rights user - standard user rights
                    or an unsigned long value
        @param allowedRemotes List with remote computers that are allowed to
       connect. Empty list is no restrictions.
        @param allowedEvents List with allowed events that a remote user is
       allowed to send.
        @param bSystemUser If true this user is a user that should not be saved
       to the DB
        @return true on success. false on failure.
    */
    bool addUser(const std::string &user,
                 const std::string &password,
                 const std::string &fullname,
                 const std::string &strNote,
                 const vscpEventFilter *pFilter    = NULL,
                 const std::string &userRights     = "",
                 const std::string &allowedRemotes = "",
                 const std::string &allowedEvents  = "",
                 uint32_t bFlags                   = 0);

    /*!
     * Add user from comma separated string data
     * @param strUser Comma separated list with user information.
     *      name;password;fullname;filtermask;rights;remotes;events;note
     * @return true on success. false on failure.
     */
    bool addUser(const std::string &strUser, bool bUnpackNote = false);

    /*!
     * Delete a user given it's username.
     * Only non-system users can be deleted.
     * @param user Username for user to delete
     * @param true on success. false on failure.
     */
    bool deleteUser(const std::string &user);

    /*!
     * Delete a user given it's userid.
     * Only non-system users can be deleted.
     * @param userid Userid for user to delete
     * @param true on success. false on failure.
     */
    bool deleteUser(const long userid);

    /*!
        Get user
        @param user Username
        @return Pointer to user if available else NULL
    */
    CUserItem *getUser(const std::string &user);

    /*!
        Get user
        @param luserid Index for user in database.
                link_to_user == 0 is admin user
                -1 == unknown user
        @return Pointer to user if available else NULL
    */
    CUserItem *getUser(const long userid);

    /*!
        Validate a username/password pair
        @param user Username to test.
        @param password Password to test
        @return Pointer to useritem if valid, NULL if not.
    */
    CUserItem *validateUser(const std::string &user,
                            const std::string &password);

    /*!
        Validate a username using the user domain. (WEB/WEBSOCKETS)
        @param user Username to test.
        @param md5password MD5(user;domain;password)
        @return Pointer to useritem if valid, NULL if not.
    */
    CUserItem *validateUserDomain(const std::string &user,
                                  const std::string &md5password);

    /*!
     * Get number of users on the system
     * @return number of users.
     */
    uint32_t getUserCount(void) { return m_userhashmap.size(); };

    /*!
     * Get user info as string
     * Format is:
     *   userid;name;password;fullname;filter;mask;rights;remotes;events;note
     *   note is always BINHEX64 coded.
     * @param pUserItem Pointer to user.
     * @param strUser String that will receive information
     * @return true on success.
     */
    bool getUserAsString(CUserItem *pUserItem, std::string &strUser);

    /*!
     * Get user information on string form from user index
     * @param idx Index to user in storage array.
     * @param strUser String that will receive information
     * @return true on success.
     */
    bool getUserAsString(uint32_t idx, std::string &strUser);

    /*!
     * Fetch all users in semicolon separated string form ready for transfer.
     *
     * Format is:
     *   userid;name;password;fullname;filter;mask;rights;remotes;events;note
     *   note is always BINHEX64 coded.
     *
     * @param  strAllUser String containing user records in string form.
     */
    bool getAllUsers(std::string &strAllusers);

    /*!
     * Fetch all users into a string array
     * @param arrayUsers Am array with all usernames
     * @return true on success.
     */
    bool getAllUsers(std::deque<std::string> &arrayUsers);

    /*!
     * Get user item from ordinal in user array
     * @param idx Index into user array.
     * @return A pointer to the useritem at that position or NULL if no item is
     *          at that position.
     */
    CUserItem *getUserItemFromOrdinal(uint32_t idx);

  protected:
    /*!
        hash with user items
    */
    std::map<std::string, CUserItem *> m_userhashmap;

    /*!
        hash with group items
    */
    std::map<std::string, CGroupItem *> m_grouphashmap;

  private:
    unsigned short m_cntLocaluser; // Counter for local user id's
};

#endif
