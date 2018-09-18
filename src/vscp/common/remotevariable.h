// vscpvariable.h
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

//-----------------------------------------------------------------------------
// REMEMBER !!!
// ============
// Remote variables is displayed in clear text but are transferred BASE64 encoded.
// That is XML files,tcp/ip links, websocket links etc use BASE64 encoded
// values and notes. However they are stored in clear text in the database.
//-----------------------------------------------------------------------------

// VSCP daemon variable codes are defined in this file

#if !defined(VSCPVARIABLE__INCLUDED_)
#define VSCPVARIABLE__INCLUDED_

#include <wx/hashset.h>
#include <wx/datetime.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include <sqlite3.h>
#include <guid.h>
#include <userlist.h>
#include <variablecodes.h>
#include <vscphelper.h>

#define VAR_MAXBUF_SIZE             0x10000     // Size for variable strings etc

#define VSCP_VAR_PERSISTENT         true
#define VSCP_VAR_NON_PERISTENT      false

// Permissions
#define PERMISSON_OWNER_READ        0x400
#define PERMISSON_OWNER_WRITE       0x200
#define PERMISSON_OWNER_EXECUTE     0x100
#define PERMISSON_GROUP_READ        0x040   // Not used
#define PERMISSON_GROUP_WRITE       0x020   // Not used
#define PERMISSON_GROUP_EXECUTE     0x010   // Not used
#define PERMISSON_OTHER_READ        0x004
#define PERMISSON_OTHER_WRITE       0x002
#define PERMISSON_OTHER_EXECUTE     0x001

#define PERMISSON_OWNER_ALL         0x700
#define PERMISSON_OWNER_NONE        0x000
#define PERMISSON_GROUP_ALL         0x070   // Not used
#define PERMISSON_GROUP_NONE        0x000   // Not used
#define PERMISSON_OTHER_ALL         0x007
#define PERMISSON_OTHER_NONE        0x000

#define PERMISSON_ALL_READ          0x444
#define PERMISSON_ALL_WRITE         0x222

#define PERMISSON_ALL_RIGHTS        0x777
#define PERMISSON_NO_RIGHTS         0x000
#define PERMSSION_VARIABLE_DEFAULT  0x744



#define ID_NON_PERSISTENT           UINT32_MAX

// Table types - bit-field
#define VARIABLE_STOCK              0x01
#define VARIABLE_INTERNAL           0x02
#define VARIABLE_EXTERNAL           0x04


// Forward declarations
class wxFFileOutputStream;


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

#define VARIABLE_JSON_TEMPLATE "{\n"\
    "\"name\": \"%s\",\n"\
    "\"type\": %hu,\n"\
    "\"user\": %lu,\n"\
    "\"accessrights\": %lu,\n"\
    "\"persistence\": %s,\n"\
    "\"lastchange\": \"%s\",\n"\
    "\"bnumeric\": %s,\n"\
    "\"bbase64\": %s,\n"\
    "\"value\": %s,\n"\
    "\"note\": \"%s\"\n"\
"}"


#define VARIABLE_XML_TEMPLATE "<variable \n"\
    "name=\"%s\" \n"\
    "type=\"%hu\" \n"\
    "user=\"%lu\" \n"\
    "access-rights=\"%lu\" \n"\
    "persistence=\"%s\" \n"\
    "last-change=\"%s\" \n"\
    "value=\"%s\" \n"\
    "note=\"%s\" />"


/*
 * Structure used for multi call variable
 * queries. 
 */

struct varQuery {
    uint8_t tableType;      // Table type: type  1 = Stock, 2=internal, 3=external
    uint32_t stockId;       // ID for next stock variable to return
    sqlite3_stmt *ppStmt;   // Handle for SQL query. NULL if stock
};

typedef struct varQuery varQuery;


// Class that holds one VSCP variable
// Persistent variables should have names staring with $

class CVSCPVariable {
    
public:

    /// Constructor
    CVSCPVariable( void );

    // Destructor
    virtual ~CVSCPVariable( void );
    
    /// Set default values
    void init( void );
    
    /*!
        Name should not contain special characters so if it does
        we replace them with 'underscore'
    */
    void fixName( void );
    
    /*!
     * Make an access right string from supplied rights.
     * 
     * @param  accessrights Standard access rights
     * @param strAccessRights String that will get text form of 
     *          access eights.
     */
    static void makeAccessRightString( uint32_t accessrights, 
                                        wxString& strAccessRights );

    /*!
        Get variable type as string
        @param type Variable numerical type
        @return Variable type as a string.
    */
    static const char *getVariableTypeAsString( int type );

   
    /*!
        Get numerical variable type from symbol or numeric version
        @parm strVariableType Variable type in string form
        @param variable type in numerical form.
     */
    static uint16_t getVariableTypeFromString(const wxString& strVariableType);
    
    /*!
     * Check if the variable is a numerical type
     * @return true if numerical, false otherwise.
     */
    bool isNumerical( void );



    /*!
        Set variable information from a string value
        Format is: "variable name","type","persistence","user","rights","value","note"
        User can be numeric (userid) or given as a username. It is valid to leave all fields
        blank ";;" except for variablename
        @param str String that will get string representation of variable with all info.
        @param bBase64 If true value will be BASE64 encoded before it is stored.
        @param strUser User name that is used if user field is empty.
        @return true if the string could be parsed correctly.
     */
    bool setVariableFromString( const wxString& strVariable, 
                                    bool bBase64=false,
                                    const wxString& strUser = _("admin") );

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
    wxString getAsString( bool bShort=true );
    
    /*!
     * Get the variable as a JSON object
     * @param strVariable Filled in with JSON data
     * @return true of all is OK, false otherwise.
     */
    bool getAsJSON( wxString &strVariable );
    
    /*!
     * Set variable information from a JSON object
     * 
     * @param strVariable JSON object
     * @return true if the string could be parsed correctly.
     */
    bool setFromJSON( wxString &strVariable );
    
    
    /*!
     * Get the variable as a XML object
     * @param strVariable Filled in with JSON data
     * @return true of all is OK, false otherwise.
     */
    bool getAsXML( wxString &strVariable );
    
    /*!
     * Set variable information from a XML object
     * 
     * @param strVariable XML object
     * @return true if the string could be parsed correctly.
     */
    bool setFromXML( wxString &strVariable );

    /*!
        Reset the variable to it's default value
        Set variable to default values
     */
    void Reset(void);


    /*!
        isTrue 
        @return Return true if the variable is of type bool and is true. 
        True is also returned for a numerical varable (int, long, float) if 
        it has a non zero value. In all other cases false will be returned.
     */
    bool isTrue(void);
    
    /*!
     Get the length for the string value of a variable
     */
    size_t getLength( void ) { return m_strValue.Length(); };

    /*!
        setTrue
        Set a variable to true. A boolean variable will be set to true.
        A numerical variable (int, long, float) is set to -1. A string is set to "true".
     */
    void setTrue(void);

    /*!
        setFalse
        Set a variable to false. A boolean variable will be set to false.
        A numerical variable (int, long, float) is set to 0. A string is set to "false".
     */
    void setFalse(void);

        /*!
        Set variable value from string
        @param type Type of value
        @param strValue Value in string form
        @param bBase64 True if strValue is BASE64 encoded.
        @return true on success.
     */
    bool setValueFromString(int type, const wxString& strValue, bool bBase64 = false );


    /*!
        Get the variable value as a string value
        @param str String that will get string representation of variable.
        @param  bBase64 If set to true value is encoded in BASE64
     */
    void writeValueToString( wxString& strValue, bool bBase64=false );

    /*!
        Get string value
        @param bBase64 If true the returned value will be BASE64 encoded.
        @return value in string form is returned regardless of type.
     */
    wxString getValue( bool bBase64 = false );
    
    /*!
        getValue
        @param pval Pointer to string that will get value.
        @param bBase64 If true the returned value will be BASE64 encoded.
     */
    void getValue( wxString *pval, bool bBase64=false );
    
    /*!
        getValue
        @param value String that will receive value.
     */
    bool setValue( wxString strValue, bool bBase64=false  );  

    /*!
        setValue
        @param val long to set value to.
     */
    void setValue( int val );


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
        setValue - vscpEvent
        @param event VSCP Event to set.
    */
    void setValue( vscpEvent& event );
    
    /*!
        getValue
        @param value Bool that will receive the value.
     */
    void getValue( vscpEvent *pEvent );
    
    /*!
        getValue - EventEx
        @param pEventEx [OUT ]VSCP EventEx from variable
     */
    void getValue( vscpEventEx *pEventEx );
    
    /*!
        setValue - vscpEventEx
        @param event VSCP EventEx to set.
     */
    void setValue( vscpEventEx& eventex );
    
    /*!
        GetValue - Date + time format
        @param pValue [OUT] Date + time value in ISO format
     */
    void getValue( wxDateTime *pValue );
    
    /*!
        SetValue - Date + Time ISO format
        @param val Date + time in ISO format.
     */
    void setValue( wxDateTime& val );
        

    /*!
     * setUser
     */
    bool setOwnerId( uint32_t ownerid );
    bool setOwnerIdFromUserName( wxString& strUser );
    bool setOwnerdFromClient( uint32_t ownerid );

    /*!
        Change last change date time to now
    */
    void setLastChangedToNow( void ) { m_lastChanged = wxDateTime::Now(); };

    /*!
     * Get last changed date/time
     * @return Last changed date/time
     */
    wxDateTime& getLastChange( void ) { return m_lastChanged; };

    // Getters/Setters
    
    // id
    void setID( uint32_t id ) { m_id = id; };
    uint32_t getID( void ) { return m_id; };
    
    // name
    /*!
        Set variable name
        @param strName Name of variable
        @return true on success
     */
    bool setName( const wxString& strName );
    wxString& getName( void ) { return m_name.MakeUpper(); };
    
    // type
    /*!
        set type
        @param strType type in numerical or string form
        @return True on success.
     */
    bool setType( const wxString& strType );
    void setType( uint16_t type ) { m_type = type; };
    uint16_t getType( void ) { return m_type; };

    // note
    bool setNote( const wxString& strNote, bool bBase64=false );
    wxString getNote( bool bBase64=false );
    bool getNote( wxString& strNote, bool bBase64 = false );
    
    // Persistence
    bool isPersistent( void ) { return m_bPersistent; };
    void setPersistent( bool b ) { m_bPersistent = b; };
    
    // Access rights
    void setAccessRights( uint32_t accessRights ) { m_accessRights = accessRights; };
    uint32_t getAccessRights( void ) { return m_accessRights; };
    void getAccessRightStr( wxString& strAccessRights  ) 
        { makeAccessRightString( m_accessRights, strAccessRights ); };
    
    bool isUserWritable( void ) { return ( m_accessRights & 0x02 ) ? true : false; };
    void makeUserWritable( bool b ) { m_accessRights |= 0x02; };

    bool isOwnerWritable( void ) { return ( m_accessRights & 0x80 ) ? true : false; };
    void makeOwnerWritable( bool b ) { m_accessRights |= 0x80; };
    
    // owner id
    void setOwnerID( uint32_t uid ) { m_userid = uid; };
    uint32_t getOwnerID( void ) { return m_userid; };
    
    // stock variable
    void setStockVariable( bool bStock = true ) { m_bStock = bStock; };
    bool isStockVariable( void ) { return m_bStock; };
    
private:

    // id in database
    uint32_t m_id;
    
    // Name of variable
    wxString m_name;
    
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
    wxString m_note;
    
    /*! 
        Storage
        A "class" variable is stored in the VSCP event.
        A "type" variable is stored in the VSCP event.
        A "GUID" variable is stored in the VSCP event.
        A VSCP data variables s stored in the VSCP event.
     */
    wxString m_strValue;
    
    // True if this is a stock variable
    bool m_bStock;
    
    // Variable hook  TODO
    // Used to get notification in websocket when a variable changed it's value
    // Contains client id's for clients that should be informed if the variable is written to.
    wxArrayLong m_clientHooks;

public:
    
    // Time when variable was last changed.
    wxDateTime m_lastChanged;
    
};





class CVariableStorage {
public:
    /// Constructor
    CVariableStorage();

    /// Destructor
    virtual ~CVariableStorage();
    
    // Initialise variable storage
    // @return true on success.
    bool init( void );
    
    /*!
     * Find variable from it's name
     * 
     * First stock variables is searched (start with "vscp.", next non-persistent
     * variables (in-memory) is searched, and last persistent variables is
     * searched.
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @return >0 if variable is found, zero if not. 
     */
    uint32_t find( const wxString& name, CVSCPVariable& variable );
    
    
    /*!
     * Check if a variable exist.
     * @param name Name of variable
     * @return id if the variable exists, zero else.
     * 
     */
    uint32_t exist( const wxString& name );
    
    
    /*!
     * Get a stock variable
     * 
     * First stock variable (start with "vscp.").
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @param pUser Pointer to user performing operation. NULL
                    is system of admin user.
     * @return Return id if variable is found, UINT_MAX if variable is internal and dynamic, 
     *      zero if the variable is not found. 
     */
    uint32_t getStockVariable( const wxString& name, 
                                    CVSCPVariable& pVar, 
                                    CUserItem *pUser = NULL );
    
    /*!
        Write stock variable.    
        @param name Name of variable.
        @param var Reference to variable to write
        @param pUser Pointer to user performing operation. NULL
                    is system of admin user.
        @return true on success.
     */
    bool putStockVariable( CVSCPVariable& var, CUserItem *pUser = NULL );
    
    /*!
       Set variable value from Database record.
       @param ppStmt sqlite3 ref to record
       @param variable Variable that will get record data.
       @return true on success.
     */
    bool setVariableFromDbRecord( sqlite3_stmt *ppStmt, 
                                    CVSCPVariable& variable );
    
    /*!
     * Find a non-persistent variable
     * 
     * Find non-persistent variable (in-memory variable).
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @return id if variable is found, zero if not. 
     */
    uint32_t findNonPersistentVariable( const wxString& name, CVSCPVariable& pVar );
    
    /*!
     * Find a persistent variable
     * 
     * Find persistent variable.
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @return id if variable is found, zero if not. 
     */
    uint32_t findPersistentVariable( const wxString& name, CVSCPVariable& pVar );

    /*!
     * Add stock variable
     * Stock variable is added to internal variables but there 
     * values is not read/written from there. They are just in the
     * internal space for sorting/listing.
     * @param var Reference to variable to be added.
     * @return true on success.
     */
    bool addStockVariable( CVSCPVariable& var );
    
    /*!
        Add a variable.
        @param var Variable object.
        @return true on success, false on failure.
     */
    bool add( CVSCPVariable& var );
    
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
    bool add( const wxString& name,
                    const wxString& value,
                    const uint16_t type = VSCP_DAEMON_VARIABLE_CODE_STRING,
                    const uint32_t userid = USER_ID_ADMIN,
                    const bool bPersistent = false,
                    const uint32_t accessRights = PERMISSON_OWNER_ALL,
                    const wxString& note = _("") );

    // Variant of the above with string as type
    bool add( const wxString& name,
                            const wxString& value,
                            const wxString& strType, 
                            const uint32_t userid = USER_ID_ADMIN,
                            const bool bPersistent = false,
                            const uint32_t accessRights = PERMISSON_OWNER_ALL,
                            const wxString& note = _("") );
    
    // Update a variable (write to db)
    bool update( CVSCPVariable& var );

    /*!
        Remove named variable
        @param name Name of variable
        @return true on success, false on failure.
     */
    bool remove( wxString& name );

    /*!
        Remove variable from object
        @param pVariable Pointe to variable object
        @return true on success, false on failure.
     */
    bool remove( CVSCPVariable& variable );

    /*!
        Read persistent variables from XML file. 
        @param Path to XML file. Defaults to empty in which case
                the default XML path is used.
        @return Returns true on success false on failure.
     */
    bool loadFromXML( const wxString& path = _("") );

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
    bool save( wxString& path, 
                uint8_t whatToSave = VARIABLE_INTERNAL | VARIABLE_EXTERNAL );
    
    /*!
        Write one variable out to XML persistent storage
        @param pFileStream Pointer to an open output stream
        @param variable Variable to write out.
        @return True on success.
     */
    bool writeVariableToXmlFile( wxFFileOutputStream *pFileStream, CVSCPVariable& variable );
    
     /*!
        Create external variable table
        @return true on success
      */
     bool doCreateExternalVariableTable( void );
     
    /*!
        Create internal variable table
        This table is always created from scratch
        @return true on success
     */
    bool doCreateInternalVariableTable( void );
     
     
    /*!
     * Prepare list functionality. Must be called before any of the other
     * list methods.
     * @param nameArray String array that will receive sorted array with variables
     *                  names that meet regular expression.
     * @param search Regular expression to use for names of variables.
     * @param type If not zero selects variable type to list.
     * @param bClear If true the string array will be cleared before it is filled.
     * @return Returns true on success
     */
    bool getVarlistFromRegExp( wxArrayString& nameArray,
                                const wxString& regex = _("(.*)"),
                                const int type = 0,
                                bool bClear = true );
    
    /*!
     * Prepare list enumeration. Must be called before any of the other
     * list methods.
     * @param pq Context for list enumeration..
     * @param search Regular expression to use for names of variables.
     * @param type If not zero selects variable type to list.
     * @return Returns true on success
     */
    
    bool listEnumerationFromRegExp( varQuery *pq, 
                                        const wxString& regex = _("(.*)"),
                                        const int type = 0 );
    /*!
     * Get next list item
     * @oaram ppStmt List handle.
     * @param variable Reference to variable which will get data.
     * @return Ture is returned if valid data is returned. 
     */
    bool listItem( varQuery *pq, CVSCPVariable& variable );
    
    /*!
     * Clean up list operation
     * ppStmt List handle.
     */
    void listFinalize( varQuery *pq );

  
     // -------------------------------------------------------------------
     

public:
    
    /// Path to variable XML file 
    wxString m_xmlPath;
        
    /// Path to the external VSCP variable database
    wxFileName m_dbFilename; 
    sqlite3 *m_db_vscp_external_variable;
    
    /// Internal variable database
    sqlite3 *m_db_vscp_internal_variable;

    // Last variable save
    wxDateTime m_lastSaveTime;
    
    // Stock variables names
    //wxArrayString m_StockVariable;
    
};


#endif

