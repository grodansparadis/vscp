// userlist.h
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


#if !defined(USERLIST__INCLUDED_)
#define USERLIST__INCLUDED_

#include <wx/hashset.h>
#include <wx/socket.h>

#include <dllist.h>
#include <vscphelper.h>
#include <vscp.h>

// permission bits for an object (variables)
// uuugggooo
// uuu = user(owner) (rwx)
// ggg = group       (rwx)  // Currently not used
// ooo = other       (rwx)
// First user rights is checked. If user have rights to do the operation it is 
// allowed. If not group rights are checked and if the user is member of a group
// that is allowed to do the operation it is allowed. Last other rights are checked
// and if the the other rights allow the user to do the operation it is allowed.

// User rights bit array
// "admin" has all rights.
// "user" standard user rights
// "driver" can send and receive events and log in to tcp/ip through local host

// Rights byte 7
#define VSCP_USER_RIGHT_ALLOW_RESTART                   0x80000000

// Rights byte 6
#define VSCP_USER_RIGHT_ALLOW_VARIABLE_SAVE             0x10000000

// Rights byte 5
#define VSCP_USER_RIGHT_ALLOW_DM_EDIT                   0x08000000
#define VSCP_USER_RIGHT_ALLOW_DM_LOAD                   0x04000000
#define VSCP_USER_RIGHT_ALLOW_DM_SAVE                   0x02000000
// Undefined

#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_LOAD             0x00800000
#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_UNLOAD           0x00400000

// Rights byte 4

// Rights byte 3

// Rights byte 2
#define VSCP_USER_RIGHT_ALLOW_SEND_EVENT                0x00010000

// Rights byte 1
#define VSCP_USER_RIGHT_ALLOW_UDP                       0x00000200
#define VSCP_USER_RIGHT_ALLOW_COAP                      0x00000100

// Rights byte 0
#define VSCP_USER_RIGHT_ALLOW_MQTT                      0x00000080
#define VSCP_USER_RIGHT_ALLOW_WEB                       0x00000040
#define VSCP_USER_RIGHT_ALLOW_WEBSOCKET                 0x00000020
#define VSCP_USER_RIGHT_ALLOW_TCPIP                     0x00000010

// Some interfaces has a privilege level for commands 
// Higher privileges is "better"
#define VSCP_USER_RIGHT_PRIORITY3                       0x00000008
#define VSCP_USER_RIGHT_PRIORITY2                       0x00000004
#define VSCP_USER_RIGHT_PRIORITY1                       0x00000002
#define VSCP_USER_RIGHT_PRIORITY0                       0x00000001

#define VSCP_ADD_USER_UNINITIALISED                     -1

// Local users have an id below this value
#define VSCP_LOCAL_USER_OFFSET                          0x10000

// Add user flags
#define VSCP_ADD_USER_FLAG_LOCAL                        0x00000001  // local user

// Users not in db is local
//#define USER_IS_LOCAL                                -1           // Never saved to db
//#define USER_IS_UNSAVED                               0           // Should be saved to db

#define USER_PRIVILEGE_MASK                             0x0f
#define USER_PRIVILEGE_BYTES                            8

#define USER_ID_ADMIN                                   0x00        // The one and only admin user

class CGroupItem {
    
public:

    /// Constructor
    CGroupItem(void);

    /// Destructor
    virtual ~CGroupItem(void);
    
    
private:
    
};


// hash table for groups
WX_DECLARE_HASH_MAP(wxString, CGroupItem*, wxStringHash, wxStringEqual, VSCPGROUPHASH);


class CUserItem {
    
public:

    /// Constructor
    CUserItem(void);

    /// Destructor
    virtual ~CUserItem(void);
    
    /*!
     *  Make sure user name is a valid name      
     */
    void fixName( void );

    /*!
        Check if a remote client is allowed to connect.
        First full ip address is checked against hash set (a.b.c.d)
        Next LSB byte is replaced with a star and tested. (a.b.c.*)
        Next the lsb-1 is also replaced with a star and tested. (a.b.*.*)
        Last the lsb-2 is replaced with a star and tested.(a.*.*.*)
        @param remote ip-address for remote machine.
        @return true if the remote machine is allowed to connect.
    */
    //bool isAllowedToConnect( const wxString& remote);
    
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
        @return -1 if ACL is malformed, 0 if address is disallowed, 1 if allowed.
     
     */
    int isAllowedToConnect( uint32_t remote_ip );

    /*!
        Check if use is allowed to send event.
        First check "*.*"
        Next check "class:type"
        Next check "class:*"
        @param vscp_class VSCP class to test.
        @param vscp_type VSCP type to test.
        @return true if the client is allowed to send event.
        */
    bool isUserAllowedToSendEvent( const uint32_t vscp_class,
                                    const uint32_t vscp_type);
    
    
    /*!
     * Set user rights from a comma separated string. The string can have 
     * up to eight comma separated bit field octets.  
     * 
     * As an alternative one can use mnenomics
     * 
     * admin    - user get full access.
     * user     - user get standard user rights.
     * driver   - user get standard driver rights.
     */
    bool setUserRightsFromString( const wxString& strRights );
    
    /*!
     * Set allowed remote addresses for string
     * Comma separated list if IP v4 or IP v6 addresses. Wildcards can be used
     * on any position ('*').
     */
    bool setAllowedRemotesFromString( const wxString& strConnect );
    
    /*!
     * Set allowed event for string.
     * Comma separated string where each item is class:type where either
     * class or type or both can be wildcard '*'
     */
    bool setAllowedEventsFromString( const wxString& strEvents );
    
    /*!
     * Save record to database
     * @return true on success.
     */
    bool saveToDatabase( void );
    
    /*!
     * Read record to database
     * @return true on success.
     */
    bool readBackIndexFromDatabase( void );
    
    /*!
     * Check if a user is with a specific userid is available in the db
     * @param userid userid to look for
     * @return true on success
     */
    static bool isUserInDB(const unsigned long userid );
    
    /*!
     * Check if a user is with a specific username is available in the db
     * @param user username to look for.
     * @param pid  Optional Pointer to integer that receives id for the record if
     *              the user is found.
     * @return true on success
     */
    static bool isUserInDB(const wxString& user, long *pid = NULL );
    
    bool checkPassword( const wxString& password ) { return ( getPassword().IsSameAs( password ) ? true : false); };
    bool checkPasswordDomain( const wxString& md5password ) { return ( getPasswordDomain().IsSameAs( md5password ) ? true : false); };
    
    // Getters/Setters
    long getUserID( void ) { return m_userID; };
    void setUserID( const long id ) { m_userID = id; };
    
    wxString getUserName( void ) { return m_user; };
    void setUserName( const wxString& strUser ) { m_user = strUser; };
    
    wxString getPassword( void ) { return m_password; };
    void setPassword( const wxString& strPassword ) { m_password = strPassword; };
    
    wxString getPasswordDomain( void ) { return m_md5PasswordDomain.Lower(); };
    void setPasswordDomain( const wxString& strPassword ) 
                                    { m_md5PasswordDomain = strPassword.Lower(); };
    
    wxString getFullname( void ) { return m_fullName; };
    void setFullname( const wxString& strUser ) { m_fullName = strUser; };
    
    wxString getNote( void ) { return m_note; };
    void setNote( const wxString& note ) { m_note = note; };
    
    uint8_t getUserRights( const uint8_t pos ) { return m_userRights[ pos & 0x07 ]; };
    void setUserRights( const uint8_t pos, const uint8_t right ) { m_userRights[ pos & 0x07 ] = right; };
    wxString getUserRightsAsString( void );
    
    void clearAllowedEventList( void ) { m_listAllowedEvents.Clear(); };
    void addAllowedEvent( const wxString& strEvent ) { m_listAllowedEvents.Add( strEvent ); };
    wxString getAllowedEventsAsString( void );
    
    void clearAllowedRemoteList( void ) { m_listAllowedIPV4Remotes.Clear(); m_listAllowedIPV6Remotes.Clear(); };
    void addAllowedRemote( const wxString& strRemote ) { if ( wxNOT_FOUND != strRemote.Find(':') ) m_listAllowedIPV6Remotes.Add( strRemote ); else m_listAllowedIPV4Remotes.Add( strRemote ); };     
    wxString getAllowedRemotesAsString( void );
 
    const vscpEventFilter *getFilter( void ) { return &m_filterVSCP; };
    void setFilter( const vscpEventFilter * pFilter ) { if ( NULL != pFilter ) memcpy( &m_filterVSCP, 
                                                                    pFilter,
                                                                    sizeof( vscpEventFilter ) ); };
    bool setFilterFromString( wxString strFilter ) { return vscp_readFilterFromString( &m_filterVSCP, strFilter ); };
    bool setMaskFromString( wxString strMask ) { return vscp_readMaskFromString( &m_filterVSCP, strMask ); };
    
    /*!
     * Set user settings from string
     * @param userSettings User settings in a semicolon separated list in the form. 
     *          leave fields blank if they should not be updated.
     * @return true on success, false on failure.
     */
    bool setFromString( wxString userSettings );
    
    /*!
     * Get user settings as string
     * @param userSettings String that will get user settings
     * @return true on success, false on failure.
     */
    bool getAsString( wxString& userSettings );
    
protected:
    
    // System assigned ID for user (-1 -  for system users (not in DB), 0 for adḿin user )
    long m_userID;
    
    /// Username
    wxString m_user;
    
    /// Password
    wxString m_password;
    
    /// MD5 of user:domain:password (h1)
    wxString m_md5PasswordDomain;
    
    /// Full name
    wxString m_fullName;
    
    /// note
    wxString m_note;
    
    /*!
        Bit array (64-bits) with user rights i.e. tells what
        this user is allowed to do.
    */ 
    uint8_t m_userRights[ USER_PRIVILEGE_BYTES ];
    
    /*!
        This list holds allowed events that user can send.
        Wildcards can be used and the default is all events 
        allowed. Form is class:type where either or both of
        class and type can use wildcard '*'
    */
    wxArrayString m_listAllowedEvents;
    
    /*!
        This list holds ip-addresses for remote
        computers that are allowed to connect to this
        machine. IP v4 and IP v6 on standard form.
    */
    wxArrayString m_listAllowedIPV4Remotes;
    wxArrayString m_listAllowedIPV6Remotes;
    
    
    
    /*!
        Filter associated with this user
    */
    vscpEventFilter m_filterVSCP;

};

// hash table for users
WX_DECLARE_HASH_MAP(wxString, CUserItem*, wxStringHash, wxStringEqual, VSCPUSERHASH);


class CUserList {
public:

    /// Constructor
    CUserList(void);

    /// Destructor
    virtual ~CUserList(void);
    
    /*!
     * Load users from database
     * @return true on success
     */
    bool loadUsers( void );

    /*!
     * Add the super (admin) user. This can only be the user setup in the
     * configuration file..
     * @param user Username for user.
     * @param password Password.
     * @param List of allowed remote locations from which the
     *          super user is allowed to connect to this system. If empty
     *          the super user can connect form all remote locations.
     * @param bSystemUser If true this user is a user that should not be saved to the DB
       @return true on success. false on failure.
     */
    bool addSuperUser( const wxString& user,
                            const wxString& password,
                            const wxString& allowedRemotes = _(""),
                            uint32_t bFlags = 0 );
    
    /*!
        Add a user to the in memory list. Must saved to database to make persistent.
        The configuration set username is not a valid username.
        @param user Username for user.
        @param password Password.
        @param fullname Fullname for user.
        @param note An arbitrary note about the user
        @param Pointer to a VSCP filter associated with this user.
        @param userRights list with user rights on the form right1,right2,right3....
                    admin - all rights
                    user - standard user rights
                    or an unsigned long value
        @param allowedRemotes List with remote computers that are allowed to connect. 
                    Empty list is no restrictions.			
        @param allowedEvents List with allowed events that a remote user is allowed
                    to send.
        @param bSystemUser If true this user is a user that should not be saved to the DB
        @return true on success. false on failure.	
    */
    bool addUser( const wxString& user,
                            const wxString& password,
                            const wxString& fullname,
                            const wxString& strNote,
                            const vscpEventFilter *pFilter = NULL,
                            const wxString& userRights = _(""),
                            const wxString& allowedRemotes = _(""),
                            const wxString& allowedEvents = _(""),
                            uint32_t bFlags = 0 );

    /*!
     * Add user from comma separated string data
     * @param strUser Comma separated list with user information. 
     *      name;password;fullname;filtermask;rights;remotes;events;note
     * @return true on success. false on failure.
     */
    bool addUser( const wxString& strUser, bool bUnpackNote = false );
    
    /*!
     * Delete a user given it's userid. 
     * Only non-system users can be deleted.
     * @param users Username for user to delete
     * @param true on success. false on failure.
     */
    bool deleteUser( const wxString& user );
    
    /*!
        Get user 
        @param user Username
        @return Pointer to user if available else NULL
    */
    CUserItem *getUser( const wxString& user );
    
    /*!
        Get user 
        @param luserid Index for user in database. 
                link_to_user == 0 is admin user
                -1 == unknown user
        @return Pointer to user if available else NULL
    */
    CUserItem *getUser( const long userid );

    /*!
        Validate a username/password pair
        @param user Username to test.
        @param password Password to test
        @return Pointer to useritem if valid, NULL if not.
    */
    CUserItem *validateUser( const wxString& user, const wxString& password );

    /*!
        Validate a username using the user domain. (WEB/WEBSOCKETS)
        @param user Username to test.
        @param md5password MD5(user;domain;password)
        @return Pointer to useritem if valid, NULL if not.
    */
    CUserItem *validateUserDomain( const wxString& user, const wxString& md5password );
    
    /*!
     * Get number of users on the system
     * @return number of users.
     */
    uint32_t getUserCount( void ) { return m_userhashmap.size(); };
    
    /*!
     * Get user info as string
     * Format is:
     *   userid;name;password;fullname;filter;mask;rights;remotes;events;note
     *   note is always BINHEX64 coded.
     * @param pUserItem Pointer to user.
     * @param strUser String that will receive information
     * @return true on success.
     */
    bool getUserAsString( CUserItem *pUserItem, wxString& strUser );
    
    /*!
     * Get user information on string form from user index
     * @param idx Index to user in storage array.
     * @param strUser String that will receive information
     * @return true on success.
     */
    bool getUserAsString( uint32_t idx, wxString& strUser );
    
    /*!
     * Fetch all users in semicolon separated string form ready for transfer.
     * 
     * Format is:
     *   userid;name;password;fullname;filter;mask;rights;remotes;events;note
     *   note is always BINHEX64 coded.
     * 
     * @param  strAllUser String containing user records in string form.
     */
    bool getAllUsers( wxString& strAllusers ); 
    
    /*!
     * Fetch all users into a string array
     * @param arrayUsers Am array with all usernames
     * @return true on success.
     */
    bool getAllUsers( wxArrayString& arrayUsers );
    
    /*!
     * Get user item from ordinal in user array
     * @param idx Index into user array.
     * @return A pointer to the useritem at that position or NULL if no item is
     *          at that position.
     */
    CUserItem *getUserItemFromOrdinal( uint32_t idx );

protected:

    /*!
        hash with user items
    */
    VSCPUSERHASH m_userhashmap;
    
    /*!
        hash with group items
    */
    VSCPGROUPHASH m_grouphashmap;
    
private:

    unsigned short m_cntLocaluser;  // Counter for local user id's

};



#endif


