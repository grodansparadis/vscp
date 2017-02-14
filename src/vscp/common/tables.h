// tables.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017
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

#if !defined(VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#include <sqlite3.h>
#include <clientlist.h>

// Table types
enum vscpTableType {
    VSCP_TABLE_DYNAMIC = 0,
    VSCP_TABLE_STATIC
} ;

// Holds a table item
struct vscpTableItem {
    wxDateTime date;
    double value;
};

// Class that holds one VSCP table

class CVSCPTable {
public:

    /*!
     *  Constructor table
     *
     *  @param folder The base folder for table database files. Must be a 
     *                  valid path.
     *  @param name Name of this VSCP table. Must be system unique.
     *  @param bMemeory If true the table will be created in memory.
     *  @param type Either static or dynamic table. A static table have a 
     *              static size.
     *  @param size This is the size for a static table. Must be greater than 
     *                  zero for a static table but is a no care for a dynamic 
     *                  table.
     */
    
    CVSCPTable( const wxString &folder,    
                    const wxString &name, 
                    const bool bInMemory = false, 
                    const vscpTableType type = VSCP_TABLE_DYNAMIC, 
                    uint32_t size = 0 );

    // Destructor
    virtual ~CVSCPTable(void);

    /*!
        Open/create file and get ready to work with it.
        @return true on success, false on error.
    */
    
    bool init();
    

   /*!
    * Create the database
    * @return true in success, false on failure
    */
    
    bool createDatabase( void );
    
    /*!
     *  Set table info
     *   
     *  @param owner The name of the table owner
     *  @param rights Permissions for this object
     *  @param title Diagram title
     *  @param xlavel Name of X-axis
     *  @param ylabel Name of Y axis
     *  @param note Diagram note
     *  @param sqlcreate SQL to create table
     *  @param sqlinsert SQL to insert value in table
     *  @param sqldelete SQL to delete item in table
     *  @param description Description for table
     *  @return true on success, false otherwise
     */
    
    bool setTableInfo( const wxString &owner,
                            const uint16_t rights,
                            const wxString &title,
                            const wxString &xname, 
                            const wxString &yname,
                            const wxString &note,
                            const wxString &sqlcreate,
                            const wxString &sqlinsert,
                            const wxString &sqldelete,
                            const wxString &description ); 
    
    /*!
     *  Set table info
     *   
     *  @param pUserItem Pointer to user object
     *  @param rights Permissions for this object
     *  @param title Diagram title
     *  @param xlavel Name of X-axis
     *  @param ylabel Name of Y axis
     *  @param note Diagram note
     *  @param sqlcreate SQL to create table
     *  @param sqlinsert SQL to insert value in table
     *  @param sqldelete SQL to delete item in table
     *  @param description Description for table
     *  @return true on success, false otherwise
     */
    
    bool setTableInfo( CUserItem *pUserItem,
                            const uint16_t rights,
                            const wxString &title,
                            const wxString &xname, 
                            const wxString &yname,
                            const wxString &note,
                            const wxString &sqlcreate,
                            const wxString &sqlinsert,
                            const wxString &sqldelete,
                            const wxString &description );
    
    /*!
     * Set event info for this table
     * 
     * @param vscp_class VSCP class used for the measurement values.
     * @param vscp_type VSCP Type used for the measurement values.
     * @param vscp_sensorindex Sensor index for the measurements.
     * @param vscp_unit Unit for the measurements.
     * @param vscp_zone Zone for the measurements.
     * @param vscp_subzone Sunzone for the measurements. 
     */
    
    void setTableEventInfo( uint16_t vscp_class,
                                uint16_t vscp_type,
                                uint8_t vscp_sensorindex = 0,
                                uint8_t vscp_unit = 0,
                                uint8_t vscp_zone = 255,
                                uint8_t vscp_subzone = 255 );
    
    /*!
     * Test if a measurement event match the data that should be logged.
     * 
     * @param Decision Matrix event to check
     * @return True if the event is one that should be logged, false otherwise.
     * 
     */
    
    bool isThisEventForMe( vscpEvent *pEvent );
        
    /*!
     * Clean up everything an close he database
     */
    
    void CleanUpAndClose( void );
    
    /*!
     * Log data
     * 
     * @param time Date/time from value holding the number of seconds since Jan 1, 1970 UTC. 
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */
    
    bool logData( time_t time, double value );
    
    /*!
     * Log data
     * 
     * @param Date/time double that represent the date from the so-called Julian Day Number
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */
    
    bool logData( double jdn, double value );
    
    /*!
     * Log data
     * 
     * @param tm Date/time from the standard tm structure.
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */
    
    bool logData( const struct tm &tm, double value );
    
    /*!
     * Log data
     * 
     * @param strtime Date/time string on ISO format "YYYY-MM-DDTHH:MM:SS.sss"
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */
    
    bool logData( wxString &strtime, double value );
    
    /*!
     * Log data
     * 
     * @param Date DateTime object.
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */
    
    bool logData( wxDateTime &time, double value );
    
    /*!
     * Log data
     * 
     * @param Date DateTime object.
     * @param value Double that represent the value for the measurement.
     * @param sqlInsert This is the original sqlInsert expression after VSCP
     *          decision matrix escapes has been inserted in it but before value 
     *          and date/time insertion.
     * @return true on success, false if failure.
     */
    
    bool logData( wxDateTime &time, double value, const wxString &sqlInsert );
    
    /*!
     * Insert data into the table
     * 
     * @param strInsert SQL expression for insert. This is usually the 
     *         stored sqlinsert expression with VSCP decision matrix escapes
     *          inserted and value and date/time inserted.
     * @return true on success, false if failure.
     */
    bool doInsert( const wxString strInsert );
    
    /*!
     * Clear the content of a VSCP table
     * 
     * @return true om success, false on failure.
     */
    bool clearTable( void );
    
      
    bool start_RangeOfData( wxDateTime& wxStart, wxDateTime& wxEnd, int *phandle );

    bool end_RangeOfData( int *phandle, vscpTableItem *pItem );
    
    bool getItem_RangeOfData( int *phandle );

    /*! 
        Get number of records for a range of data
        @param from date/time from which data should be fetched
        @param to date/time to which data should be fetched
        @return Number of records read or to read or -1 if error.
    */
    
    long getRangeOfDataCount( wxDateTime& wxStart, wxDateTime& wxEnd );
    
    /*!
        Get a data range
        @param from date/time from which data should be fetched
        @param to date/time to which data should be fetched
        @param buf Buffer where result will be placed. If buffer is NULL
                no data will be filled only the number of records will be
                returned.
        @param size Size of buffer in bytes
        @return Number of records read or to read or -1 if error.
    */
    
    long getRangeOfData( wxDateTime& wxStart, wxDateTime& wxEnd, void *buf, uint16_t size );

    /*!
        Get static dataset
        @param buf Buffer that holds the result
        @param size of buffer
        @return Number of records in buffer or zero on error
    */
    
    long getStaticData( void *buf, uint32_t size );

    /*!
        Get required buffer size for static file
        @return Returns the number of bytes needed to hold the
                static file, -1 on error.
    */
    
    long getStaticRequiredBuffSize( void );
        
    /*!
     * Get the first date/ime in table
     */
    
    wxDateTime getFirstDate( void );
    
    /*!
     * Get the last date/time in table
    */
    
    wxDateTime getLastDate( void );
    
    /*!
     * Get max value
    */
    
    double getMax( wxDateTime from, wxDateTime to );
    
    /*!
     * Get min value
    */
    
    double getMin( wxDateTime from, wxDateTime to );
    
    /*!
     * Calculate mean over a range
    */
    
    double getMean( wxDateTime from, wxDateTime to );
    
    /*!
     * Get median value
    */
    double getMedian( wxDateTime from, wxDateTime to );

    // Get number of records in database
    long getNumberOfRecords( void );
    
    // * * * Getters & setters
        
    /*!
     *  Set path to database
     *  @param dbpath Pointer to null terminated string with path to 
     *          database location or where database should b created.
     *  @return true on success, false on failure.
     */
    
    bool setPathToDB( const wxString &dbpath ) { m_dbFile.Assign( dbpath ); return m_dbFile.IsOk(); };
    
    // Get path to database
    wxString getPathToDB( void ) { return m_dbFile.GetFullPath(); };

    // Get the table name
    wxString& getTableName( void ) { return m_tableName; };
    
    // Get SQL insert expression
    wxString& getSQLInsert( void ) { return m_sqlInsert; };

public:
    
    // Protector for this table
    wxMutex m_mutexThisTable;

private:

    /// Path to database file
    wxFileName m_dbFile;
    
    /// Database
    sqlite3 *m_dbTable;
    
    /// Root folder for tables
    wxString m_folder;
    
    // Name for table.
    wxString m_tableName;
    
    // Flag for in memory database
    bool m_bInMemory;
    
    // Table type
    vscpTableType m_type;
    
    // Size for table with constant size (zero for ever growing)
    uint32_t m_size;
    
    
    // Table description
    wxString m_tableDescription;
    
    // Diagram title
    wxString m_labelTitle;
    
    // Diagram note
    wxString m_labelNote;

    // Label for X-axis
    wxString m_labelX;
    
    // Label for y-axis
    wxString m_labelY;
    
    
    
    // SQL to create table.
    wxString m_sqlCreate;
    
    // SQL to insert value.
    wxString m_sqlInsert;
    
    /// True if '%f' is places before '%s' in insert string
    bool m_bValueFirst;
    
    // SQL to delete table.
    wxString m_sqlDelete;
    
    // Owner
    CUserItem *m_pOwner;
    
    /// Permissions for other user to use this table
    uint16_t m_rights;
    
    // * * * Used for event compares * * *
    
    /// VSCP class we collect data in
    uint16_t m_vscp_class;
    
    /// VSCP type we collect data in
    uint16_t m_vscp_type;
    
    /// Sensor index for the sensor we collect data from
    uint8_t m_vscp_sensorindex;
    
    /// Unit for the sensor data we collect
    uint8_t m_vscp_unit;
    
    /// Zone for the sensor data
    uint8_t m_vscp_zone;
    
    /// Subzone for the sensor data
    uint8_t m_vscp_subzone;

};


WX_DECLARE_LIST( CVSCPTable, listVSCPTables );


/******************************************************************************/
/*                           CUserTableObjList                                */
/******************************************************************************/


// Class that holds all user defined VSCP tables

class CUserTableObjList {
    
public:
    
    /// Constructor
    CUserTableObjList( void );

    /// Destructor
    virtual ~CUserTableObjList( void );
    
    /*!
     * Add table
     * 
     * @param pTable Table object to add.
     * @return True on success and false on failure.
     * 
     */
    bool addTable( CVSCPTable *pTable );
    
    /*!
     * Get user table from name
     * 
     * @param name Name of table to lookup
     * @return Pointer to table on success, NULL on failure. 
     * 
     */
    CVSCPTable *getTable( wxString &name );
    
    /*!
     * Check if a name is used.
     * 
     * @param name Name to search for
     * @return True if name is used, false otherwise.
     * 
     */
    
    
    bool isNameUsed( wxString &name );
    
    /*!
     * Remove one table item from it's name
     * 
     * @param name NAme of table
     * @return True on success, false on failure.
     */
    bool removeTable( wxString &name );
    
    /*!
     * Remove all items in the table
     */
    void clearTable( void );
    
    /*!
     * Initialize all tables
     * 
     * @return true on success
     */
    bool init( void );
    
    /*!
     * Read tables definitions from database
     */
    bool readTablesFromDB( void );
    
    /// Table list
    listVSCPTables m_listTables;
    
    /// Mutex for table list
    wxMutex m_mutexTableList;
};


#endif
