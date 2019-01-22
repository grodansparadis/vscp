// vscpvariable.h
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

//-----------------------------------------------------------------------------
// REMEMBER !!!
// ============
// Remote variables is displayed in clear text but are transferred BASE64
// encoded. That is XML files,tcp/ip links, websocket links etc use BASE64
// encoded values and notes. However they are stored in clear text in the
// database.
//-----------------------------------------------------------------------------

// VSCP daemon variable codes are defined in this file

#if !defined(VSCP_REMOTE_VARIABLE__INCLUDED_)
#define VSCP_REMOTE_VARIABLE__INCLUDED_

#include <guid.h>
#include <sqlite3.h>
#include <userlist.h>
#include <variablecodes.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

#define VAR_MAXBUF_SIZE 0x10000 // Size for variable strings etc

// Variable read/write error codes
#define VAR_ERROR_OK (0)             // Positive response
#define VAR_ERROR_USER (-1)          // User is not allowed to do this operation
#define VAR_ERROR_PERMISSION (-2)    // RWX permission not right to do operation
#define VAR_ERROR_TYPE_MISMATCH (-3) // Wrong variable type for operation
#define VAR_ERROR_SYNTAX (-4)        // Syntax or format is wrong

// Persistence flags
#define VSCP_VAR_PERSISTENT true
#define VSCP_VAR_NON_PERISTENT false

// Permissions
#define PERMISSION_OWNER_READ 0x400
#define PERMISSION_OWNER_WRITE 0x200
#define PERMISSION_OWNER_EXECUTE 0x100
#define PERMISSION_GROUP_READ 0x040    // Not used
#define PERMISSION_GROUP_WRITE 0x020   // Not used
#define PERMISSION_GROUP_EXECUTE 0x010 // Not used
#define PERMISSION_OTHER_READ 0x004
#define PERMISSION_OTHER_WRITE 0x002
#define PERMISSION_OTHER_EXECUTE 0x001

#define PERMISSION_OWNER_ALL 0x700
#define PERMISSION_OWNER_NONE 0x000
#define PERMISSION_GROUP_ALL 0x070  // Not used
#define PERMISSION_GROUP_NONE 0x000 // Not used
#define PERMISSION_OTHER_ALL 0x007
#define PERMISSION_OTHER_NONE 0x000

#define PERMISSION_ALL_READ 0x444
#define PERMISSION_ALL_WRITE 0x222

#define PERMISSION_ALL_RIGHTS 0x777
#define PERMISSION_NO_RIGHTS 0x000
#define PERMSSION_VARIABLE_DEFAULT 0x744

// A dynamic variable is a variable that is not defined
// in any of the databases during init but instead is
// constructed on read.
#define ID_VAR_DYNAMIC UINT32_MAX

// Table types - bit-field
#define VARIABLE_STOCK 0x01
#define VARIABLE_INTERNAL 0x02
#define VARIABLE_EXTERNAL 0x04

// Forward declarations

// name,type,user,rights,persistence,last,bnumerical,bbase64,value,note
//{
//	"name": "variable-name",
//	"type": 1,
//	"user": 2,
//	"accessrights": 777,
//      "persistence": true|false,
//	"lastchange": "YYYYMMDDTHHMMSS",
//      "bnumerical": true|false,
//      "bbase64": true:false,
//	"value": "This is a test variable",
//	"note": "This is a note about this variable"
//}
// value is numerical for a numerical variable else string.

#define VARIABLE_JSON_TEMPLATE                                                 \
    "{\n"                                                                      \
    "\"name\": \"%s\",\n"                                                      \
    "\"type\": %hu,\n"                                                         \
    "\"user\": %lu,\n"                                                         \
    "\"accessrights\": %lu,\n"                                                 \
    "\"persistence\": %s,\n"                                                   \
    "\"lastchange\": \"%s\",\n"                                                \
    "\"bnumeric\": %s,\n"                                                      \
    "\"bbase64\": %s,\n"                                                       \
    "\"value\": %s,\n"                                                         \
    "\"note\": \"%s\"\n"                                                       \
    "}"

#define VARIABLE_XML_TEMPLATE                                                  \
    "<variable \n"                                                             \
    "name=\"%s\" \n"                                                           \
    "type=\"%hu\" \n"                                                          \
    "user=\"%lu\" \n"                                                          \
    "access-rights=\"%lu\" \n"                                                 \
    "persistence=\"%s\" \n"                                                    \
    "last-change=\"%s\" \n"                                                    \
    "value=\"%s\" \n"                                                          \
    "note=\"%s\" />"

class CStockVar
{

  public:
    CStockVar(std::string name);
    ~CStockVar();

    typedef bool (*STOCKVAR_SET)(std::string setString);
    typedef std::string (*STOCKVAR_GET)(void);
};

/*
 * Structure used for multi call variable
 * queries.
 */

struct varQuery
{
    uint8_t tableType;    // Table type: type  1 = Stock, 2=internal, 3=external
    uint32_t stockId;     // ID for next stock variable to return
    sqlite3_stmt *ppStmt; // Handle for SQL query. NULL if stock
};

typedef struct varQuery varQuery;

// Class that holds one VSCP variable
// Persistent variables should have names staring with $

class CVariable
{

  public:
    /// Constructor
    CVariable(void);

    // Destructor
    virtual ~CVariable(void);

    /// Set default values
    void init(void);

    /*!
        Name should not contain special characters so if it does
        we replace them with 'underscore'
    */
    void fixName(void);

    /*!
     * Make an access right string from supplied rights.
     *
     * @param  accessrights Standard access rights
     * @param strAccessRights String that will get text form of
     *          access eights.
     */
    static void makeAccessRightString(uint32_t accessrights,
                                      std::string &strAccessRights);

    /*!
        Get variable type as string
        @param type Variable numerical type
        @return Variable type as a string.
    */
    static const char *getVariableTypeAsString(int type);

    /*!
        Get numerical variable type from symbol or numeric version
        @parm strVariableType Variable type in string form
        @param variable type in numerical form.
     */
    static uint16_t getVariableTypeFromString(
      const std::string &strVariableType);

    /*!
     * Check if the variable is a numerical type
     * @return true if numerical, false otherwise.
     */
    bool isNumerical(void);

    /*!
        Set variable information from a string value
        Format is: "variable
       name";"type";"persistence";"user";"rights";"value";"note" User can be
       numeric (userid) or given as a username. It is valid to leave all fields
        blank ";;" except for variablename
        @param str String that will get string representation of variable with
       all info.
        @param bBase64 If true value will be BASE64 encoded before it is stored.
        @param strUser User name that is used if user field is empty.
        @return true if the string could be parsed correctly.
     */
    bool setFromString(const std::string &strVariable,
                       const std::string &strUser = "admin");

    /*!
     * Get string content as semicolon separated string form.
     * @param bShort
     * If bShort is true
     *    name;type;bPersistent;userid;rights;lastchanged
     * is returned.
     * If not bShort is false
     *    name;type;bPersistent;userid;rights;lastchanged;value;note
     * is returned.
     * @return Variable content in semicolon separated form.
     */
    std::string getAsString(bool bShort = true);

    /*!
     * Get the variable as a JSON object
     * @param strVariable Filled in with JSON data
     * @return true of all is OK, false otherwise.
     */
    bool getAsJSON(std::string &strVariable);

    /*!
     * Set variable information from a JSON object
     *
     * @param strVariable JSON object
     * @return true if the string could be parsed correctly.
     */
    bool setFromJSON(std::string &strVariable);

    /*!
     * Get the variable as a XML object
     * @param strVariable Filled in with JSON data
     * @return true of all is OK, false otherwise.
     */
    bool getAsXML(std::string &strVariable);

    /*!
     * Set variable information from a XML object
     *
     * @param strVariable XML object
     * @return true if the string could be parsed correctly.
     */
    bool setFromXML(std::string &strVariable);

    /*!
        Get the length for the string value of a variable
     */
    size_t getLength(void) { return m_strValue.length(); };

    /*!
        Reset the variable to it's default value
        Set variable to default values
     */
    void reset(void);

    /*!
        isTrue
        @return Return true if the variable is of type bool and is true.
        True is also returned for a numerical varable (int, long, float) if
        it has a non zero value. In all other cases false will be returned.
     */
    bool isTrue(void);

    /*!
        setTrue
        Set a variable to true. A boolean variable will be set to true.
        A numerical variable (int, long, float) is set to -1. A string is set to
       "true".
     */
    void setTrue(void);

    /*!
        setFalse
        Set a variable to false. A boolean variable will be set to false.
        A numerical variable (int, long, float) is set to 0. A string is set to
       "false".
     */
    void setFalse(void);

    /*!
        Set variable value from string
        @param type Type of value
        @param strValue Value in string form
        @param bBase64 True if strValue is BASE64 encoded.
        @return true on success.
    */
    bool setValueFromString(int type,
                            const std::string &strValue,
                            bool bBase64 = false);

    /*!
        Get the variable value as a string value
        @param str String that will get string representation of variable.
        @param  bBase64 If set to true value is encoded in BASE64
     */
    void writeValueToString(std::string &strValue, bool bBase64 = false);

    /*!
        Get string value
        @param bBase64 If true the returned value will be BASE64 encoded.
        @return value in string form is returned regardless of type.
     */
    std::string getValue(bool bBase64 = false);

    /*!
        getValue
        @param pval Pointer to string that will get value.
        @param bBase64 If true the returned value will be BASE64 encoded.
     */
    void getValue(std::string *pval, bool bBase64 = false);

    /*!
        getValue
        @param value String that will receive value.
     */
    bool setValue(const std::string &strValue, bool bBase64 = false);

    /*!
        setValue
        @param val long to set value to.
     */
    void setValue(int val);

    /*!
        getValue
        @param value Int that will receive value.
     */
    void getValue(int *pValue);

    /*!
        setValue
        @param val long to set value to.
     */
    void setValue(long val);

    /*!
        getValue
        @param value Long that will receive value.
     */
    void getValue(long *pValue);

    /*!
        setValue
        @param val double to set value to.
     */
    void setValue(double val);

    /*!
        getValue
        @param value Double that will receive the value.
     */
    void getValue(double *pValue);

    /*!
        setValue
        @param val Boolean to set value to.
     */
    void setValue(bool val);

    /*!
        getValue
        @param value Bool that will receive the value.
     */
    void getValue(bool *pValue);

    /*!
        setValue - cguid
        @param guid GUID to set.
    */
    void setValue(cguid &guid);

    /*!
        setValue - vscpEvent
        @param event VSCP Event to set.
    */
    void setValue(vscpEvent &event);

    /*!
        getValue
        @param value Bool that will receive the value.
     */
    void getValue(vscpEvent *pEvent);

    /*!
        getValue - EventEx
        @param pEventEx [OUT ]VSCP EventEx from variable
     */
    void getValue(vscpEventEx *pEventEx);

    /*!
        setValue - vscpEventEx
        @param event VSCP EventEx to set.
     */
    void setValue(vscpEventEx &eventex);

    /*!
        GetValue - Date + time format
        @param pValue [OUT] Date + time value in ISO format
     */
    void getValue(vscpdatetime *pValue);

    /*!
        SetValue - Date + Time ISO format
        @param val Date + time in ISO format.
     */
    void setValue(vscpdatetime &val);

    /*!
        setUser owner id
        @param Owner id to set
        @return True on success
     */
    bool setOwnerId(uint32_t ownerid);

    /*!
        Set owner id from user name
        @param strUser Name of user
        @return True on success
    */
    bool setOwnerIdFromUserName(std::string &strUser);

    /*!
        Set last changed date/time to now (current local date/time)
     */
    void setLastChangedToNow(void) { m_lastChanged = vscpdatetime::Now(); };

    /*!
     * Get last changed date/time
     * @return Last changed date/time
     */
    vscpdatetime &getLastChange(void) { return m_lastChanged; };

    // * * * Getters/Setters

    /*!
        Set id for variable
        @param id Non zero id to set for variable
    */
    void setID(uint32_t id) { m_id = id; };

    /*!
        Get id for variable
        @return variable id
    */
    uint32_t getID(void) { return m_id; };

    // name

    /*!
        Set variable name
        @param strName Name of variable. Variables with prefix "vscp." are
       reserved for stock variables,
        @return true on success
     */
    bool setName(const std::string &strName);

    /*!
        Get variable name
        @return Name of variable always in upper case.
    */
    std::string getName(void) { return vscp_upper(m_name); };

    // type

    /*!
        Set variable type
        @param strType type in numerical or mnemonic form
        @return True on success.
     */
    bool setType(const std::string &strType);

    /*!
        Set variable type
        @param type Type to set variable to.
        @return true if type is a valid type.
    */
    bool setType(uint16_t type);

    /*!
        Get variable type
        @return variable type;
    */
    uint16_t getType(void) { return m_type; };

    /*!
        Set note for variable
        @param strNote Note to set.
        @param bBase64 If true strNote will be decoded from base64 before it is
       stored.
        @return True on success.
    */
    bool setNote(const std::string &strNote, bool bBase64 = false);

    /*!
        Get variable note
        @param bBase64 If true the note will be returned base64 encoded.
        @return Variable note.
    */
    std::string getNote(bool bBase64 = false);

    /*!
        Get variable note
        @param strNote Get the variable note if call is successfull.
        @param bBase64 If true the note will be returned base64 encoded.
        @return True on success.
    */
    bool getNote(std::string &strNote, bool bBase64 = false);

    /*!
        test if variable is persistent
        @return True if the variable is persistent, false otherwise.
    */
    bool isPersistent(void) { return m_bPersistent; };

    /*!
        Set persistence for variable
        @param b True if variable should be stored in a persistent way, false
       otherwise
    */
    void setPersistent(bool b) { m_bPersistent = b; };

    /*!
        Set access rights
        @param accessRights Rights to set. rw-rw-rw- for owner, group(not-used)
       and other
    */
    void setAccessRights(uint32_t accessRights)
    {
        m_accessRights = accessRights;
    };

    /*!
        Get access rights
        @return access rights.
    */
    uint32_t getAccessRights(void) { return m_accessRights; };

    /*!
        Get accessrights on string format
        @return Accessrights as a string
    */
    std::string getAccessRightStr(void)
    {
        std::string str;
        makeAccessRightString(m_accessRights, str);
        return str;
    };

    /*!
        Test if variable is readable by a general user
        @return Return true if variable is readable by
        a general user, false if not.
    */
    bool isUserReadable(void)
    {
        return (m_accessRights & 0x04) ? true : false;
    };

    /*!
       Make variable writable (or not) by the general user
       @param b True to make the variable writable by the general user. False
      to make it non writable by the general user.
   */
    void makeUserReadable(bool b)
    {
        if (b)
            m_accessRights |= 0x04;
        else
            m_accessRights &= 0xFFFB;
    };

    /*!
        Test if variable is writable by a general user
        @return Return true if variable is writable by a general user, false if
       not.
    */
    bool isUserWritable(void)
    {
        return (m_accessRights & 0x02) ? true : false;
    };

    /*!
        Make variable writable (or not) by the general user
        @param b True to make the variable writable by the general user. False
       to make it non writable by the general user.
    */
    void makeUserWritable(bool b)
    {
        if (b)
            m_accessRights |= 0x40;
        else
            m_accessRights &= 0xFFFD;
    };

    /*!
        Test if variable is executable by a general user
        @return Return true if variable is executable by a general user, false
       if not.
    */
    bool isUserExecutable(void)
    {
        return (m_accessRights & 0x01) ? true : false;
    };

    /*!
        Make variable executable (or not) by the general user
        @param b True to make the variable executable by the general user. False
       to make it non executable by the general user.
    */
    void makeUserExecutable(bool b)
    {
        if (b)
            m_accessRights |= 0x01;
        else
            m_accessRights &= 0xFFFE;
    };

    /*!
        Test if variable is readable by owner
        @return Return true if variable is readable by owner, false if not.
    */
    bool isOwnerReadable(void)
    {
        return (m_accessRights & 0x100) ? true : false;
    };

    /*!
        Make variable readable (or not) by owner
        @param b True to make the variable readable by the owner. False to make
       it non readable by owner.
    */
    void makeOwnerReadable(bool b)
    {
        if (b)
            m_accessRights |= 0x100;
        else
            m_accessRights &= 0xFEFF;
    };

    /*!
        Test if variable is writable by owner
        @return Return true if variable is writable by owner, false if not.
    */
    bool isOwnerWritable(void)
    {
        return (m_accessRights & 0x80) ? true : false;
    };

    /*!
        Make variable writable (or not) by owner
        @param b True to make the variable writable by the owner. False to make
       it non writable by owner.
    */
    void makeOwnerWritable(bool b)
    {
        if (b)
            m_accessRights |= 0x80;
        else
            m_accessRights &= 0xFF7F;
    };

    /*!
        Test if variable is executable by owner
        @return Return true if variable is executable by owner, false if not.
    */
    bool isOwnerExecutable(void)
    {
        return (m_accessRights & 0x40) ? true : false;
    };

    /*!
        Make variable executable (or not) by owner
        @param b True to make the variable executable by the owner. False to
       make it non executable by owner.
    */
    void makeOwnerExecutable(bool b)
    {
        if (b)
            m_accessRights |= 0x40;
        else
            m_accessRights &= 0xFFBF;
    };

    /*!
        Check if user is allowed to read variable. Admin can  ead all.
        Owner and "user" must have write privileges.
        @param pUser Pointer to user to check privileges for. Parameter
        can be set to NULL and if so "general user" privileges is checked.
        @return true of user is allowed to perform operation
    */
    bool isUserAllowRead(CUserItem *pUser);

    /*!
        Check if user is allowed to write variable. Admin can write all.
        Owner and "user" must have write privileges.
        @param pUser Pointer to user to check privileges for. Parameter
        can be set to NULL and if so "general user" privileges is checked.
        @return true of user is allowed to perform operation
    */
    bool isUserAllowWrite(CUserItem *pUser);

    /*!
        Check if user is allowed to execute variable content.
        @param pUser Pointer to user to check privileges for. Parameter
        can be set to NULL and if so "general user" privileges is checked.
        @return true of user is allowed to perform operation
    */
    bool isUserAllowExecute(CUserItem *pUser);

    /*!
        Set owner id for variable
        @param uid Owner if to set. Must be defined.
    */
    void setOwnerID(uint32_t uid) { m_userid = uid; };
    /*!
        Get owner id for variable
        @return Owner if for variable
    */
    uint32_t getOwnerID(void) { return m_userid; };

    /*!
        Set/reset stock property of variable
        @param bStock True to set variable to stock variable, false to set is as
       a standard variable.
    */
    void setStockVariable(bool bStock = true) { m_bStock = bStock; };
    /*!
        Check if a variable is a stock variable
        @return True if variable is stock variable
    */
    bool isStockVariable(void) { return m_bStock; };

    /*!
        Check if a user is allowed to read variable
        @param user Reference to user that wants to access variable
        @return True if user is allowed to read value of variable.
    */
    bool isAllowedToRead(CUserItem &user);

    /*!
        Check if a user is allowed to write variable
        @param user Reference to user that wants to access variable
        @return True if user is allowed to write value of variable.
     */
    bool isAllowedToWrite(CUserItem &pUser);

  private:
    // id in database
    uint32_t m_id;

    // Name of variable
    std::string m_name;

    // type of variable
    uint16_t m_type;

    // Flag to make variable persistent
    bool m_bPersistent;

    // Access right owner(rwx);group(rwx):other(rwx)
    // Usage for other bits is TODO
    uint32_t m_accessRights;

    // User this variable is owned by
    uint32_t m_userid;

    /// Note about variable (always coded in BASE64)
    std::string m_note;

    /*!
        Storage
        A "class" variable is stored in the VSCP event.
        A "type" variable is stored in the VSCP event.
        A "GUID" variable is stored in the VSCP event.
        A VSCP data variables s stored in the VSCP event.
     */
    std::string m_strValue;

    // True if this is a stock variable
    bool m_bStock;

    // Variable hook  TODO
    // Used to get notification in websocket when a variable changed it's value
    // Contains client id's for clients that should be informed if the variable
    // is written to.
    std::list<int> m_clientHooks;

  public:
    // Time when variable was last changed.
    vscpdatetime m_lastChanged;
};

// ----------------------------------------------------------------------------

class CVariableStorage
{
  public:
    /// Constructor
    CVariableStorage();

    /// Destructor
    virtual ~CVariableStorage();

    // Initialise variable storage
    // @return true on success.
    bool init(void);

    /*!
     * Find variable from it's name
     *
     * First stock variables is searched (start with "vscp.", next
     * non-persistent variables (in-memory) is searched, and last persistent
     * variables is searched.
     * @param name Name of variable
     * @param pUser This is a pointer to the user doing the search
     * @param pVar [OUT] If found supplied variable is filled with data.
     * @return >0 if variable is found, zero if not.
     */
    uint32_t find(const std::string &name,
                  CUserItem *pUser,
                  CVariable &variable);

    /*!
     * Check if a variable exist.
     * @param name Name of variable
     * @return id if the variable exists, zero else.
     *
     */
    uint32_t exist(const std::string &name);

    /*!
        Handle init/read/write for stock variables
        @param name Name of variable
        @parm op Operation to perform (INIT/READ/WRITE)
        @param var Variable with init/read/write data
        @param user User that perform the operation
        @return True if operation is performed successfully
    */
    bool handleStockVariable(std::string name,
                             int op,
                             CVariable &var,
                             CUserItem *pUser);

    /*!
        Initialize stock variables that should be visible in lists
        @param user User item used to init variables.
    */
    void initStockVariables(CUserItem *pUser);

    /*!
       Set variable value from Database record.
       @param ppStmt sqlite3 ref to record
       @param variable Variable that will get record data.
       @return true on success.
     */
    bool setVariableFromDbRecord(sqlite3_stmt *ppStmt, CVariable &variable);

    /*!
     * Find a non-persistent variable
     *
     * Find non-persistent variable (in-memory variable).
     * @param name Name of variable
     * @param pUser Pointer to uer doin g the find
     * @param pVar [OUT] If found supplied variable is filled with data. Can be
     * set to NULL in which case only availability of the variable is returned.
     * @return id if variable is found, zero if not.
     */
    uint32_t findNonPersistentVariable(const std::string &name,
                                       CUserItem *pUser,
                                       CVariable &pVar);

    /*!
     * Find a persistent variable
     *
     * Find persistent variable.
     * @param name Name of variable
     * @param pUser Pointer to user doing the find
     * @param pVar [OUT] If found supplied variable is filled with data. Can be
     * set to NULL in which case only availability of the variable is returned.
     * @return id if variable is found, zero if not.
     */
    uint32_t findPersistentVariable(const std::string &name,
                                    CUserItem *pUser,
                                    CVariable &pVar);

    /*!
     * Add stock variable
     * Stock variable is added to internal variables but there
     * values is not read/written from there. They are just in the
     * internal space for sorting/listing.
     * @param var Reference to variable to be added.
     * @return true on success.
     */
    bool addStockVariable(CVariable &var);

    /*!
        Add a variable.
        @param var Variable object.
        @return true on success, false on failure.
     */
    bool add(CVariable &var);

    /*!
        Add a variable.
        @param name Name of variable
        @param value Value for variable
        @param type Type of variable. Defaults to string.
        @param userid Id for user that owns the variable.
        @param bPersistent True if the variable should be saved to
                persistent storage.
        @param accessRights Access rights for the variable
        @return true on success, false on failure.
     */
    bool add(const std::string &name,
             const std::string &value,
             const uint16_t type         = VSCP_DAEMON_VARIABLE_CODE_STRING,
             const uint32_t userid       = USER_ID_ADMIN,
             const bool bPersistent      = false,
             const uint32_t accessRights = PERMISSION_OWNER_ALL,
             const std::string &note     = "");

    // Variant of the above with string as type
    bool add(const std::string &name,
             const std::string &value,
             const std::string &strType,
             const uint32_t userid       = USER_ID_ADMIN,
             const bool bPersistent      = false,
             const uint32_t accessRights = PERMISSION_OWNER_ALL,
             const std::string &note     = "");

    // Update a variable (write to db)
    bool update(CVariable &var, CUserItem *pUser);

    /*!
        Remove named variable
        @param name Name of variable
        @param pUserItem Pointer to user that want to remove variable
        @return true on success, false on failure.
     */
    bool remove(std::string &name, CUserItem *pUser);

    /*!
        Remove variable from object
        @param variable Variable object
        @param pUserItem Pointer to user that want to remove variable
        @return true on success, false on failure.
     */
    bool remove(CVariable &variable, CUserItem *pUser);

    /*!
        Read persistent variables from XML file.
        @param Path to XML file. Defaults to empty in which case
                the default XML path is used.
        @return Returns true on success false on failure.
     */
    bool loadFromXML(const std::string &path = "");

    /*!
        Write persistent variables to configured storage
        @return Returns true on success false on failure.
     */
    bool save(void);

    /*!
        Write persistent variables to file
        @param strcfgfile path to variable file where data should be written.
        @return Returns true on success false on failure.
     */
    bool save(std::string &path,
              uint8_t whatToSave = VARIABLE_INTERNAL | VARIABLE_EXTERNAL);

    /*!
        Write one variable out to XML persistent storage
        @param pFileStream Pointer to an open output stream
        @param variable Variable to write out.
        @return True on success.
     */
    bool writeVariableToXmlFile(std::ofstream &of, CVariable &variable);

    /*!
       Create external variable table
       @return true on success
     */
    bool doCreateExternalVariableTable(void);

    /*!
        Create internal variable table
        This table is always created from scratch
        @return true on success
     */
    bool doCreateInternalVariableTable(void);

    /*!
     * Prepare list functionality. Must be called before any of the other
     * list methods.
     * @param nameArray String array that will receive sorted array with
     * variables names that meet regular expression.
     * @param search Regular expression to use for names of variables.
     * @param type If not zero selects variable type to list.
     * @param bClear If true the string array will be cleared before it is
     * filled.
     * @return Returns true on success
     */
    bool getVarlistFromRegExp(std::deque<std::string> &nameArray,
                              const std::string &regex = "(.*)",
                              const int type           = 0,
                              bool bClear              = true);

    /*!
     * Prepare list enumeration. Must be called before any of the other
     * list methods.
     * @param pq Context for list enumeration..
     * @param search Regular expression to use for names of variables.
     * @param type If not zero selects variable type to list.
     * @return Returns true on success
     */

    bool listEnumerationFromRegExp(varQuery *pq,
                                   const std::string &regex = "(.*)",
                                   const int type           = 0);
    /*!
     * Get next list item
     * @oaram ppStmt List handle.
     * @param variable Reference to variable which will get data.
     * @return True is returned if valid data is returned.
     */
    bool listItem(varQuery *pq, CVariable &variable);

    /*!
     * Clean up list operation
     * ppStmt List handle.
     */
    void listFinalize(varQuery *pq);

    // -------------------------------------------------------------------

  public:
    /// Path to variable XML file
    std::string m_xmlPath;

    /// Path to the external VSCP variable database
    std::string m_dbFilename;
    sqlite3 *m_db_vscp_external_variable;

    /// Internal variable database
    sqlite3 *m_db_vscp_internal_variable;

    // Last variable save
    vscpdatetime m_lastSaveTime;

    // Stock variables names
    std::deque<std::string> m_StockVariable; // TODO map?
};

#endif
