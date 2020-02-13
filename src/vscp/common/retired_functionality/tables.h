// tables.h
//
// This file is part of the VSCP (https://www.vscp.org)
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

#if !defined(VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#include <clientlist.h>
#include <sqlite3.h>
#include <vscpdatetime.h>

// Table types
enum vscpTableType
{
    VSCP_TABLE_DYNAMIC = 0, // Ever growing table
    VSCP_TABLE_STATIC,      // Table with specific size
    VSCP_TABLE_MAX          // Table that go up to size and then restart.
};

// Holds a table item
struct vscpTableItem
{
    vscpdatetime date; // YYYY-MM-DDTHH:MM:SS.sss
    double value;      // Measurement value
};

// Class that holds one VSCP table

class CVSCPTable
{

  public:
    /*!
     *  Constructor table
     *
     *  @param folder The base folder for table database files. Must be a
     *                  valid path.
     *  @param name Name of this VSCP table. Must be system unique.
     *  @param bEnable If true the table should be initialized by the system.
     *  @param bMemeory If true the table will be created in memory.
     *  @param type Either max, static or dynamic table. A static and max table
     * have a static size.
     *  @param size This is the size for a static table. Must be greater than
     *                  zero for a static table but is a no care for a dynamic
     *                  table.
     */

    CVSCPTable(const std::string &folder,
               const std::string &name,
               const bool bEnable       = true,
               const bool bInMemory     = false,
               const vscpTableType type = VSCP_TABLE_DYNAMIC,
               uint32_t size            = 0);

    // Destructor
    virtual ~CVSCPTable(void);

    /*!
     *  Set table info
     *
     *  @param pUserItem Pointer to user object
     *  @param rights Permissions for this object
     *  @param title Diagram title
     *  @param xlavel Name of X-axis
     *  @param ylabel Name of Y axis
     *  @param note Diagram note
     *  @param sqlcreate SQL to create table. Defaults to
     *              CREATE TABLE 'vscptable' ( `idx` INTEGER NOT NULL PRIMARY
     * KEY UNIQUE, `datetime` TEXT, `value` REAL DEFAULT 0 ); if empty.
     *  @param sqlinsert SQL to insert value in table. Defaults to
     *              INSERT INTO 'vscptable' (datetime,value) VALUES
     * ('%%s','%%f'); if empty.
     *  @param sqldelete SQL to delete item in table
     *  @param description Description for table
     *  @return true on success, false otherwise
     */

    bool setTableInfo(const std::string &owner,
                      const uint16_t rights,
                      const std::string &title,
                      const std::string &xname,
                      const std::string &yname,
                      const std::string &note,
                      const std::string &sqlcreate,
                      const std::string &sqlinsert,
                      const std::string &sqldelete,
                      const std::string &description);

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

    void setTableEventInfo(uint16_t vscp_class,
                           uint16_t vscp_type,
                           uint8_t vscp_sensorindex = 0,
                           uint8_t vscp_unit        = 0,
                           uint8_t vscp_zone        = 255,
                           uint8_t vscp_subzone     = 255);

    /*!
        Open/create file and get ready to work with it.
        @return true on success, false on error.
    */

    bool init();

    /*!
     * Create the database
     * @return true in success, false on failure
     */

    bool createDatabase(void);

    /*!
     * Test if a measurement event match the data that should be logged.
     *
     * @param Decision Matrix event to check
     * @return True if the event is one that should be logged, false otherwise.
     *
     */

    bool isThisEventForMe(vscpEvent *pEvent);

    /*!
     * Clean up everything an close he database
     */

    void cleanUpAndClose(void);

    /*!
     * Log data
     *
     * @param time Date/time from value holding the number of seconds since Jan
     * 1, 1970 UTC.
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */

    bool logData(time_t time, double value, int ms = 0);

    /*!
     * Log data
     *
     * @param Date/time long that represent the date from the so-called Julian
     * Day Number
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */

    bool logData(const double jdn, double value, int ms = 0);

    /*!
     * Log data
     *
     * @param tm Date/time from the standard tm structure.
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */

    bool logData(const struct tm &tm, double value, int ms = 0);

    /*!
     * Log data
     *
     * @param strtime Date/time string on ISO format "YYYY-MM-DDTHH:MM:SS.sss"
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */

    bool logData(std::string &strtime, double value);

    /*!
     * Log data
     *
     * @param dt DateTime object.
     * @param value Double that represent the value for the measurement.
     * @return true on success, false if failure.
     */

    bool logData(vscpdatetime &dt, double value);

    /*!
     * Log data
     *
     * @param dt DateTime object.
     * @param value Double that represent the value for the measurement.
     * @param sqlInsert This is the original sqlInsert expression after VSCP
     *          decision matrix escapes has been inserted in it but before value
     *          and date/time insertion.
     * @return true on success, false if failure.
     */

    bool logData(vscpdatetime &dt, double value, const std::string &sqlInsert);

    /*!
     * Insert data into the table
     *
     * @param strInsert SQL expression for insert. This is usually the
     *         stored sqlinsert expression with VSCP decision matrix escapes
     *          inserted and value and date/time inserted.
     * @return true on success, false if failure.
     */

    bool logData(const std::string &strInsert);

    /*!
     * Clear the content of a VSCP table
     *
     * @return true om success, false on failure.
     */

    bool clearTable(void);

    /*!
        Clear a date range of records in a table

       @param start Start of range to delete.
       @param end End of range to delete.
       @return true om success, false on failure.
     */
    bool clearTableRange(vscpdatetime &start, vscpdatetime &end);

    /*!
     * Execute SQL expression
     *
     * @param sql SQL expression to execute.
     * @return True on success, false on failure.
     */

    bool executeSQL(std::string &sql);

    /*!
     * Prepare to get a range of rows with data (columns)
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param ppStmt Handle to range.
     * @param bAll If true all columns will be returned. If false (default)
     *              only value and datetime will be returned.
     * @return True on success, false on failure.
     *
     */

    bool prepareRangeOfData(vscpdatetime &start,
                            vscpdatetime &end,
                            sqlite3_stmt **ppStmt,
                            bool bAll = false);

    /*!
     * Prepare to get ALL rows with data (columns)
     *
     * @param ppStmt Handle to range.
     * @param bAll If true all columns will be returned. If false (default)
     *              only value and datetime will be returned.
     * @return True on success, false on failure.
     *
     */
    bool prepareRangeOfData(sqlite3_stmt **ppStmt, bool bAll = false);

    /*!
     * Prepare to get a range of rows with data (columns)
     *
     * @param sql USer supplied SQL expression
     * @param ppStmt Handle to range.
     * @return True on success, false on failure.
     */

    bool prepareRangeOfData(std::string &sql, sqlite3_stmt **ppStmt);

    /*!
     * Get row of data as a comma separated string. datetime,value will alway be
     * the first two records
     *
     * @param ppStmt Handle to range.
     * @param rowData Comma separated list with values.
     * @return True on success, false on failure.
     *
     */
    bool getRowRangeOfData(sqlite3_stmt *ppStmt, std::string &rowData);

    /*!
     * Get row of data in a table item.
     *
     * @param ppStmt Handle to range.
     * @param vscpTableItem Item data.
     * @return True on success, false on failure.
     *
     */

    bool getRowRangeOfData(sqlite3_stmt *ppStmt, vscpTableItem *pTableItem);

    /*!
     * Get row of data as a comma separated string. Row will have all fields
     *
     * @param ppStmt Handle to range.
     * @param rowData Comma separated list with values.
     * @return True on success, false on failure.
     *
     */
    bool getRowRangeOfDataRaw(sqlite3_stmt *ppStmt, std::string &rowData);

    /*!
     * Finalise the range of data set.
     *
     * @param ppStmt Handle to range.
     * @return True on success, false on failure.
     */
    bool finalizeRangeOfData(sqlite3_stmt *ppStmt);

    /*!
     * Get data for a range as a string array
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param strArray String array that will hold resulting data.
     * @return True on success, false on failure.
     *
     */
    bool getRangeInArray(vscpdatetime &start,
                         vscpdatetime &end,
                         std::deque<std::string> &strArray,
                         bool bAll = false);

    /*!
     * Get the first date/ime in table
     *
     * @param dt Date time for first date in table.
     * @return true on success false on failure.
     */

    bool getFirstDate(vscpdatetime &dt);

    /*!
     * Get the last date/time in table
     *
     * @param dt Date time for last date time in table.
     * @return true on success false on failure.
     */

    bool getLastDate(vscpdatetime &dt);

    /*!
     * Get number of records in database
     *
     * @param pCount Pointer to uint32_t which will get the total number of
     *          records in the table.
     * @return true on success, false on failure.
     *
     */

    bool getNumberOfRecords(uint32_t *pCount);

    /*!
     * Get a double value from a SQL expression. HELPER
     *
     * @param sqltemplate Template SQL expression to execute. It should return a
     * numerical value as result 0. The template must contain two %s for the
     * dates that specifies the range.
     * @param from date/time from which data should be fetched.
     * @param to date/time to which data should be fetched.
     * @param pValue Pointer to double that will receive result.
     * @return true on success, false on failure.
     */
    bool getSQLValue(const std::string &sqltemplate,
                     vscpdatetime &start,
                     vscpdatetime &end,
                     double *pValue);

    /*!
        Get number of records for a range of data
        @param start Datetime from which data should be searched.
        @param end Datetime to which data should be searched.
        @param pCount Pointer to double that will get count.
        @return true on success, false on failure.
    */

    bool getNumberOfRecordsForRange(vscpdatetime &start,
                                    vscpdatetime &end,
                                    double *pCount);

    /*!
     * Get sum value  from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pSum Pointer to double that will receive the sum value for
     *          the range.
     * @return true on success, false on failure.
     */

    bool getSumValue(vscpdatetime start, vscpdatetime end, double *pSum);

    /*!
     * Get min value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pMedian Pointer to double that will receive the min value for
     *          the range.
     * @return true on success, false on failure.
     */

    bool getMinValue(vscpdatetime start, vscpdatetime end, double *pMin);

    /*!
     * Get max value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pMedian Pointer to double that will receive the max value for
     *          the range.
     * @return true on success, false on failure.
     */

    bool getMaxValue(vscpdatetime start, vscpdatetime end, double *pMax);

    /*!
     * Calculate mean over a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pMedian Pointer to double that will receive the avarage for the
     * range.
     * @return true on success, false on failure.
     */

    bool getAverageValue(vscpdatetime start,
                         vscpdatetime end,
                         double *pAvarage);

    /*!
     * Get median value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pMedian Pointer to double that will receive the median for the
     * range.
     * @return true on success, false on failure.
     */

    bool getMedianValue(vscpdatetime start, vscpdatetime end, double *pMedian);

    /*!
     * Get standard deviation value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pStdev Pointer to double that will receive the standard deviation
     * for the range.
     * @return true on success, false on failure.
     */

    bool getStdevValue(vscpdatetime start, vscpdatetime end, double *pStdev);

    /*!
     * Get variance value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pVariance Pointer to double that will receive the variance for the
     * range.
     * @return true on success, false on failure.
     */

    bool getVarianceValue(vscpdatetime start,
                          vscpdatetime end,
                          double *pVariance);

    /*!
     * Get mode value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pMode Pointer to double that will receive the mode for the range.
     * @return true on success, false on failure.
     */

    bool getModeValue(vscpdatetime start, vscpdatetime end, double *pMode);

    /*!
     * Get lower quartile value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pLowerQuartile Pointer to double that will receive the mode for
     * the range.
     * @return true on success, false on failure.
     */

    bool getLowerQuartileValue(vscpdatetime start,
                               vscpdatetime end,
                               double *pLowerQuartile);

    /*!
     * Get upper quartile value from a range of values
     *
     * @param start Datetime from which data should be searched.
     * @param end Datetime to which data should be searched.
     * @param pUpperQuartile Pointer to double that will receive the mode for
     * the range.
     * @return true on success, false on failure.
     */

    bool getUppeQuartileValue(vscpdatetime start,
                              vscpdatetime end,
                              double *pUpperQuartile);

    /*!
     *  Set path to database
     *  @param dbpath Pointer to null terminated string with path to
     *          database location or where database should b created.
     *  @return true on success, false on failure.
     */

    bool setPathToDB(const std::string &dbpath)
    {
        m_dbFile = dbpath;
        return vscp_fileExists(dbpath);
    };

    // Get path to database
    std::string getPathToDB(void) { return m_dbFile; };

    bool isEnabled(void) { return m_bEnable; };

    bool isInMemory(void) { return m_bInMemory; };

    // Get the table name
    std::string &getTableName(void) { return m_tableName; };

    CUserItem *getUserItem(void) { return m_pUserItemOwner; };

    /*!
     * Get user id
     */
    long getUserID(void)
    {
        if (NULL == m_pUserItemOwner) {
            return -1;
        }

        return m_pUserItemOwner->getUserID();
    };

    uint16_t getRights(void) { return m_rights; };

    vscpTableType getType(void) { return m_type; };

    uint32_t getSize(void) { return m_size; };

    std::string &getLabelX(void) { return m_labelX; };

    std::string &getLabelY(void) { return m_labelY; };

    std::string &getTitle(void) { return m_labelTitle; };

    std::string &getNote(void) { return m_labelNote; };

    std::string &getDescription(void) { return m_tableDescription; };

    // Get SQL create expression
    std::string &getSQLCreate(void) { return m_sqlCreate; };

    // Get SQL insert expression
    std::string &getSQLInsert(void) { return m_sqlInsert; };

    // Get SQL delete expression
    std::string &getSQLDelete(void) { return m_sqlDelete; };

    uint16_t getVSCPClass(void) { return m_vscp_class; };

    uint16_t getVSCPType(void) { return m_vscp_type; };

    uint8_t getVSCPSensorIndex(void) { return m_vscp_sensorindex; };

    uint8_t getVSCPUnit(void) { return m_vscp_unit; };

    uint8_t getVSCPZone(void) { return m_vscp_zone; };

    uint8_t getVSCPSubZone(void) { return m_vscp_subzone; };

  public:
    /// Protector for this table
    pthread_mutex_t m_mutexThisTable;

    /// List with table columns
    std::deque<std::string> m_listColumns;

  private:
    /// Enable flag
    bool m_bEnable;

    /// Path to database file
    std::string m_dbFile;

    /// Database
    sqlite3 *m_dbTable;

    /// Root folder for tables
    std::string m_folder;

    // Name for table.
    std::string m_tableName;

    // Flag for in memory database
    bool m_bInMemory;

    // Table type
    vscpTableType m_type;

    // Size for table with constant size (zero for ever growing)
    uint32_t m_size;

    // Table description
    std::string m_tableDescription;

    // Diagram title
    std::string m_labelTitle;

    // Diagram note
    std::string m_labelNote;

    // Label for X-axis
    std::string m_labelX;

    // Label for y-axis
    std::string m_labelY;

    // SQL to create table.
    std::string m_sqlCreate;

    // SQL to insert value.
    std::string m_sqlInsert;

    /// True if '%f' is places before '%s' in insert string
    bool m_bValueFirst;

    // SQL to delete table.
    std::string m_sqlDelete;

    std::string m_strOwner; // owner item is not available when config is read
                            // so it is looked up from name

    // Owner
    CUserItem *m_pUserItemOwner;

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

/******************************************************************************/
/*                           CUserTableObjList                                */
/******************************************************************************/

// Class that holds all user defined VSCP tables

class CUserTableObjList
{

  public:
    /// Constructor
    CUserTableObjList(void);

    /// Destructor
    virtual ~CUserTableObjList(void);

    /*!
     * Add table
     *
     * @param pTable Table object to add.
     * @return True on success and false on failure.
     *
     */
    bool addTable(CVSCPTable *pTable);

    /*!
     * Create a table from a semicolon separated string expression
     *
     * @param strCreate String used to create table.
     * @return True on success and false on failure.
     *
     * Elements below
     *      name = "test_table2"
     *      benable = "true"
     *      binmemory = "false"
     *      type = "dynamic"
     *      size = "0"
     *      owner = "admin"
     *      rights = "0x777"
     *      title = "This is a title"
     *      xname = "This the x-lable"
     *      yname = "This is the y-label"
     *      note = "This is a note"
     *      sqlcreate = "SQL create expression"
     *      sqlinsert = "SQL insert expression"
     *      sqldelete = "SQL delete expression"
     *      description = "This is the description"
     *      vscpclass = "10"
     *      vscptype = "6"
     *      sensorindex = "0"
     *      unit = "1"
     *      zone = "0"
     *      subzone = "0"
     */
    bool createTableFromString(const std::string &strCreate);

    /*!
     * Create one or more tables from a XML expression
     *
     * @param strCreateXML XML expression used to create table.
     * @return True on success and false on failure.
     *
     * <tables>
     * <table
     *      name = "test_table2"
     *      benable = "true"
     *      binmemory = "false"
     *      type = "dynamic"
     *      size = "0"
     *      owner = "admin"
     *      rights = "0x777"
     *      title = "This is a title"
     *      xname = "This the x-lable"
     *      yname = "This is the y-label"
     *      note = "This is a note"
     *      sqlcreate = "SQL create expression"
     *      sqlinsert = "SQL insert expression"
     *      sqldelete = "SQL delete expression"
     *      description = "This is the description"
     *      vscpclass = "10"
     *      vscptype = "6"
     *      sensorindex = "0"
     *      unit = "1"
     *      zone = "0"
     *      subzone = "0"
     * />
     * </tables>
     */
    bool createTableFromXML(const std::string &strCreateXML);

    /*!
     * Get user table from name
     *
     * @param name Name of table to lookup
     * @return Pointer to table on success, NULL on failure.
     *
     */
    CVSCPTable *getTable(std::string &name);

    /*!
     * Check if a name is used.
     *
     * @param name Name to search for
     * @return True if name is used, false otherwise.
     *
     */

    bool isNameUsed(std::string &name);

    /*!
     * Remove one table item from the internal structure
     *
     * @param name Name of table
     * @param bRemoveFile If true the database file is deleted
     * @return True on success, false on failure.
     */
    bool removeTable(std::string &name, bool bRemoveFile = true);

    /*!
     * Remove all items in the table
     */
    void clearTable(void);

    /*!
     * Initialize all tables
     *
     * @return True on success, false on failure.
     *
     */
    bool init(void);

    /*!
     * Check if a record with a table name exist or not
     *
     * @param table Table record
     * @return True on success, false on failure.
     *
     */
    bool isTableInDB(CVSCPTable &table)
    {
        return isTableInDB(table.getTableName());
    };

    /*!
     * Check if a record with a table name exist or not
     *
     * @param name Name of table to search
     * @return True on success, false on failure.
     *
     */
    bool isTableInDB(std::string &name);

    /*!
     * Add a new table to the table database-
     *
     * @param table VSCP table to add.
     * @return True on success, false on failure.
     *
     */

    bool addTableToDB(CVSCPTable &table);

    /*!
     * Update data for table
     *
     * @param table VSCP table to update.
     * @return True on success, false on failure.
     */
    bool updateTableToDB(CVSCPTable &table);

    /*!
     * Remove a VSCP table from the database
     *
     * @param name Name of table.
     * @return True on success, false on failure.
     *
     */

    bool removeTableFromDB(std::string &name);

    /*!
     * Load tables definitions from database into
     * internal structure.
     */

    bool loadTablesFromDB(void);

    /*!
     * Get list of tablenames
     * @param Reference to string array
     * @return True on success, false on failure.
     */
    bool getTableNames(std::deque<std::string> &arrayNames);

    // Pointer to main object
    CControlObject *m_pobj;

    /// Table list
    std::deque<CVSCPTable *> m_listTables;

    /// Mutex for table list
    pthread_mutex_t m_mutexTableList;
};

#endif
