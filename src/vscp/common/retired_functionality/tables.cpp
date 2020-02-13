// tables.cpp
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

#ifdef __GNUG__
//#pragma implementation
#endif

#define _POSIX

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>

#include <expat.h>
#include <sqlite3.h>

#include <controlobject.h>
#include <vscpdatetime.h>
#include <variablecodes.h>
#include <vscp.h>
#include <vscpdb.h>
#include <vscphelper.h>

#include "tables.h"

// From sqlite3 math extensions  sqlite3math.c
#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */
    int RegisterExtensionFunctions(sqlite3 *db);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

#define XML_BUFF_SIZE 100000 // XML parseer buffer

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

// Constructor
CVSCPTable::CVSCPTable(const std::string &folder,
                       const std::string &name,
                       const bool bEnable,
                       const bool bInMemory,
                       const vscpTableType type,
                       uint32_t size)
{

    m_pUserItemOwner = NULL; // No owner yet
    m_dbTable        = NULL; // No database handle yet

    m_bValueFirst = true; // %f is placed before %s in insert

    // Initialize event data
    m_vscp_class       = 0;
    m_vscp_type        = 0;
    m_vscp_sensorindex = 0;
    m_vscp_unit        = 0;
    m_vscp_zone        = 0;
    m_vscp_subzone     = 0;

    m_folder    = folder;    // save table root folder
    m_tableName = name;      // save table name
    m_bEnable   = bEnable;   // save enable state
    m_bInMemory = bInMemory; // save table in memory flag
    m_type      = type;      // save table type
    m_size      = size;      // save table size

    pthread_mutex_init(&m_mutexThisTable, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPTable::~CVSCPTable(void)
{
    // Close the database if it is open
    if (NULL != m_dbTable) {
        sqlite3_close(m_dbTable);
        m_dbTable = NULL;
    }

    pthread_mutex_destroy(&m_mutexThisTable);
}

///////////////////////////////////////////////////////////////////////////////
// setTableInfo
//

bool
CVSCPTable::setTableInfo(const std::string &owner,
                         const uint16_t rights,
                         const std::string &title,
                         const std::string &xname,
                         const std::string &yname,
                         const std::string &note,
                         const std::string &sqlcreate,
                         const std::string &sqlinsert,
                         const std::string &sqldelete,
                         const std::string &description)
{
    // Owner must have a value
    if (0 == owner.length()) return false;
    m_strOwner = owner;

    if (0 == sqlcreate.length()) {
        m_sqlCreate = (VSCPDB_TABLE_DEFAULT_CREATE);
    } else {
        m_sqlCreate = sqlcreate;
    }

    if (0 == sqlinsert.length()) {
        m_sqlInsert = (VSCPDB_TABLE_DEFAULT_INSERT);
    } else {
        m_sqlInsert = sqlinsert;
    }

    if (0 == sqldelete.length()) {
        m_sqlDelete = (VSCPDB_TABLE_DEFAULT_DELETE);
    } else {
        m_sqlDelete = sqldelete;
    }

    // the sqlinsert expression must have %f and %s in it for
    // value and date. It s invalid if not
    int posValue;
    int posDate;
    if ((m_sqlInsert.npos == (posValue = m_sqlInsert.find(("%f")))) ||
        (m_sqlInsert.npos == (posDate = m_sqlInsert.find(("%s"))))) {
        return false;
    }

    if (posValue < posDate) {
        m_bValueFirst = true;
    } else {
        m_bValueFirst = false;
    }

    m_rights     = rights;
    m_labelTitle = title;
    m_labelX     = xname;
    m_labelY     = yname;
    m_labelNote  = note;

    m_tableDescription = description;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setTableEventInfo
//

void
CVSCPTable::setTableEventInfo(uint16_t vscp_class,
                              uint16_t vscp_type,
                              uint8_t vscp_sensorindex,
                              uint8_t vscp_unit,
                              uint8_t vscp_zone,
                              uint8_t vscp_subzone)
{
    m_vscp_class       = vscp_class;
    m_vscp_type        = vscp_type;
    m_vscp_sensorindex = vscp_sensorindex;
    m_vscp_unit        = vscp_unit;
    m_vscp_zone        = vscp_zone;
    m_vscp_subzone     = vscp_subzone;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

bool
CVSCPTable::init()
{
    // Get useritem
    m_pUserItemOwner = gpobj->m_userList.getUser(m_strOwner);
    if (NULL == m_pUserItemOwner) return false;

    m_dbFile = gpobj->m_rootFolder + ("tables/"), m_tableName, ("sqlite3");

    // Check filename
    if (!m_bInMemory && vscp_fileExists(m_dbFile)) {

        // File name OK - Open database

        if (SQLITE_OK !=
            sqlite3_open((const char *)m_dbFile.c_str(), &m_dbTable)) {

            // Failed to open/create the database file
            std::string str;
            syslog(LOG_ERR, "VSCP table database could not be opened.");
            syslog(LOG_ERR,
                   "Path=%s error=%s",
                   (const char *)m_dbFile.c_str(),
                   sqlite3_errmsg(m_dbTable));
            if (NULL != m_dbTable) sqlite3_close(m_dbTable);
            m_dbTable = NULL;
            return false;
        }

    } else {

        // File does not exist (and should be created) or something else is
        // wrong. Can also be in-memory db which always should be created,

        if (m_bInMemory) {

            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            std::string str;
            syslog(LOG_ERR,
                   "VSCP table database does not exist - will be created.");
            syslog(LOG_ERR, "Path=%s\n", (const char *)m_dbFile.c_str());

            if (SQLITE_OK == sqlite3_open(m_bInMemory
                                            ? ":memory:"
                                            : (const char *)m_dbFile.c_str(),
                                          &m_dbTable)) {
                // create the database.
                if (!createDatabase()) {
                    syslog(LOG_ERR,
                           "Failed to create VSCP table database structure.\n");
                    sqlite3_close(m_dbTable);
                    m_dbTable = NULL;
                    return false;
                }
            } else {
                syslog(LOG_ERR, "Failed to create VSCP table database.\n");
                return false;
            }

        } else {
            std::string str;
            syslog(LOG_ERR,
                   "VSCP table path/filename is invalid.\n"
                   "Path=%s",
                   (const char *)m_dbFile.c_str());
            return false;
        }
    }

    // Add math & statistical functions
    // sqlite3_enable_load_extension( m_dbTable, 1 );
    RegisterExtensionFunctions(m_dbTable);

    // Now find and save the columns of the table
    int rc;

    sqlite3_stmt *ppStmt;
    rc = sqlite3_prepare_v2(
      m_dbTable, VSCPDB_TABLE_GET_COLUMNS, -1, &ppStmt, NULL);

    if (SQLITE_OK == rc) {

        while (SQLITE_ROW == sqlite3_step(ppStmt)) {

            const unsigned char *p = sqlite3_column_text(ppStmt, 1);
            std::string tblColumn  = std::string((const char *)p);
            m_listColumns.push_back(tblColumn);
            syslog(LOG_ERR, "User table column: %s", tblColumn.c_str());
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isThisEventForMe
//

bool
CVSCPTable::isThisEventForMe(vscpEvent *pEvent)
{
    int offset = 0;

    // Check pointers
    if (NULL == pEvent) return false;

    // If class >= 512 and class < 1024 we
    // have GUID in front of data.
    if ((pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL) &&
        (pEvent->vscp_class < VSCP_CLASS2_PROTOCOL)) {
        offset = 16;
    }

    if ((VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class) ||
        (VSCP_CLASS2_LEVEL1_MEASUREMENT == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {
            return true;
        }

    } else if ((VSCP_CLASS1_MEASUREZONE == pEvent->vscp_class) ||
               (VSCP_CLASS2_LEVEL1_MEASUREZONE == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {

            uint8_t zone    = vscp_getMeasurementZone(pEvent);
            uint8_t subzone = vscp_getMeasurementSubZone(pEvent);
            if (((m_vscp_zone == zone) || (255 == zone)) &&
                ((m_vscp_subzone == subzone) || (255 == subzone))) {
                return true;
            }
        }

    } else if ((VSCP_CLASS1_SETVALUEZONE == pEvent->vscp_class) ||
               (VSCP_CLASS2_LEVEL1_SETVALUEZONE == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {

            uint8_t zone    = vscp_getMeasurementZone(pEvent);
            uint8_t subzone = vscp_getMeasurementSubZone(pEvent);
            if (((m_vscp_zone == zone) || (255 == zone)) &&
                ((m_vscp_subzone == subzone) || (255 == subzone))) {
                return true;
            }
        }

    } else if ((VSCP_CLASS1_MEASUREMENT32 == pEvent->vscp_class) ||
               (VSCP_CLASS2_LEVEL1_MEASUREMENT32 == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {
            return true;
        }

    } else if ((VSCP_CLASS1_MEASUREMENT64 == pEvent->vscp_class) ||
               (VSCP_CLASS2_LEVEL1_MEASUREMENT64 == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {
            return true;
        }

    } else if ((VSCP_CLASS2_MEASUREMENT_STR == pEvent->vscp_class) ||
               (VSCP_CLASS2_MEASUREMENT_FLOAT == pEvent->vscp_class)) {

        if ((m_vscp_sensorindex ==
             vscp_getMeasurementSensorIndex(pEvent)) &&
            (m_vscp_unit == vscp_getMeasurementUnit(pEvent))) {

            uint8_t zone    = vscp_getMeasurementZone(pEvent);
            uint8_t subzone = vscp_getMeasurementSubZone(pEvent);
            if (((m_vscp_zone == zone) || (255 == zone)) &&
                ((m_vscp_subzone == subzone) || (255 == subzone))) {
                return true;
            }
        }

    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cleanUpAndClose
//

void
CVSCPTable::cleanUpAndClose(void)
{
    // Close the database if it is open
    if (NULL != m_dbTable) {
        sqlite3_close(m_dbTable);
        m_dbTable = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// createDatabase
//

bool
CVSCPTable::createDatabase(void)
{
    char *pErrMsg = 0;

    syslog(LOG_ERR, "Creating database for table.");

    // Check if database is open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "Failed to create VSCP log database - closed.");
        return false;
    }

    pthread_mutex_lock(&m_mutexThisTable);

    if (SQLITE_OK !=
        sqlite3_exec(
          m_dbTable, (const char *)m_sqlCreate.c_str(), NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP log table with error %s.", pErrMsg);
        pthread_mutex_unlock(&m_mutexThisTable);
        return false;
    }

    pthread_mutex_unlock(&m_mutexThisTable);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(double jdn, double value, int ms)
{
    vscpdatetime tobj(jdn);
    return logData(tobj, value);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(const struct tm &tm, double value, int ms)
{
    vscpdatetime tobj(tm);
    return logData(tobj, value);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(time_t time, double value, int ms)
{
    vscpdatetime tobj(time);
    return logData(tobj, value);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(std::string &strtime, double value)
{
    vscpdatetime tobj;
    tobj.set(strtime);
    return logData(tobj, value);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(vscpdatetime &time, double value)
{
    std::string strInsert = m_sqlInsert; // template for sql

    // Milliseconds
    long ms; // Milliseconds
#ifdef WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);
    ms = st.wMilliseconds;
#else
    time_t s; // Seconds
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
#endif
    // int ms = vscpdatetime::Now().GetMillisecond();   !!!!! Does not work
    // !!!!!
    std::string msstr = vscp_str_format(".%d", ms);

    std::string dtstamp = time.getISODateTime() + msstr;
    if (m_bValueFirst) {
        strInsert = vscp_str_format(m_sqlInsert, value, dtstamp.c_str());
    } else {
        strInsert = vscp_str_format(m_sqlInsert, dtstamp.c_str(), value);
    }

    return logData(strInsert);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(vscpdatetime &dt,
                    double value,
                    const std::string &sqlInsert)
{
    std::string strInsertMod;

    // Milliseconds
    long ms; // Milliseconds
#ifdef WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);
    ms = st.wMilliseconds;
#else
    time_t s;                         // Seconds
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
#endif

    std::string msstr   = vscp_str_format((".%d"), ms);
    std::string dtstamp = dt.getISODateTime() + msstr;

    if (m_bValueFirst) {
        strInsertMod = vscp_str_format(sqlInsert, value, dtstamp.c_str());
    } else {
        strInsertMod = vscp_str_format(sqlInsert, dtstamp.c_str(), value);
    }

    return logData(strInsertMod);
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool
CVSCPTable::logData(const std::string &strInsert)
{
    char *zErrMsg = 0;

    // Database file must be open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, ("VSCP database file is not open.\n"));
        return false;
    }

    // If max type and the table contains max number of records it should be
    // cleared before new data is added
    uint32_t nRecords = 0;
    getNumberOfRecords(&nRecords);
    if (nRecords && (VSCP_TABLE_MAX == m_type) && (nRecords >= m_size)) {
        if (SQLITE_OK !=
            sqlite3_exec(
              m_dbTable, VSCPDB_TABLE_DEFAULT_DELETE, NULL, NULL, &zErrMsg)) {
            syslog(LOG_ERR,
                   "Add table item: Unable to delete items "
                   "in db [VSCP_TABLE_MAX]. [%s] Err=%s",
                   strInsert.c_str(),
                   zErrMsg);
            return false;
        }
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_dbTable, strInsert.c_str(), NULL, NULL, &zErrMsg)) {
        syslog(LOG_ERR,
               "Add table item: Unable to insert table item "
               "in db. [%s] Err=%s",
               strInsert.c_str(),
               zErrMsg);
        return false;
    }

    // If the table is a static table we may need to delete the oldest records
    // here to keep the table size
    if ((VSCP_TABLE_STATIC == m_type) && m_size) {
        std::string strDel =
          vscp_str_format(VSCPDB_TABLE_DELETE_STATIC, (unsigned long)m_size);
        if (SQLITE_OK !=
            sqlite3_exec(
              m_dbTable, (const char *)strDel.c_str(), NULL, NULL, &zErrMsg)) {
            syslog(LOG_ERR,
                   "Add table item: Unable to delete records from "
                   "static table. [%s] Err=%s",
                   (const char *)strDel.c_str(),
                   zErrMsg);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// clearTable
//

bool
CVSCPTable::clearTable(void)
{
    std::string str = std::string(VSCPDB_TABLE_DEFAULT_DELETE);
    return executeSQL(str);
}

///////////////////////////////////////////////////////////////////////////////
// clearTableRange
//

bool
CVSCPTable::clearTableRange(vscpdatetime &start, vscpdatetime &end)
{
    // Range must be valid
    if (!start.isValid() || !end.isValid()) return false;

    std::string str =
      (VSCPDB_TABLE_DELETE_RANGE, start.getISODateTime(), end.getISODateTime());
    return executeSQL(str);
}

///////////////////////////////////////////////////////////////////////////////
// executeSQL
//

bool
CVSCPTable::executeSQL(std::string &sql)
{
    char *zErrMsg = 0;

    // Database file must be open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, ("VSCP database file is not open."));
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_dbTable, sql.c_str(), NULL, NULL, &zErrMsg)) {
        syslog(LOG_ERR,
               "Table: Unable to delete "
               "records of table. [%s] Err=%s",
               sql.c_str(),
               zErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getFirstDate
//

bool
CVSCPTable::getFirstDate(vscpdatetime &dt)
{
    bool rv = true;
    std::string str;
    sqlite3_stmt *ppStmt;

    // Check if database is open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "Database is closed.");
        return false;
    }

    if (SQLITE_OK != sqlite3_prepare(
                       m_dbTable, VSCPDB_TABLE_DATE_FIRST, -1, &ppStmt, NULL)) {

        syslog(LOG_ERR,
               "Load tables: Error=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)));
        return false;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        const unsigned char *p =
          sqlite3_column_text(ppStmt, 0); // Get first date
        if (NULL != p) {
            if (!dt.set(std::string((const char *)p))) {
                rv = false;
            }
        } else {
            rv = false;
        }
    } // While

    sqlite3_finalize(ppStmt);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getLastDate
//

bool
CVSCPTable::getLastDate(vscpdatetime &dt)
{
    bool rv = true;
    std::string str;
    sqlite3_stmt *ppStmt;

    // Check if database is open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "Database is closed.\n");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_dbTable, VSCPDB_TABLE_DATE_LAST, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "Load tables: Error=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)));
        return false;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        const unsigned char *p =
          sqlite3_column_text(ppStmt, 0); // Get first date
        if (NULL != p) {
            if (!dt.set(std::string((char *)p))) {
                rv = false;
            }
        } else {
            rv = false;
        }
    } // While

    sqlite3_finalize(ppStmt);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getNumberOfRecords
//

bool
CVSCPTable::getNumberOfRecords(uint32_t *pCount)
{
    std::string str;
    sqlite3_stmt *ppStmt;

    // Check pointer
    if (NULL == pCount) return false;

    // Check if database is open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "Database is closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_dbTable, VSCPDB_TABLE_COUNT, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "Load tables: Error=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)));
        return false;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        *pCount = sqlite3_column_int(ppStmt, 0); // Get record count
    }                                            // While

    sqlite3_finalize(ppStmt);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getSQLValue
//

bool
CVSCPTable::getSQLValue(const std::string &sqltemplate,
                        vscpdatetime &start,
                        vscpdatetime &end,
                        double *pValue)
{
    std::string str;
    sqlite3_stmt *ppStmt;

    // Check pointer
    if (NULL == pValue) return false;

    // Database file must be open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "VSCP database file is not open.");
        return false;
    }

    std::string sql = vscp_str_format( sqltemplate.c_str(),
                       (const char *)start.getISODateTime(false).c_str(),
                       (const char *)end.getISODateTime(false).c_str());

    if (SQLITE_OK !=
        sqlite3_prepare(
          m_dbTable, (const char *)sql.c_str(), -1, &ppStmt, NULL)) {

        syslog(LOG_ERR,
               "Failed to execute SQL expression: Error=%s SQL=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)), sql.c_str());
        return false;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        *pValue = sqlite3_column_double(ppStmt, 0);
    }

    sqlite3_finalize(ppStmt);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getNumberOfRecordsForRange
//

bool
CVSCPTable::getNumberOfRecordsForRange(vscpdatetime &start,
                                       vscpdatetime &end,
                                       double *pCount)
{
    return getSQLValue((VSCPDB_TABLE_COUNT_RANGE), start, end, pCount);
}

///////////////////////////////////////////////////////////////////////////////
// getSumValue
//

bool
CVSCPTable::getSumValue(vscpdatetime start, vscpdatetime end, double *pSum)
{
    return getSQLValue((VSCPDB_TABLE_GET_SUM), start, end, pSum);
}

///////////////////////////////////////////////////////////////////////////////
// getMinValue
//

bool
CVSCPTable::getMinValue(vscpdatetime start, vscpdatetime end, double *pMin)
{
    return getSQLValue((VSCPDB_TABLE_GET_MIN), start, end, pMin);
}

///////////////////////////////////////////////////////////////////////////////
// getMaxValue
//

bool
CVSCPTable::getMaxValue(vscpdatetime start, vscpdatetime end, double *pMax)
{
    return getSQLValue((VSCPDB_TABLE_GET_MAX), start, end, pMax);
}

///////////////////////////////////////////////////////////////////////////////
// getAverageValue
//

bool
CVSCPTable::getAverageValue(vscpdatetime start,
                            vscpdatetime end,
                            double *pAvarage)
{
    return getSQLValue((VSCPDB_TABLE_GET_AVG), start, end, pAvarage);
}

////////////////////////////////////////////////////////////////////////////
// getMedianValue
//

bool
CVSCPTable::getMedianValue(vscpdatetime start,
                           vscpdatetime end,
                           double *pMedian)
{
    return getSQLValue((VSCPDB_TABLE_GET_MEDIAN), start, end, pMedian);
}

////////////////////////////////////////////////////////////////////////////
// getStdevValue
//

bool
CVSCPTable::getStdevValue(vscpdatetime start, vscpdatetime end, double *pStdev)
{
    return getSQLValue((VSCPDB_TABLE_GET_STDDEV), start, end, pStdev);
}

////////////////////////////////////////////////////////////////////////////
// getVarianceValue
//

bool
CVSCPTable::getVarianceValue(vscpdatetime start,
                             vscpdatetime end,
                             double *pVariance)
{
    return getSQLValue((VSCPDB_TABLE_GET_VARIANCE), start, end, pVariance);
}

////////////////////////////////////////////////////////////////////////////
// getModeValue
//

bool
CVSCPTable::getModeValue(vscpdatetime start, vscpdatetime end, double *pMode)
{
    return getSQLValue((VSCPDB_TABLE_GET_MODE), start, end, pMode);
}

////////////////////////////////////////////////////////////////////////////
// getLowerQuartileValue
//

bool
CVSCPTable::getLowerQuartileValue(vscpdatetime start,
                                  vscpdatetime end,
                                  double *pLowerQuartile)
{
    return getSQLValue(
      (VSCPDB_TABLE_GET_LOWER_QUARTILE), start, end, pLowerQuartile);
}

////////////////////////////////////////////////////////////////////////////
// getUppeQuartileValue
//

bool
CVSCPTable::getUppeQuartileValue(vscpdatetime start,
                                 vscpdatetime end,
                                 double *pUpperQuartile)
{
    return getSQLValue(
      (VSCPDB_TABLE_GET_UPPER_QUARTILE), start, end, pUpperQuartile);
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool
CVSCPTable::prepareRangeOfData(vscpdatetime &start,
                               vscpdatetime &end,
                               sqlite3_stmt **ppStmt,
                               bool bAll)
{
    std::string sqlInsert;
    std::string str;

    // Database file must be open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "VSCP database file is not open.");
        return false;
    }

    if (bAll) {
        sqlInsert = (VSCPDB_TABLE_SELECT_STANDARD_RANGE);
    } else {
        sqlInsert = (VSCPDB_TABLE_SELECT_CUSTOM_RANGE);
    }

    std::string sql = (sqlInsert.c_str(),
                       (const char *)start.getISODateTime(' ').c_str(),
                       (const char *)end.getISODateTime(' ').c_str());

    if (SQLITE_OK !=
        sqlite3_prepare(
          m_dbTable, (const char *)sql.c_str(), -1, ppStmt, NULL)) {

        syslog(LOG_ERR,
               "Failed to execute SQL expression: Error=%s SQL=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)),
               (const char *)sql.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool
CVSCPTable::prepareRangeOfData(sqlite3_stmt **ppStmt, bool bAll)
{
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set("0000-01-01T00:00:00"); // The first date
    end.set("9999-12-31T23:59:59");   // The last date

    return prepareRangeOfData(start, end, ppStmt, bAll);
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool
CVSCPTable::prepareRangeOfData(std::string &sql, sqlite3_stmt **ppStmt)
{
    std::string str;

    // Database file must be open
    if (NULL == m_dbTable) {
        syslog(LOG_ERR, "VSCP database file is not open.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(
          m_dbTable, (const char *)sql.c_str(), -1, ppStmt, NULL)) {

        syslog(LOG_ERR,
               "Failed to execute SQL expression: Error=%s SQL=%s",
               sqlite3_errstr(sqlite3_errcode(m_dbTable)),
               (const char *)sql.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfData
//

bool
CVSCPTable::getRowRangeOfData(sqlite3_stmt *ppStmt, std::string &rowData)
{
    if (SQLITE_ROW != sqlite3_step(ppStmt)) {
        return false;
    }

    rowData.clear();

    // Fill in the data
    int count = sqlite3_data_count(ppStmt);

    for (int i = 0; i < count; i++) {

        // Skip 'datetime' and 'value' they are in pos 0 and 1
        if (i > 1) {
            const char *pName = sqlite3_column_name(ppStmt, i);
            if (NULL != strstr(pName, "datetime")) continue;
            if (NULL != strstr(pName, "value")) continue;
        }

        const unsigned char *p = sqlite3_column_text(ppStmt, i);
        if (NULL != p) {
            rowData += std::string((const char *)p);
        }

        if (i < (count - 1)) rowData += ("|");
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfData
//

bool
getRowRangeOfData(sqlite3_stmt *ppStmt, vscpTableItem *pTableItem)
{
    // Check pointer
    if (NULL == pTableItem) {
        return false;
    }

    if (SQLITE_ROW != sqlite3_step(ppStmt)) {
        return false;
    }

    // datetime
    const unsigned char *p = sqlite3_column_text(ppStmt, 0);
    if (NULL == p) return false;
    pTableItem->date.set((const char *)p);

    // Value
    pTableItem->value = sqlite3_column_double(ppStmt, 1);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfDataRaw
//

bool
CVSCPTable::getRowRangeOfDataRaw(sqlite3_stmt *ppStmt, std::string &rowData)
{
    if (SQLITE_ROW != sqlite3_step(ppStmt)) {
        return false;
    }

    rowData.clear();

    // Fill in the data
    int count = sqlite3_data_count(ppStmt);

    for (int i = 0; i < count; i++) {

        const unsigned char *p = sqlite3_column_text(ppStmt, i);
        if (NULL != p) {
            rowData += std::string((const char *)p);
        }

        if (i < (count - 1)) rowData += ("|");
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// finalizeRangeOfData
//

bool
CVSCPTable::finalizeRangeOfData(sqlite3_stmt *ppStmt)
{
    return (SQLITE_OK == sqlite3_finalize(ppStmt));
}

///////////////////////////////////////////////////////////////////////////////
// getRangeInArray
//

bool
CVSCPTable::getRangeInArray(vscpdatetime &start,
                            vscpdatetime &end,
                            std::deque<std::string> &strArray,
                            bool bAll)
{
    sqlite3_stmt *ppStmt;

    if (!prepareRangeOfData(start, end, &ppStmt, bAll)) {
        return false;
    }

    std::string rowData;
    while (getRowRangeOfDataRaw(ppStmt, rowData)) {
        strArray.push_back(rowData);
    }

    return finalizeRangeOfData(ppStmt);
}

/******************************************************************************/
/*                           CUserTableObjList                                */
/******************************************************************************/

// Constructor
CUserTableObjList::CUserTableObjList(void)
{
    pthread_mutex_init(&m_mutexTableList, NULL);
}

// Destructor
CUserTableObjList::~CUserTableObjList(void)
{
    clearTable();

    pthread_mutex_destroy(&m_mutexTableList);
}

///////////////////////////////////////////////////////////////////////////////
// addTable
//

bool
CUserTableObjList::addTable(CVSCPTable *pTable)
{
    // Check pointer
    if (NULL == pTable) return false;

    if (isNameUsed(pTable->getTableName())) return false;

    pthread_mutex_lock(&m_mutexTableList);
    m_listTables.push_back(pTable);
    pthread_mutex_unlock(&m_mutexTableList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// createTableFromString
//

bool
CUserTableObjList::createTableFromString(const std::string &strCreate)
{
    std::string str;
    std::string strCreateXML = "<tables><table ";

    std::deque<std::string> tokens;
    vscp_split(tokens, strCreate, ";");

    // name = "test_table2"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "name = \"" + str + "\" ";

    // benable = "true"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "benable = \"" + str + "\" ";

    // binmemory = "false"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "binmemory = \"" + str + "\" ";

    // type = "dynamic"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "type = \"" + str + "\" ";

    // size = "0"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "size = \"" + str + "\" ";

    // owner = "admin"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "owner = \"" + str + "\" ";

    // rights = "0x777"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "rights = \"" + str + "\" ";

    // title = "This is a title"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "title = \"" + str + "\" ";

    // xname = "This the x-lable"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "xname = \"" + str + "\" ";

    // yname = "This is the y-label"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "yname = \"" + str + "\" ";

    // note = "This is a note"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "note = \"" + str + "\" ";

    // sqlcreate = "SQL create expression"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "sqlcreate = \"" + str + "\" ";

    // sqlinsert = "SQL insert expression"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "sqlinsert = \"" + str + "\" ";

    // sqldelete = "SQL delete expression"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "sqldelete = \"" + str + "\" ";

    // description = "This is the description"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "description = \"" + str + "\" ";

    // vscpclass = "10"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "vscpclass = \"" + str + "\" ";

    // vscptype = "6"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "vscptype = \"" + str + "\" ";

    // sensorindex = "0"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "sensorindex = \"" + str + "\" ";

    // unit = "1"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "unit = \"" + str + "\" ";

    // zone = "0"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "zone = \"" + str + "\" ";

    // subzone = "0"
    if (tokens.empty()) {
        return false;
    }

    str = tokens.front();
    tokens.pop_front();
    strCreateXML += "subzone = \"" + str + "\" ";

    strCreateXML += "/></tables>";

    return createTableFromXML(strCreateXML);
}

// ----------------------------------------------------------------------------

static int depth_table_xml_parser = 0;

static void
startTableXmlParser(void *data, const char *name, const char **attr)
{
    CControlObject *pobj = (CControlObject *)data;
    if (NULL == data) return;

    if (0 == depth_table_xml_parser) {
        ;
    }
    if (1 == depth_table_xml_parser) {

        if (0 == vscp_strcasecmp(name, "table")) {

            std::string strName;
            bool bEnable       = true;
            bool bInMemory     = false;
            vscpTableType type = VSCP_TABLE_DYNAMIC;
            uint32_t size      = 0;
            std::string strOwner;
            uint16_t rights;
            std::string strTitle;
            std::string strXname;
            std::string strYname;
            std::string strNote;
            std::string strSqlCreate;
            std::string strSqlInsert;
            std::string strSqlDelete;
            std::string strDescription;
            uint8_t vscpclass = 0;
            uint8_t vscptype = 0;
            uint8_t sensorindex = 0;
            uint8_t unit        = 0;
            uint8_t zone        = 255;
            uint8_t subzone     = 255;

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);
                if ((0 == vscp_strcasecmp(attr[i], "enable")) || (0 == vscp_strcasecmp(attr[i], "benable"))) {
                    vscp_trim(attribute);
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false") ) {
                        bEnable = false;
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "name")) {
                    vscp_trim(attribute);
                    strName = attribute;
                    if ( 0 == strName.length() ) {
                        syslog(LOG_ERR,"createTableFromXML: Table has no name.");
                        return;
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "bmemory")) {
                    vscp_trim(attribute);
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "true") ) {
                        bEnable = true;
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "type")) {
                    vscp_trim(attribute);
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "static") ) {
                        type = VSCP_TABLE_STATIC;
                    }
                    else if ( 0 == vscp_strcasecmp( attribute.c_str(), "dynamic") ) {
                        type = VSCP_TABLE_DYNAMIC;
                    }
                    else if ( 0 == vscp_strcasecmp( attribute.c_str(), "max") ) {
                        type = VSCP_TABLE_MAX;
                    }
                    else {
                        syslog(LOG_ERR,"createTableFromXML: Table has unknow type, "
                        "setting type to 'dynamic' [%s].",attribute.c_str() );
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "size")) {
                    size = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "owner")) {
                    strOwner = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "permission")) {
                    rights = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "vscp-class")) {
                    vscpclass = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "vscp-type")) {
                    vscptype  = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "sensorindex")) {
                    sensorindex = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "vscp-unit")) {
                    unit = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "vscp-zone")) {
                    zone = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "vscp-subzone")) {
                    subzone = vscp_readStringValue( attribute );
                }
                else if (0 == vscp_strcasecmp(attr[i], "xname")) {
                    strXname = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "yname")) {
                    strYname = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "title")) {
                    strTitle = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "note")) {
                    strNote = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "sql-create")) {
                    strSqlCreate = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "sql-insert")) {
                    strSqlDelete = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "sql-delete")) {
                    strSqlDelete = attribute;
                }
                else if (0 == vscp_strcasecmp(attr[i], "description")) {
                    strDescription = attribute;
                }

            } // attr

            // Check if size value is valid for static table
            if ((VSCP_TABLE_STATIC == type) && (0 == size)) {
                syslog(LOG_ERR, "createTableFromXML: Invalid table size for "
                "static table. Can't be zero. '%s'", strName.c_str());
                return;
            }

            // Check if size value is valid for max table
            if ((VSCP_TABLE_MAX == type) && (0 == size)) {
                syslog(LOG_ERR, "createTableFromXML: Invalid table size for "
                "max table. Can't be zero. '%s'",strName.c_str());
                return;
            }

            // Create the table
            CVSCPTable *pTable =
              new CVSCPTable(pobj->m_rootFolder + ("table/"),
                             strName,
                             bEnable,
                             bInMemory,
                             type,
                             size);

            if (NULL == pTable) {
                syslog(LOG_ERR,
                       "createTableFromXML:: Failed to "
                       "allocate new table '%s'.",strName.c_str());
                return;
            }

            // Set table info
            if (!pTable->setTableInfo(strOwner,
                                      rights,
                                      strTitle,
                                      strXname,
                                      strYname,
                                      strNote,
                                      strSqlCreate,
                                      strSqlInsert,
                                      strSqlDelete,
                                      strDescription)) {

                delete pTable;
                syslog(LOG_ERR,
                       "createTableFromXML: Failed to set "
                       "table-infofor table '%s'.",strName.c_str());
                return;
            }

            // Set event info
            pTable->setTableEventInfo(
              vscpclass, vscptype, sensorindex, unit, zone, subzone);

            // Initialize the table
            if (!pTable->init()) {
                delete pTable;
                syslog(
                  LOG_ERR,
                  "createTableFromXML: Failed to initialize "
                  "table '%sä in system.",
                  strName.c_str());
                return;
            }

            // Add the table to the database
            if (!gpobj->m_userTableObjects.addTableToDB(*pTable)) {
                delete pTable;
                syslog(LOG_ERR,
                       "createTableFromXML: Failed to add table to database.");
                return;
            }

            // Add the table to the system table
            if (!gpobj->m_userTableObjects.addTable(pTable)) {
                delete pTable;
                syslog(LOG_ERR,
                       "createTableFromXML: Failed to add table to system.");
                return;
            }

        }
    }

    depth_table_xml_parser++;
}

static void
endTableXmlParser(void *data, const char *name)
{
    depth_table_xml_parser--;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// createTableFromXML
//

bool
CUserTableObjList::createTableFromXML(const std::string &strCreateXML)
{
    bool bFail = false;
    std::string str;

    std::string decoded = strCreateXML;
    vscp_base64_std_decode(decoded);

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, m_pobj);
    XML_SetElementHandler(xmlParser,
                          startTableXmlParser,
                          endTableXmlParser);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf)
    {
        XML_ParserFree(xmlParser);
        syslog(LOG_ERR,
               "createTableFromXML: Failed to allocate buffer.");
        return false;
    }

    memcpy( buf, decoded.c_str(), decoded.length() );
    size_t file_size = decoded.length();


    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0))
    {
        syslog(LOG_ERR,
               "createTableFromXML: Failed parse XML configuration file.");
        XML_ParserFree(xmlParser);
        return false;
    }

    XML_ParserFree(xmlParser);


    return true;
}

///////////////////////////////////////////////////////////////////////////////
// initAll
//

CVSCPTable *
CUserTableObjList::getTable(std::string &name)
{
    pthread_mutex_lock(&m_mutexTableList);
    std::deque<CVSCPTable *>::iterator it;
    for (it = m_listTables.begin(); it != m_listTables.end(); ++it) {

        CVSCPTable *pTable = *it;
        if ((NULL != pTable) && (name == pTable->getTableName())) {
            pthread_mutex_unlock(&m_mutexTableList);
            return pTable;
        }
    }
    pthread_mutex_unlock(&m_mutexTableList);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// isNameUsed
//

bool
CUserTableObjList::isNameUsed(std::string &name)
{
    if (NULL == getTable(name)) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

bool
CUserTableObjList::init(void)
{
    // Init. table files
    pthread_mutex_lock(&m_mutexTableList);
    std::deque<CVSCPTable *>::iterator it;
    for (it = m_listTables.begin(); it != m_listTables.end(); ++it) {
        CVSCPTable *pTable = *it;
        pthread_mutex_lock(&pTable->m_mutexThisTable);
        pTable->init();
        pthread_mutex_unlock(&pTable->m_mutexThisTable);
    }
    pthread_mutex_unlock(&m_mutexTableList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeTable
//

bool
CUserTableObjList::removeTable(std::string &name, bool bRemoveFile)
{
    pthread_mutex_lock(&m_mutexTableList);

    std::deque<CVSCPTable *>::iterator it;
    for (it = m_listTables.begin(); it != m_listTables.end(); ++it) {
        CVSCPTable *pTable = *it;
        if ((NULL != pTable) && (name == pTable->getTableName())) {
            pTable->cleanUpAndClose(); // Close the table
            delete pTable;
            // m_listTables.DeleteNode( it.m_node );
            m_listTables.erase(it);
            if (bRemoveFile) {
                std::string filename = gpobj->m_rootFolder + "tables/" +
                                       name + ".sqlite3";
                                       remove(filename.c_str());
            }
            pthread_mutex_unlock(&m_mutexTableList);
            return true;
        }
    }

    pthread_mutex_unlock(&m_mutexTableList);
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// clearTable
//

void
CUserTableObjList::clearTable(void)
{
    pthread_mutex_lock(&m_mutexTableList);

    std::deque<CVSCPTable *>::iterator it;
    for (it = m_listTables.begin(); it != m_listTables.end(); /* inline */) {
        CVSCPTable *pTable = *it;
        delete pTable;
        it = m_listTables.erase(it);
    }

    pthread_mutex_unlock(&m_mutexTableList);
}

///////////////////////////////////////////////////////////////////////////////
// isTableInDB
//

bool
CUserTableObjList::isTableInDB(std::string &name)
{
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;

    // Database file must be open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Load tables: Loading. VSCP database file is not open.");
        return false;
    }

    std::string sql = "SELECT COUNT(*) FROM 'table' WHERE name='%s';";
    sql             = (sql, (const char *)name.c_str());

    if (SQLITE_OK !=
        sqlite3_prepare(
          gpobj->m_db_vscp_daemon, sql.c_str(), -1, &ppStmt, NULL)) {

        syslog(LOG_ERR,
               "Table: Unable to check if name is defined. Error=%s",
               sqlite3_errstr(sqlite3_errcode(gpobj->m_db_vscp_daemon)));
        return false;
    }

    int cnt = 0;
    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        cnt = sqlite3_column_int(ppStmt, 0);
    }

    sqlite3_finalize(ppStmt);

    return (cnt ? true : false);
}

///////////////////////////////////////////////////////////////////////////////
// addTableToDB
//

bool
CUserTableObjList::addTableToDB(CVSCPTable &table)
{
    char *zErrMsg = 0;

    // Database must be open
    // Database file must be open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR, "addTableToDB: VSCP database file is not open.");
        return false;
    }

    // No need to add if there already
    if (isTableInDB(table)) return false;

    char *psql = sqlite3_mprintf(VSCPDB_TABLE_INSERT,
                                 table.isEnabled(),
                                 table.isInMemory(),
                                 (const char *)table.getTableName().c_str(),
                                 table.getUserID(),
                                 (int)table.getRights(),
                                 (int)table.getType(),
                                 table.getSize(),

                                 (const char *)table.getLabelX().c_str(),
                                 (const char *)table.getLabelY().c_str(),
                                 (const char *)table.getTitle().c_str(),
                                 (const char *)table.getNote().c_str(),
                                 (const char *)table.getSQLCreate().c_str(),
                                 (const char *)table.getSQLInsert().c_str(),
                                 (const char *)table.getSQLDelete().c_str(),
                                 (const char *)table.getDescription().c_str(),

                                 (int)table.getVSCPClass(),
                                 (int)table.getVSCPType(),
                                 (int)table.getVSCPSensorIndex(),
                                 (int)table.getVSCPUnit(),
                                 (int)table.getVSCPZone(),
                                 (int)table.getVSCPSubZone());

    if (SQLITE_OK !=
        sqlite3_exec(gpobj->m_db_vscp_daemon, psql, NULL, NULL, &zErrMsg)) {
        sqlite3_free(psql);
        syslog( LOG_ERR, "addTableToDB: Unable to insert "
                                           "record in table. [%s] Err=%s",
                                            psql, zErrMsg );
        return false;
    }

    sqlite3_free(psql);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateTableToDB
//

bool
CUserTableObjList::updateTableToDB(CVSCPTable &table)
{
    std::string sql;
    char *zErrMsg = 0;

    // Database must be open
    // Database file must be open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR, "updateTableToDB: VSCP database file is not open.");
        return false;
    }

    // No need to add if there already
    if (!isTableInDB(table)) return addTableToDB(table);

    sql = (VSCPDB_TABLE_UPDATE,
           table.isEnabled(),
           table.isInMemory(),
           (const char *)table.getTableName().c_str(),
           table.getUserID(),
           (int)table.getRights(),
           (int)table.getType(),
           table.getSize(),

           table.getLabelX(),
           (const char *)table.getLabelY().c_str(),
           (const char *)table.getTitle().c_str(),
           (const char *)table.getNote().c_str(),
           (const char *)table.getSQLCreate().c_str(),
           (const char *)table.getSQLInsert().c_str(),
           (const char *)table.getSQLDelete().c_str(),
           (const char *)table.getDescription().c_str(),

           (int)table.getVSCPClass(),
           (int)table.getVSCPType(),
           (int)table.getVSCPSensorIndex(),
           (int)table.getVSCPUnit(),
           (int)table.getVSCPZone(),
           (int)table.getVSCPSubZone());

    if (SQLITE_OK !=
        sqlite3_exec(
          gpobj->m_db_vscp_daemon, sql.c_str(), NULL, NULL, &zErrMsg)) {
        syslog( LOG_ERR, "updateTableToDB: Unable to update "
                                           "record in table. [%s] Err=%s",
                                            sql.c_str(), zErrMsg );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadTablesFromDB
//

bool
CUserTableObjList::loadTablesFromDB(void)
{
    std::string str;
    sqlite3_stmt *ppStmt;

    // Database file must be open
    if (NULL == gpobj->m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Load tables: Loading. VSCP database file is not open.");
        return false;
    }

    syslog(LOG_ERR, "Load tables: Loading tables from database :");

    pthread_mutex_lock(&m_mutexTableList);

    if (SQLITE_OK != sqlite3_prepare(gpobj->m_db_vscp_daemon,
                                     "SELECT * FROM 'table' WHERE bEnable;",
                                     -1,
                                     &ppStmt,
                                     NULL)) {
        syslog(LOG_ERR,
               "Load tables: Error=%s",
               sqlite3_errstr(sqlite3_errcode(gpobj->m_db_vscp_daemon)));
        pthread_mutex_unlock(&m_mutexTableList);
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const char *p;
        bool bMemory;
        bool bEnable;
        std::string name;
        long link_to_user;
        CUserItem *pUserItem;
        uint16_t rights;
        vscpTableType type;
        uint32_t size;

        // link_to_user
        link_to_user =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_LINK_TO_USER);
        pUserItem = gpobj->m_userList.getUser(link_to_user);
        if (NULL == pUserItem) {
            syslog(LOG_ERR,
                   "Load tables: Links to user "
                   "that can't be found, skipped.");
            continue;
        }

        // bEnable
        bEnable = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_ENABLE)
                    ? true
                    : false;

        // bMemory
        bMemory =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_BMEM) ? true : false;

        // name
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_NAME))) {
            name = std::string(p);
        }

        // Rights
        rights = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_PERMISSION);

        // type
        type =
          (vscpTableType)sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_TYPE);

        // size
        size = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_SIZE);

        // Set row default values for row
        CVSCPTable *pTable = new CVSCPTable(
          gpobj->m_rootFolder + ("table/"), name, bEnable, bMemory, type, size);
        if (NULL == pTable) {
            syslog(LOG_ERR,
                   "Load tables: Could not create table object, skipped.");
            continue;
        }

        std::string xname, yname, title, note, sql_create, sql_insert,
          sql_delete, description;

        // xname
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_XNAME))) {
            xname = std::string(p);
        }

        // yname
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_YNAME))) {
            yname = std::string(p);
        }

        // title
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_TITLE))) {
            title = std::string(p);
        }

        // note
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_NOTE))) {
            note = std::string(p);
        }

        // SQL create
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_SQL_CREATE))) {
            sql_create = std::string(p);
        }

        // SQL insert
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_SQL_INSERT))) {
            sql_insert = std::string(p);
        }

        // SQL delete
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_SQL_DELETE))) {
            sql_delete = std::string(p);
        }

        // description
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_TABLE_DESCRIPTION))) {
            description = std::string(p);
        }

        if (!pTable->setTableInfo(pUserItem->getUserName(),
                                  rights,
                                  title,
                                  xname,
                                  yname,
                                  note,
                                  sql_create,
                                  sql_insert,
                                  sql_delete,
                                  description)) {
            syslog(LOG_ERR, "Load tables: Could not set info, skipped.");
            delete pTable;
            continue;
        }

        int vscp_class, vscp_type, vscp_sensorindex, vscp_unit, vscp_zone,
          vscp_subzone;

        // vscp_class
        vscp_class =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_CLASS);

        // vscp_type
        vscp_type = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_TYPE);

        // sensor index
        vscp_sensorindex =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_SENSOR_INDEX);

        // unit
        vscp_unit = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_UNIT);

        // zone
        vscp_zone = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_ZONE);

        // sub zone
        vscp_subzone =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_SUBZONE);

        pTable->setTableEventInfo(vscp_class,
                                  vscp_type,
                                  vscp_sensorindex,
                                  vscp_unit,
                                  vscp_zone,
                                  vscp_subzone);

        if (!addTable(pTable)) {
            syslog(LOG_ERR,
                   "Load tables: Failed to add table object, skipped.");
            delete pTable;
            continue;
        }

    } // While

    sqlite3_finalize(ppStmt);
    pthread_mutex_unlock(&m_mutexTableList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getTableNames
//

bool
CUserTableObjList::getTableNames(std::deque<std::string> &arrayNames)
{
    // Empty
    arrayNames.clear();

    // If no tables defined return empty list
    if (0 == m_listTables.size()) {
        return true;
    }

    std::deque<CVSCPTable *>::iterator it;
    for (it = m_listTables.begin(); it != m_listTables.end(); ++it) {
        CVSCPTable *pTable = *it;
        arrayNames.push_back(pTable->getTableName());
    }

    return true;
}