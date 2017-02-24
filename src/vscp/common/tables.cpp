// tables.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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


#ifdef __GNUG__
    //#pragma implementation
#endif

#ifdef WIN32
#include <winsock2.h>
#include <wx/msw/winundef.h>    // https://wiki.wxwidgets.org/WxMSW_Issues
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <sys/stat.h>

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <wx/listimpl.cpp>

#ifndef WIN32
#include <errno.h>
#endif

#include <sqlite3.h>

#include <vscp.h>
#include <vscpdb.h>
#include <vscphelper.h>
#include <variablecodes.h>
#include <controlobject.h>
#include <tables.h>

// From sqlite3 math extensions  sqlite3math.c
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
int RegisterExtensionFunctions( sqlite3 *db );
#if defined(__cplusplus)
}
#endif /* __cplusplus */

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;


// This list holds the tables for vscpd
WX_DEFINE_LIST( listVSCPTables );


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

// Constructor 
CVSCPTable::CVSCPTable( const wxString &folder,
                            const wxString &name,
                            const bool bEnable,
                            const bool bInMemory,
                            const vscpTableType type,
                            uint32_t size )
{
    
    m_pUserItemOwner = NULL;    // No owner yet
    m_dbTable = NULL;           // No database handle yet
    
    m_bValueFirst = true;       // %f is placed before %s in insert
    
    // Initialize event data
    m_vscp_class = 0;
    m_vscp_type = 0;
    m_vscp_sensorindex = 0;
    m_vscp_unit = 0;
    m_vscp_zone = 0;
    m_vscp_subzone = 0;
    
    m_folder = folder;          // save table root folder
    m_tableName = name;         // save table name
    m_bEnable = bEnable;        // save enable state
    m_bInMemory = bInMemory;    // save table in memory flag
    m_type = type;              // save table type
    m_size = size;              // save table size
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPTable::~CVSCPTable( void )
{
    // Clos ethe database if it is open
    if ( NULL != m_dbTable ) {
        sqlite3_close( m_dbTable );
    }
        
}


///////////////////////////////////////////////////////////////////////////////
// setTableInfo
//

bool CVSCPTable::setTableInfo( const wxString &owner,
                                const uint16_t rights,
                                const wxString &title,
                                const wxString &xname, 
                                const wxString &yname,
                                const wxString &note,
                                const wxString &sqlcreate,
                                const wxString &sqlinsert,
                                const wxString &sqldelete,
                                const wxString &description )
{
    // Owner must have a value
    if ( 0 == owner.Length() ) return false;
    m_strOwner = owner;
    
    // the sqlinsert expression must have %f and %s in it for 
    // value and date. It s invalid if not
    int posValue;
    int posDate;
    if ( ( wxNOT_FOUND == ( posValue = sqlinsert.Find( _("%f") ) ) ) ||
            ( wxNOT_FOUND == ( posDate = sqlinsert.Find( _("%s") ) ) ) ) {
        return false;
    }
    
    if ( posValue < posDate ) {
        m_bValueFirst = true;
    }
    else {
        m_bValueFirst = false;
    }
    
    m_rights = rights;
    m_labelTitle = title;
    m_labelX = xname;
    m_labelY = yname;
    m_labelNote = note;
    
    if ( 0 == sqlcreate.Length() ) {
        m_sqlCreate = _(VSCPDB_TABLE_DEFAULT_CREATE);
    }
    else {
        m_sqlCreate = sqlcreate;
    }
    
    if ( 0 == sqlinsert.Length() ) {
        m_sqlInsert = _(VSCPDB_TABLE_DEFAULT_INSERT);
    }
    else {
        m_sqlInsert = sqlinsert;
    }
    
    
    if ( 0 == sqldelete.Length() ) {
        m_sqlInsert = _(VSCPDB_TABLE_DEFAULT_DELETE);
    }
    else {
        m_sqlInsert = sqldelete;
    }        
    
    m_tableDescription = description;
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setTableEventInfo
//

void CVSCPTable::setTableEventInfo( uint16_t vscp_class,
                                        uint16_t vscp_type,
                                        uint8_t vscp_sensorindex,
                                        uint8_t vscp_unit,
                                        uint8_t vscp_zone,
                                        uint8_t vscp_subzone )
{
    m_vscp_class = vscp_class;
    m_vscp_type = vscp_type;
    m_vscp_sensorindex = vscp_sensorindex;
    m_vscp_unit = vscp_unit;
    m_vscp_zone = vscp_zone;
    m_vscp_subzone = vscp_subzone;
}



///////////////////////////////////////////////////////////////////////////////
// init
//

bool CVSCPTable::init() 
{
    // Get useritem
    m_pUserItemOwner = gpobj->m_userList.getUser( m_strOwner );
    if ( NULL == m_pUserItemOwner ) return false;
        
    m_dbFile.Assign( gpobj->m_rootFolder + _("tables/"), m_tableName, _("sqlite3") );
     
    // Check filename 
    if ( !m_bInMemory && m_dbFile.IsOk() && m_dbFile.FileExists() ) {

        // File name OK - Open database
        
        if ( SQLITE_OK != sqlite3_open( (const char *)m_dbFile.GetFullPath().mbc_str(), &m_dbTable ) ) {

            // Failed to open/create the database file
            wxString str;
            gpobj->logMsg("VSCP table database could not be opened.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_dbFile.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_dbTable ) );
            gpobj->logMsg( str.mbc_str() );
            if ( NULL != m_dbTable ) sqlite3_close( m_dbTable );
            m_dbTable = NULL;
            return false;
            
        }

    }
    else {
        
        // File does not exist (and should be created) or something else is wrong.
        // Can also be in-memory db which always should be created,

        if ( m_bInMemory || m_dbFile.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            wxString str;
            gpobj->logMsg( "VSCP table database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_dbFile.GetFullPath().mbc_str() );
            gpobj->logMsg( (const char *)str.mbc_str() );
            
            if ( SQLITE_OK == sqlite3_open( m_bInMemory ? ":memory:" : (const char *)m_dbFile.GetFullPath().mbc_str(),
                                            &m_dbTable ) ) {            
                // create the database.
                if ( !createDatabase() ) {
                    gpobj->logMsg( "Failed to create VSCP table database structure.\n" );
                    sqlite3_close( m_dbTable );
                    m_dbTable = NULL;
                    return false;
                }
            }
            else {
                gpobj->logMsg( "Failed to create VSCP table database.\n" );
                return false;
            }
            
        }
        else {
            wxString str;
            gpobj->logMsg( "VSCP table path/filename is invalid.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_dbFile.GetFullPath().mbc_str() );
            gpobj->logMsg( str.mbc_str() );
            return false;
        }

    }
    
    // Add math & statistical functions
    //sqlite3_enable_load_extension( m_dbTable, 1 );
    RegisterExtensionFunctions( m_dbTable );
    
    // Now find and save the columns of the table
    int rc;

    sqlite3_stmt *ppStmt;
    rc = sqlite3_prepare_v2( m_dbTable, 
                                VSCPDB_TABLE_GET_COLUMNS, 
                                -1, 
                                &ppStmt, 
                                NULL );

    if ( SQLITE_OK == rc ) {
        
        while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
            const unsigned char *p = sqlite3_column_text( ppStmt, 1 );
            wxString tblColumn = wxString::FromUTF8( (const char *)p );
            m_listColumns.Add( tblColumn );
            gpobj->logMsg( _("User table column:") + tblColumn + _("\n"), DAEMON_LOGMSG_DEBUG );
            
        }
        
    }
    
    return true; 
}


///////////////////////////////////////////////////////////////////////////////
// isThisEventForMe
//

bool CVSCPTable::isThisEventForMe( vscpEvent *pEvent )
{
    int offset = 0;
    
    // Check pointers
    if (NULL == pEvent) return false;
    
    // If class >= 512 and class < 1024 we
    // have GUID in front of data. 
    if ( ( pEvent->vscp_class >= VSCP_CLASS2_LEVEL1_PROTOCOL )  && 
            ( pEvent->vscp_class < VSCP_CLASS2_PROTOCOL ) ) {
        offset = 16;
    }
    
    if ( ( VSCP_CLASS1_MEASUREMENT == pEvent->vscp_class ) || 
         ( VSCP_CLASS2_LEVEL1_MEASUREMENT == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
            return true;
        }
        
    }            
    else if ( ( VSCP_CLASS1_MEASUREZONE == pEvent->vscp_class ) || 
              ( VSCP_CLASS2_LEVEL1_MEASUREZONE == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
            
            uint8_t zone = vscp_getVSCPMeasurementZone( pEvent );
            uint8_t subzone = vscp_getVSCPMeasurementSubZone( pEvent );
            if ( ( ( m_vscp_zone == zone ) || ( 255 == zone ) ) && 
                 ( ( m_vscp_subzone == subzone ) || ( 255 == subzone ) ) ) {
                return true;
            }
            
        }
        
    }            
    else if ( ( VSCP_CLASS1_SETVALUEZONE == pEvent->vscp_class ) ||
              ( VSCP_CLASS2_LEVEL1_SETVALUEZONE == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
            
            uint8_t zone = vscp_getVSCPMeasurementZone( pEvent );
            uint8_t subzone = vscp_getVSCPMeasurementSubZone( pEvent );
            if ( ( ( m_vscp_zone == zone ) || ( 255 == zone ) ) && 
                 ( ( m_vscp_subzone == subzone ) || ( 255 == subzone ) ) ) {
                return true;
            }
            
        }
        
        
    }
    else if ( ( VSCP_CLASS1_MEASUREMENT32 == pEvent->vscp_class ) || 
              ( VSCP_CLASS2_LEVEL1_MEASUREMENT32 == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
            return true;
        }
        
    }
    else if ( ( VSCP_CLASS1_MEASUREMENT64 == pEvent->vscp_class ) || 
              ( VSCP_CLASS2_LEVEL1_MEASUREMENT64 == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
            return true;
        }
        
    }
    else if ( ( VSCP_CLASS2_MEASUREMENT_STR == pEvent->vscp_class ) || 
            ( VSCP_CLASS2_MEASUREMENT_FLOAT == pEvent->vscp_class ) ) {
        
        if ( ( m_vscp_sensorindex == vscp_getVSCPMeasurementSensorIndex( pEvent ) ) && 
             ( m_vscp_unit == vscp_getVSCPMeasurementUnit( pEvent ) ) ) {
             
            uint8_t zone = vscp_getVSCPMeasurementZone( pEvent );
            uint8_t subzone = vscp_getVSCPMeasurementSubZone( pEvent );
            if ( ( ( m_vscp_zone == zone ) || ( 255 == zone ) ) && 
                 ( ( m_vscp_subzone == subzone ) || ( 255 == subzone ) ) ) {
                return true;
            }
            
        }
        
    }
    else {
        return false;
    }
        
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cleanUpAndClose
//

void CVSCPTable::cleanUpAndClose( void )
{
    // Close the database if it is open
    if ( NULL != m_dbTable ) {
        sqlite3_close( m_dbTable );
        m_dbTable = NULL;
    }
        
}

///////////////////////////////////////////////////////////////////////////////
// createDatabase
//

bool CVSCPTable::createDatabase( void )
{
    char *pErrMsg = 0;
    
    gpobj->logMsg( "Creating database for table..\n" );
    
    // Check if database is open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( "Failed to create VSCP log database - closed.\n" );
        return false;
    }
    
    m_mutexThisTable.Lock();
    
    if ( SQLITE_OK  !=  sqlite3_exec( m_dbTable, 
                                        (const char *)m_sqlCreate.mbc_str(), 
                                        NULL, 
                                        NULL, 
                                        &pErrMsg ) ) {
        gpobj->logMsg( wxString::Format( _("Failed to create VSCP log table with error %s.\n"), pErrMsg ) );
        m_mutexThisTable.Unlock();
        return false;
    }
    
    m_mutexThisTable.Unlock();
        
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( double jdn, double value )
{
    wxDateTime tobj( jdn );    
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( const struct tm &tm, double value )
{
    wxDateTime tobj( tm );   
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( time_t time, double value )
{
    wxDateTime tobj( time );
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( wxString &strtime, double value )
{
    wxDateTime tobj;
    tobj.ParseISOCombined( strtime );
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( wxDateTime &time, double value )
{
    wxString strInsert = m_sqlInsert;
    
    // Milliseconds  
    long            ms; // Milliseconds
#ifdef WIN32  
    SYSTEMTIME st;
    GetSystemTime( &st );
    ms = st.wMilliseconds;
#else                
    time_t          s;  // Seconds
    struct timespec spec;
    clock_gettime( CLOCK_REALTIME, &spec );
    s = spec.tv_sec;
    ms = round( spec.tv_nsec / 1.0e6 ); // Convert nanoseconds to milliseconds
#endif
    //int ms = wxDateTime::Now().GetMillisecond();   !!!!! Does not work  !!!!!
    wxString msstr = wxString::Format(_(".%d"), ms );
    
    if ( m_bValueFirst ) {
        strInsert.Printf( m_sqlInsert, value, time.FormatISOCombined().mbc_str() + msstr );
    }
    else {
        strInsert.Printf( m_sqlInsert, time.FormatISOCombined().mbc_str() + msstr, value );
    }
    
    return logData( strInsert );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( wxDateTime &time, double value, const wxString &sqlInsert )
{        
    wxString strInsertMod;
    
    // Milliseconds  
    long            ms; // Milliseconds
#ifdef WIN32  
    SYSTEMTIME st;
    GetSystemTime( &st );
    ms = st.wMilliseconds;
#else                
    time_t          s;  // Seconds
    struct timespec spec;
    clock_gettime( CLOCK_REALTIME, &spec );
    s = spec.tv_sec;
    ms = round( spec.tv_nsec / 1.0e6 ); // Convert nanoseconds to milliseconds
#endif
    //int ms = wxDateTime::Now().GetMillisecond();   !!!!! Does not work  !!!!!
    wxString msstr = wxString::Format(_(".%d"), ms );
    
    if ( m_bValueFirst ) {
        strInsertMod.Printf( sqlInsert, value, time.FormatISOCombined().mbc_str() + msstr );
    }
    else {
        strInsertMod.Printf( sqlInsert, time.FormatISOCombined().mbc_str() + msstr, value );
    }
    
    return logData( strInsertMod );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( const wxString strInsert )
{
    char *zErrMsg = 0;
    
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    // If max type and the table contains max number of records it should be
    // cleared before new data is added
    uint32_t nRecords = 0;
    getNumberOfRecords( &nRecords );
    if ( nRecords && ( VSCP_TABLE_MAX == m_type ) && ( nRecords >= m_size ) ) { 
        if ( SQLITE_OK != sqlite3_exec( m_dbTable, 
                                        VSCPDB_TABLE_DEFAULT_DELETE, NULL, NULL, &zErrMsg ) ) {            
            gpobj->logMsg( wxString::Format( _("Add table item: Unable to delete items in db [VSCP_TABLE_MAX]. [%s] Err=%s\n"), 
                                                strInsert.mbc_str(), zErrMsg ) );
            return false; 
        }
    }
    
    if ( SQLITE_OK != sqlite3_exec( m_dbTable, 
                                        strInsert.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
        gpobj->logMsg( wxString::Format( _("Add table item: Unable to insert table item in db. [%s] Err=%s\n"), 
                                            strInsert.mbc_str(), zErrMsg ) );
        return false;
    }
    
    // If the table is a static table we may need to delete the oldest records here
    // to keep the table size     
    if ( ( VSCP_TABLE_STATIC == m_type ) && m_size  ) {  
        wxString strDel = wxString::Format( VSCPDB_TABLE_DELETE_STATIC, (unsigned long)m_size );
        if ( SQLITE_OK != sqlite3_exec( m_dbTable, 
                                            (const char *)strDel.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
            gpobj->logMsg( wxString::Format( _("Add table item: Unable to delete records from static table. [%s] Err=%s\n"), 
                                            (const char *)strDel.mbc_str(), zErrMsg ) );
            return false;
        }
    
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// executeSQL
//

bool CVSCPTable::executeSQL( wxString &sql )
{
    char *zErrMsg = 0;
    
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_exec( m_dbTable, 
                                        sql.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
        gpobj->logMsg( wxString::Format( _("Table: Unable to delete records of table. [%s] Err=%s\n"), 
                                            sql.mbc_str(), zErrMsg ) );
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getNumberOfRecords
//

bool CVSCPTable::getNumberOfRecords( uint32_t *pCount )
{
    wxString wxstr;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    
    // Check pointer
    if ( NULL == pCount ) return false;
        
    // Check if database is open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( "Database is closed.\n" );
        return false;
    }
       
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            VSCPDB_TABLE_COUNT,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Load tables: Error=%s"), sqlite3_errstr( sqlite3_errcode( m_dbTable ) ) );
        gpobj->logMsg( wxstr );
        return false;
    }
    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {              
        *pCount = sqlite3_column_int( ppStmt, 0 );   // Get record count        
    } // While
    
    sqlite3_finalize( ppStmt );
    
    return true;  
}


///////////////////////////////////////////////////////////////////////////////
// getSQLValue
//
    
bool CVSCPTable::getSQLValue( const wxString &sqltemplate, 
                                    const wxDateTime &wxStart, 
                                    const wxDateTime &wxEnd, 
                                    double *pValue  )
{
    wxString wxstr;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    
    // Check pointer
    if ( NULL == pValue ) return false;
        
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    wxString sql = wxString::Format( sqltemplate.mbc_str(),
                                        (const char *)wxStart.FormatISOCombined(' ').mbc_str(),
                                        (const char *)wxEnd.FormatISOCombined(' ').mbc_str() );
    
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            (const char *)sql.mbc_str(),
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Failed to execute SQL expression: Error=%s SQL=%s"), 
                                    sqlite3_errstr( sqlite3_errcode( m_dbTable ) ),
                                    (const char *)sql.mbc_str());
        gpobj->logMsg( wxstr );
        return false;
    }
    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        *pValue = sqlite3_column_double( ppStmt, 0 );
    }
    
    sqlite3_finalize( ppStmt );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getNumberOfRecordsForRange
//

bool CVSCPTable::getNumberOfRecordsForRange( wxDateTime& wxStart, wxDateTime& wxEnd, double *pCount )
{
    return getSQLValue( _("SELECT COUNT(*) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pCount  );
}

///////////////////////////////////////////////////////////////////////////////
// getSumValue
//
    
bool CVSCPTable::getSumValue( wxDateTime wxStart, wxDateTime wxEnd, double *pSum  )
{
    return getSQLValue( _("SELECT SUM(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pSum  );
}


///////////////////////////////////////////////////////////////////////////////
// getMinValue
//
    
bool CVSCPTable::getMinValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMin  )
{
    return getSQLValue( _("SELECT MIN(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pMin  );
}
    
///////////////////////////////////////////////////////////////////////////////
// getMaxValue
//
    
bool CVSCPTable::getMaxValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMax )
{
    return getSQLValue( _("SELECT MAX(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pMax  );
}
    
///////////////////////////////////////////////////////////////////////////////
// getAverageValue
//

bool CVSCPTable::getAverageValue( wxDateTime wxStart, wxDateTime wxEnd, double *pAvarage ) 
{
    return getSQLValue( _("SELECT AVG(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pAvarage  );
}

////////////////////////////////////////////////////////////////////////////
// getMedianValue
//

bool CVSCPTable::getMedianValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMedian ) 
{
    return getSQLValue( _("SELECT MEDIAN(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pMedian  );
}

////////////////////////////////////////////////////////////////////////////
// getStdevValue
//

bool CVSCPTable::getStdevValue( wxDateTime wxStart, wxDateTime wxEnd, double *pStdev ) 
{
    return getSQLValue( _("SELECT STDEV(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pStdev  );
}

////////////////////////////////////////////////////////////////////////////
// getVarianceValue
//

bool CVSCPTable::getVarianceValue( wxDateTime wxStart, wxDateTime wxEnd, double *pVariance ) 
{
    return getSQLValue( _("SELECT VARIANCE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pVariance  );
}

////////////////////////////////////////////////////////////////////////////
// getModeValue
//

bool CVSCPTable::getModeValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMode ) 
{
    return getSQLValue( _("SELECT MODE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pMode  );
}


////////////////////////////////////////////////////////////////////////////
// getLowerQuartileValue
//

bool CVSCPTable::getLowerQuartileValue( wxDateTime wxStart, wxDateTime wxEnd, double *pLowerQuartile ) 
{
    return getSQLValue( _("SELECT LOWER_QUARTILE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pLowerQuartile  );
}

////////////////////////////////////////////////////////////////////////////
// getUppeQuartileValue
//

bool CVSCPTable::getUppeQuartileValue( wxDateTime wxStart, wxDateTime wxEnd, double *pUpperQuartile ) 
{
    return getSQLValue( _("SELECT UPPER_QUARTILE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"), 
                                    wxStart, 
                                    wxEnd, 
                                    pUpperQuartile  );
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool CVSCPTable::prepareRangeOfData( wxDateTime& wxStart, 
                                            wxDateTime& wxEnd, 
                                            sqlite3_stmt **ppStmt, 
                                            bool bAll )
{
    wxString sqlInsert;
    wxString wxstr;
    char *pErrMsg;
            
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    if ( bAll ) {
        sqlInsert = _("SELECT datetime,value,* FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';");
    }
    else  {
        sqlInsert = _("SELECT datetime,value FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';");
    }
    
    wxString sql = wxString::Format( sqlInsert.mbc_str(),
                                        (const char *)wxStart.FormatISOCombined(' ').mbc_str(),
                                        (const char *)wxEnd.FormatISOCombined(' ').mbc_str() );
    
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            (const char *)sql.mbc_str(),
                                            -1,
                                            ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Failed to execute SQL expression: Error=%s SQL=%s"), 
                                    sqlite3_errstr( sqlite3_errcode( m_dbTable ) ),
                                    (const char *)sql.mbc_str());
        gpobj->logMsg( wxstr );
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool CVSCPTable::prepareRangeOfData( wxString& sql, sqlite3_stmt **ppStmt )
{
    wxString wxstr;
    char *pErrMsg;
            
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            (const char *)sql.mbc_str(),
                                            -1,
                                            ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Failed to execute SQL expression: Error=%s SQL=%s"), 
                                    sqlite3_errstr( sqlite3_errcode( m_dbTable ) ),
                                    (const char *)sql.mbc_str());
        gpobj->logMsg( wxstr );
        return false;
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfData
//

bool CVSCPTable::getRowRangeOfData( sqlite3_stmt *ppStmt, wxString& rowData )
{
    if ( SQLITE_ROW != sqlite3_step( ppStmt ) ) {
        return false;
    }
    
    rowData.Empty();
    
    // Fill in the data
    int count = sqlite3_data_count( ppStmt );
    
    for ( int i=0; i<count; i++ ) {
        
        // Skip 'datetime' and 'value' they are in pos 0 and 1
        if ( i>1 ) {
            const char *pName = sqlite3_column_name( ppStmt, i );
            if ( NULL != strstr( pName, "datetime") ) continue;
            if ( NULL != strstr( pName, "value") ) continue;            
        }
        
        const unsigned char *p = sqlite3_column_text( ppStmt, i );
        if ( NULL != p ) {
            rowData += wxString::FromUTF8( (const char *)p );
        }
        
        if ( i<(count-1) ) rowData += _("|");
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfData
//

bool getRowRangeOfData( sqlite3_stmt *ppStmt, vscpTableItem *pTableItem )
{
    // Check pointer
    if ( NULL == pTableItem ) {
        return false;
    }
    
    if ( SQLITE_ROW != sqlite3_step( ppStmt ) ) {
        return false;
    }
    
    // datetime
    const unsigned char *p = sqlite3_column_text( ppStmt, 0 );
    if ( NULL == p ) return false;   
    pTableItem->date.ParseDateTime( p );
    
    // Value
    pTableItem->value = sqlite3_column_double( ppStmt, 1 );
      
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getRowRangeOfDataRaw
//

bool CVSCPTable::getRowRangeOfDataRaw( sqlite3_stmt *ppStmt, wxString& rowData )
{
    if ( SQLITE_ROW != sqlite3_step( ppStmt ) ) {
        return false;
    }
    
    rowData.Empty();
    
    // Fill in the data
    int count = sqlite3_data_count( ppStmt );
    
    for ( int i=0; i<count; i++ ) {
                
        const unsigned char *p = sqlite3_column_text( ppStmt, i );
        if ( NULL != p ) {
            rowData += wxString::FromUTF8( (const char *)p );
        }
        
        if ( i<(count-1) ) rowData += _("|");
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// finalizeRangeOfData
//

bool CVSCPTable::finalizeRangeOfData( sqlite3_stmt *ppStmt )
{
    return ( SQLITE_OK== sqlite3_finalize( ppStmt ) );
}

///////////////////////////////////////////////////////////////////////////////
// getRangeInArray
//

bool CVSCPTable::getRangeInArray( wxDateTime& wxStart, 
                                    wxDateTime& wxEnd,
                                    wxArrayString& strArray,
                                    bool bAll )
{
    sqlite3_stmt *ppStmt;
    
    if ( !prepareRangeOfData( wxStart, wxEnd, &ppStmt, bAll ) ) {
        return false;
    }
    
    wxString rowData;
    while ( getRowRangeOfDataRaw( ppStmt, rowData ) ) {
        strArray.Add( rowData );
    }
    
    return finalizeRangeOfData( ppStmt );
}



/******************************************************************************/
/*                           CUserTableObjList                                */
/******************************************************************************/





// Constructor
CUserTableObjList::CUserTableObjList( void )
{
    
}

// Destructor
CUserTableObjList::~CUserTableObjList( void )
{
    clearTable();
}

///////////////////////////////////////////////////////////////////////////////
// addTable
//

bool CUserTableObjList::addTable( CVSCPTable *pTable )
{
    // Check pointer
    if ( NULL == pTable ) return false;
    
    if ( isNameUsed( pTable->getTableName() ) ) return false;
    
    m_mutexTableList.Lock();
    m_listTables.Append( pTable );
    m_mutexTableList.Unlock();
                        
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// initAll
//

CVSCPTable *CUserTableObjList::getTable( wxString &name )
{
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ){
        
        CVSCPTable *pTable = *iter;
        if ( ( NULL!= pTable ) && ( name == pTable->getTableName() ) ) {
            return pTable;
        }
        
    }
    m_mutexTableList.Unlock();
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// removeTable
//

bool CUserTableObjList::removeTable( wxString &name )
{
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter )
    {
        CVSCPTable *pTable = *iter;
        if ( ( NULL!= pTable ) && ( name == pTable->getTableName() ) ) {
            delete pTable;
            m_listTables.DeleteNode( iter.m_node );
            return true;
        }
    }
    m_mutexTableList.Unlock();
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// isNameUsed
//

bool CUserTableObjList::isNameUsed( wxString &name )
{
    if ( NULL == getTable( name ) ) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

bool CUserTableObjList::init( void ) 
{
    // Init. table files
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ) {
        CVSCPTable *pTable = *iter;
        pTable->m_mutexThisTable.Lock();
        pTable->init();
        pTable->m_mutexThisTable.Unlock();
    }
    m_mutexTableList.Unlock();
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// clearTable
//

void CUserTableObjList::clearTable( void )
{
    m_mutexTableList.Lock();
    
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter )
    {
        CVSCPTable *pTable = *iter;
        delete pTable;
        m_listTables.DeleteNode( iter.m_node );
    }
    
    m_mutexTableList.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// isTableInDB
//

bool CUserTableObjList::isTableInDB( wxString& name )
{
    char *zErrMsg = 0;
    sqlite3_stmt *ppStmt;
    
    // Database file must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("Load tables: Loading. VSCP database file is not open.\n") );
        return false;
    }
    
    wxString sql = _("SELECT COUNT(*) FROM 'table' WHERE name='%s';");
    sql = wxString::Format( sql, (const char *)name.mbc_str() );
    
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon, 
                                        sql.mbc_str(), 
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {    
        wxString wxstr = wxString::Format( _("Table: Unable to check if name is defined. Error=%s"), 
                            sqlite3_errstr( sqlite3_errcode( gpobj->m_db_vscp_daemon ) ) );
        gpobj->logMsg( wxstr );
        return false;
    }
    
    int cnt;    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        cnt = sqlite3_column_int( ppStmt, 0 );
    }
    
    sqlite3_finalize( ppStmt );
    
    return ( cnt ? true : false );
}

///////////////////////////////////////////////////////////////////////////////
// addTableToDB
//

bool CUserTableObjList::addTableToDB( CVSCPTable& table )
{
    wxString sql;
    char *zErrMsg = 0;
    
    // Database must be open
    // Database file must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("Load tables: Loading. VSCP database file is not open.\n") );
        return false;
    }
    
    // No need to add if there allready
    if ( isTableInDB( table ) ) return false;
    
    /*#define VSCPDB_TABLE_INSERT "INSERT INTO 'table' "\
                "(bEnable,bmem,name,link_to_user,permission,type,size,"\
                "xname,yname,title,note,sql_create,sql_insert,sql_delete,description,"\
                "vscpclass,vscptype,vscpsensoridx,vscpunit,vscpzone,vscpsubzone "\
                " ) VALUES ('%d','%d','%s','%ld',%d,'%d','%lu','%s',"\
                "'%s','%s','%s','%s','%s','%s','%s',"\
                "'%d',%d',%d',%d',%d',%d' );"*/

    sql = wxString::Format( VSCPDB_TABLE_INSERT,
            
                                1 /* bEnable*/,
                                table.isInMemory(),
                                (const char *)table.getTableName().mbc_str(),
                                table.getUserID(),
                                (int)table.getRights(),
                                (int)table.getType(),
                                table.getSize(),
            
                                table.getLabelX(),          
                                (const char *)table.getLabelY().mbc_str(),
                                (const char *)table.getTitle().mbc_str(),
                                (const char *)table.getNote().mbc_str(),
                                (const char *)table.getSQLCreate().mbc_str(),
                                (const char *)table.getSQLInsert().mbc_str(),
                                (const char *)table.getSQLDelete().mbc_str(),
                                (const char *)table.getDescription().mbc_str(),
            
                                (int)table.getVSCPClass(),
                                (int)table.getVSCPType(),
                                (int)table.VSCPSensorIndex(),
                                (int)table.VSCPUnit(),
                                (int)table.VSCPZone(),
                                (int)table.VSCPSubZone() );
    
    if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon, 
                                        sql.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
        gpobj->logMsg( wxString::Format( _("Table: Unable to insert record in table. [%s] Err=%s\n"), 
                                            sql.mbc_str(), zErrMsg ) ); 
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateTableToDB
//

bool CUserTableObjList::updateTableToDB( CVSCPTable& table )
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readTablesFromDB
//

bool CUserTableObjList::readTablesFromDB( void )
{
    wxString wxstr;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
        
    // Database file must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("Load tables: Loading. VSCP database file is not open.\n") );
        return false;
    }
    
    gpobj->logMsg( _("Load tables: Loading tables from database :\n") );
    
    m_mutexTableList.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare( gpobj->m_db_vscp_daemon,
                                            "SELECT * FROM 'table' WHERE bEnable;",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Load tables: Error=%s"), sqlite3_errstr( sqlite3_errcode( gpobj->m_db_vscp_daemon ) ) );
        gpobj->logMsg( wxstr );
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        const char *p;
        bool bMemory;
        bool bEnable;
        wxString name;
        uint32_t link_to_user;
        CUserItem *pUserItem;
        uint16_t rights;
        vscpTableType type;
        uint32_t size;
        
        // link_to_user
        link_to_user = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_LINK_TO_USER );
        pUserItem = gpobj->m_userList.getUser( link_to_user );
        if ( NULL == pUserItem ) {
            gpobj->logMsg( _("Load tables: Links to user that can't be found, skipped.\n") );
            continue;
        }
        
        // bEnable
        bEnable = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_ENABLE ) ? true : false;
        
        // bMemory
        bMemory = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_BMEM ) ? true : false;
        
        // name
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_NAME ) ) ) {
            name = wxString::FromUTF8( p );
        }        
        
        // Rights
        rights = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_PERMISSION );
        
        // type
        type = (vscpTableType)sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_TYPE );
        
        // size
        size = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_SIZE );
        
        // Set row default values for row
        CVSCPTable *pTable = new CVSCPTable( gpobj->m_rootFolder + _("table/"),
                                                name, bEnable, bMemory, type, size );
        if ( NULL == pTable ) {
            gpobj->logMsg( _("Load tables: Could not create table object, skipped.\n") );
            continue;
        }
        
        wxString xname, yname, title, note, sql_create, sql_insert, sql_delete, description;
        
        // xname
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_XNAME ) ) ) {
            xname = wxString::FromUTF8( p );
        }
        
        // yname
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_YNAME ) ) ) {
            yname = wxString::FromUTF8( p );
        }
        
        // title
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_TITLE ) ) ) {
            title = wxString::FromUTF8( p );
        }
        
        // note
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_NOTE ) ) ) {
            note = wxString::FromUTF8( p );
        }
        
        // SQL create
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_SQL_CREATE ) ) ) {
            sql_create = wxString::FromUTF8( p );
        }
        
        // SQL insert
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_SQL_INSERT ) ) ) {
            sql_insert = wxString::FromUTF8( p );
        }
        
        // SQL delete
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_SQL_DELETE ) ) ) {
            sql_delete = wxString::FromUTF8( p );
        }
        
        // description
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_TABLE_DESCRIPTION ) ) ) {
            description = wxString::FromUTF8( p );
        }
        
        if ( !pTable->setTableInfo( pUserItem->getUser(),
                                        rights,
                                        title,
                                        xname, 
                                        yname,
                                        note,
                                        sql_create,
                                        sql_insert,
                                        sql_delete,
                                        description ) ) {
            gpobj->logMsg( _("Load tables: Could not set info, skipped.\n") );
            delete pTable;
            continue;
        }
        
        int vscp_class, vscp_type, vscp_sensorindex, vscp_unit, vscp_zone, vscp_subzone;
        
        pTable->setTableEventInfo( vscp_class,
                                        vscp_type,
                                        vscp_sensorindex,
                                        vscp_unit,
                                        vscp_zone,
                                        vscp_subzone );
        
        if ( !addTable( pTable ) ) {
            gpobj->logMsg( _("Load tables: Failed to add table object, skipped.\n") );
            delete pTable;
            continue;
        }
        
        
    } // While
    
    sqlite3_finalize( ppStmt );
    
    m_mutexTableList.Unlock();
    
    return true;    
}