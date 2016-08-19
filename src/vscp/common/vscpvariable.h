// vscpvariable.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2016 
// Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


// VSCP daemon variable codes are defined in this file

#if !defined(VSCPVARIABLE__INCLUDED_)
#define VSCPVARIABLE__INCLUDED_

#include <wx/hashset.h>
#include <wx/datetime.h>
#include <wx/tokenzr.h>
#include <sqlite3.h>
#include <guid.h>
#include "variablecodes.h"
#include "vscphelper.h"

#define VAR_MAXBUF_SIZE         0x10000     // Size for variable strings etc

#define VSCP_VAR_PERSISTENT     true
#define VSCP_VAR_NON_PERISTENT  false

// Permissions
#define PERMISSON_OWNER_READ    0x400
#define PERMISSON_OWNER_WRITE   0x200
#define PERMISSON_OWNER_EXECUTE 0x100
#define PERMISSON_GROUP_READ    0x040
#define PERMISSON_GRPUP_WRITE   0x020
#define PERMISSON_GROUP_EXECUTE 0x010
#define PERMISSON_OTHER_READ    0x004
#define PERMISSON_OTHER_WRITE   0x002
#define PERMISSON_OTHER_EXECUTE 0x001

#define PERMISSON_OWNER_ALL     0x700
#define PERMISSON_OWNER_NONE    0x000
#define PERMISSON_GROUP_ALL     0x070
#define PERMISSON_GROUP_NONE    0x000
#define PERMISSON_OTHER_ALL     0x007
#define PERMISSON_OTHER_NONE    0x000

#define PERMISSON_ALL_READ      0x444

#define PERMISSON_ALL_RIGHTS    0x777
#define PERMISSON_NO_RIGHTS     0x000

#define USER_ADMIN              0x00
#define GROUP_ADMIN             0x00

// Table types - bitfield
#define VARIABLE_STOCK          0x01
#define VARIABLE_INTERNAL       0x02
#define VARIABLE_EXTERNAL       0x04

// Forward declarations
class wxFFileOutputStream;


/*
 * Structure used for multi call variable
 * queries. 
 */
struct varQuery {
    uint8_t table;          // Table type    
    uint32_t stockId;       // ID for next stock variable to return
    sqlite3_stmt *ppStmt;   // Handle for SQL query. NULL if stock
};

typedef struct varQuery varQuery;

// Class that holds one VSCP variable
// Persistent variables should have names staring with $

class CVSCPVariable {
public:
   
    enum vartype {
        STRING_T = 0,
        BOOL_T,
        INT_T,
        LONG_T,
        FLOAT_T,
        MEASUREMENT_T,
        EVENT_T,
        GUID_T,
        DATA_T,
        VSCPCLASS_T,
        VSCPTYPE_T,
        TIMESTAMP_T,
        DATETIME_T,
        BASE64_T,
        MIME_T = 100,
        HTML_T,
        JAVASCRIPT_T,
        JSON_T,
        XML_T,
        SQL_T,
        LUA_T = 200,
        LUARES_T,
        UX1_T = 300
    };

    /// Constructor
    CVSCPVariable(void);

    // Destructor
    virtual ~CVSCPVariable(void);

    
    
    /*!
     * setRights
     * Set rights for the variable. Can be set either on numerical form or
     * alphanumerical form.
     * uuugggooo
     * uuu - owner RWX/numerical
     * ggg - group RWX/numerical
     * ooo - other RWX/numerical
     */
    bool setRighs( wxString& strRights );

    /*!
     * setUser
     */
    bool setUser( wxString& strUser );
    
    /*!
     * setGroup
     */
    bool setGroup( wxString& strUser );
    
    
    /*!
        Get variable type as string
        @param type Variable numerical type
        @return Variable type as a string.
    */
    static const char * getVariableTypeAsString( int type );

   
    /*!
        Get numerical variable type from symbol or numeric version
        @parm strVariableType Variable type in string form
        @param variable type in numerical form.
     */
    static uint16_t getVariableTypeFromString(const wxString& strVariableType);


    /*!
        Set variable value from string
        @param type Type of value
        @param strValue Value in string form
        @return true on success.
     */
    bool setValueFromString(int type, const wxString& strValue, bool bBase64 = false );

    /*!
        Set variable value from string
        @param vartype Type of value (enum)
        @param strValue Value in string form
        @return true on success.
     */
    bool setValueFromString( CVSCPVariable::vartype type, const wxString& strValue, bool bBase64=false );

    /*!
        Get the variable value as a string value
        @param str String that will get string representation of variable.
        @param  bBase64 If true strings are encoded in BASE64
     */
    bool writeValueToString( wxString& strValue, bool bBase64=false );

    /*!
        Get variable information as a string value
        Format is: "variable name",type,"persistence","value"
        example 1: test_int,3,true,24000
        example 2: test_string,1,true,"This is a string"
        example 3: test_event,7,false,0,20,1,2,3,4,5,6

        @param str String that will get string representation of variable with all info.
     */
    bool getVariableFromString( const wxString& strVariable, bool bBase64=false );


    /*!
        Reset 
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
        Get string value
     */
    wxString getValue( void ) { return  m_strValue; };
    
    /*!
        getValue
        @return value in string form is returned regardless of type.
     */
    void getValue( wxString *pval ) { *pval =  m_strValue; };
    
    /*!
        getValue
        @param value String that will receive value.
     */
    void setValue(wxString value) { m_strValue = value; };

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
        Change last change date time to now
    */
    void setLastChangedToNow( void ) { m_lastChanged = wxDateTime::Now(); };

    /*!
     * Get last changed date/time
     * @return Last changed date/time
     */
    wxDateTime& getLastChange( void ) { return m_lastChanged; };
    
    /*!
        Name should not contain special characters so if it does
        we replace them with 'underscore'
    */
    void fixName( void );
    
    /*!
        Set variable name
        @param strName Name of variable
     */
    void setName(const wxString& strName);

    // Getters/Setters
    
    // id
    void setID( uint32_t id ) { m_id = id; };
    uint32_t getID( void ) { return m_id; };
    
    // name
    void setName( wxString& name ) { m_name = name; fixName(); };
    wxString& getName( void ) { return m_name; };
    
    // type
    void setType( uint16_t type ) { m_type = type; };
    uint16_t getType( void ) { return m_type; };

    // note
    void setNote(const wxString& strNote) { m_note = strNote; };
    wxString& getNote( void ) { return m_note; };
    
    // Persistence
    bool isPersistent( void ) { return m_bPersistent; };
    void setPersistent( bool b ) { m_bPersistent = b; };
    
    // Access rights
    void setAccessRights( uint32_t accessRights ) { m_accessRights = accessRights; };
    uint32_t getAccessRights( void ) { return m_accessRights; };
    bool isWritable( void ) { return m_accessRights; };
    void makeWritable( bool b ) { m_accessRights = b; };
    
    // user id
    void setUserID( uint32_t uid ) { m_userid = uid; };
    uint32_t getUserID( void ) { return m_userid; };
    
    // user id
    void setGroupID( uint32_t uid ) { m_groupid = uid; };
    uint32_t getGroupID( void ) { return m_groupid; }; 
    
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
    uint32_t m_accessRights;
    
    // User this variable is owned by
    uint32_t m_userid;
    
    // Group this variable belongs to
    uint32_t m_groupid;
    
    /// Note about variable
    wxString m_note;
    
    /*! 
        Storage
        A "class" variable is stored in the VSCP event.
        A "type" variable is stored in the VSCP event.
        A "GUID" variable is stored in the VSCP event.
        A VSCP data variables s stored in the VSCP event.
     */
    wxString m_strValue;            // String
    
    // True if this is a stick variable
    bool m_bStock;

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
    
    /*!
     * Find variable from it's name
     * 
     * First stock variables is searched (start with "vscp.", next non-persistent
     * variables (in-memory) is searched, and last persistent variables is
     * searched.
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @return true if variable is found. 
     */
    uint32_t find(const wxString& name, CVSCPVariable& variable );
    
    
    /*!
     * Check if a variable exist.
     * @param name Name of variable
     * @return id if the variable exists, zero else.
     * 
     */
    uint32_t exist(const wxString& name );
    
    
    /*!
     * Find a stock variable
     * 
     * First stock variable (start with "vscp.").
     * @param name Name of variable
     * @param pVar [OUT] If found supplied variable is filled with data. Can be 
     * set to NULL in which case only availability of the variable is returned.
     * @return True if the variable is found. 
     */
    bool findStockVariable( const wxString& name, CVSCPVariable& pVar );
    
    /*!
        Write stock variable.    
        @param name Name of variable.
        @param var Reference to variable to write
        @return true on success.
     */
    bool writeStockVariable( CVSCPVariable& var );
    
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
        @param groupid Group this variable belongs to.
        @param bPersistent True if the variable should be saved to
        persistent storage.
        @param accessRights Access rights for the variable
        @return true on success, false on failure.
     */
    bool add(const wxString& name,
                    const wxString& value,
                    const uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING,
                    const uint32_t userid = USER_ADMIN,
                    const uint32_t groupid = GROUP_ADMIN,
                    const bool bPersistent = false,
                    const uint32_t accessRights = PERMISSON_OWNER_ALL );

    // Variant of the above with string as type
    bool add(const wxString& varName,
                            const wxString& value,
                            const wxString& strType, 
                            const uint32_t userid = USER_ADMIN,
                            const uint32_t groupid = GROUP_ADMIN,
                            const bool bPersistent = false,
                            const uint32_t accessRights = PERMISSON_OWNER_ALL);

    /*!
        Remove named variable
        @param name Name of variable
        @return true on success, false on failure.
     */
    bool remove(wxString& name);

    /*!
        Remove variable from object
        @param pVariable Pointe to variable object
        @return true on success, false on failure.
     */
    bool remove( CVSCPVariable& variable );

    /*!
        Read persistent variables
        @return Returns true on success false on failure.
     */
    bool load( wxString& path );

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
    bool save(wxString& path, uint8_t whatToSave = VARIABLE_INTERNAL | VARIABLE_EXTERNAL );
    
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
     * list methods
     * @param table Select list (only one can be selected)
     * @param search Search and ordering criteria.
     * @return List handle on success, NULL on failure.
     */
    varQuery *listPrepare( const uint8_t table, const wxString& search );
    
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

    /*!
        Configuration path for variable persistent storage XML file
     */
    wxString m_configPath;
        
    /// Path to the external VSCP variable database
    wxFileName m_path_db_vscp_external_variable; 
    sqlite3 *m_db_vscp_external_variable;
    
    /// Internal variable database
    sqlite3 *m_db_vscp_internal_variable;

    // Last variable save
    wxDateTime m_lastSaveTime;
    
    // Stock variables names
    wxArrayString m_StockVariable;
};


#endif

