// tables.cpp
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
#include <wx/sstream.h>
#include <wx/listimpl.cpp>
#include <wx/base64.h> 

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
    // Close the database if it is open
    if ( NULL != m_dbTable ) {
        sqlite3_close( m_dbTable );
        m_dbTable = NULL;
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
        m_sqlDelete = _(VSCPDB_TABLE_DEFAULT_DELETE);
    }
    else {
        m_sqlDelete = sqldelete;
    }        
    
    // the sqlinsert expression must have %f and %s in it for 
    // value and date. It s invalid if not
    int posValue;
    int posDate;
    if ( ( wxNOT_FOUND == ( posValue = m_sqlInsert.Find( _("%f") ) ) ) ||
            ( wxNOT_FOUND == ( posDate = m_sqlInsert.Find( _("%s") ) ) ) ) {
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
    
    gpobj->logMsg( "Creating database for table.\n" );
    
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

bool CVSCPTable::logData( double jdn, double value, int ms )
{
    wxDateTime tobj( jdn );    
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( const struct tm &tm, double value, int ms )
{
    wxDateTime tobj( tm );   
    return logData( tobj, value );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( time_t time, double value, int ms )
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
    long ms; // Milliseconds
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

bool CVSCPTable::logData( wxDateTime &dt, double value, const wxString &sqlInsert )
{        
    wxString strInsertMod;
    
    // Milliseconds  
    long            ms; // Milliseconds
#ifdef WIN32  
    SYSTEMTIME st;
    GetSystemTime( &st );
    ms = st.wMilliseconds;
#else                
    time_t s;  // Seconds
    struct timespec spec;
    clock_gettime( CLOCK_REALTIME, &spec );
    s = spec.tv_sec;
    ms = round( spec.tv_nsec / 1.0e6 ); // Convert nanoseconds to milliseconds
#endif
    //int ms = wxDateTime::Now().GetMillisecond();   !!!!! Does not work  !!!!!
    wxString msstr = wxString::Format(_(".%d"), ms );
    
    if ( m_bValueFirst ) {
        strInsertMod.Printf( sqlInsert, value, dt.FormatISOCombined().mbc_str() + msstr );
    }
    else {
        strInsertMod.Printf( sqlInsert, dt.FormatISOCombined().mbc_str() + msstr, value );
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
// clearTable
//

bool CVSCPTable::clearTable( void )
{
    wxString str = wxString::FromAscii( VSCPDB_TABLE_DEFAULT_DELETE );
    return executeSQL( str );
}

///////////////////////////////////////////////////////////////////////////////
// clearTableRange
//

bool CVSCPTable::clearTableRange( wxDateTime& wxStart, wxDateTime& wxEnd )
{
    // Range must be valid
    if ( !wxStart.IsValid() || !wxEnd.IsValid() ) return false;
    
    wxString str = wxString::Format( VSCPDB_TABLE_DELETE_RANGE,
                                        wxStart.FormatISOCombined(),
                                        wxEnd.FormatISOCombined() );
    return executeSQL( str );
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
// getFirstDate
//

bool CVSCPTable::getFirstDate( wxDateTime& dt )
{
    bool rv = true;
    wxString wxstr;
    sqlite3_stmt *ppStmt;
        
    // Check if database is open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( "Database is closed.\n" );
        return false;
    }
       
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            VSCPDB_TABLE_DATE_FIRST,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Load tables: Error=%s"), sqlite3_errstr( sqlite3_errcode( m_dbTable ) ) );
        gpobj->logMsg( wxstr );
        return false;
    }
    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {              
        const unsigned char *p = sqlite3_column_text( ppStmt, 0 );   // Get first date 
        if ( NULL != p ) {
            if ( !dt.ParseISOCombined( wxString::FromAscii( p ) ) ) {
                rv = false;
            }
        }
        else {
            rv = false;
        }
    } // While
    
    sqlite3_finalize( ppStmt );
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getLastDate
//

bool CVSCPTable::getLastDate( wxDateTime& dt )
{
    bool rv = true;
    wxString wxstr;
    sqlite3_stmt *ppStmt;
        
    // Check if database is open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( "Database is closed.\n" );
        return false;
    }
       
    if ( SQLITE_OK != sqlite3_prepare( m_dbTable,
                                            VSCPDB_TABLE_DATE_LAST,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxstr = wxString::Format( _("Load tables: Error=%s"), sqlite3_errstr( sqlite3_errcode( m_dbTable ) ) );
        gpobj->logMsg( wxstr );
        return false;
    }
    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {              
        const unsigned char *p = sqlite3_column_text( ppStmt, 0 );   // Get first date 
        if ( NULL != p ) {
            if ( !dt.ParseISOCombined( wxString::FromAscii( p ) ) ) {
                rv = false;
            }
        }
        else {
            rv = false;
        }
    } // While
    
    sqlite3_finalize( ppStmt );
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getNumberOfRecords
//

bool CVSCPTable::getNumberOfRecords( uint32_t *pCount )
{
    wxString wxstr;
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
    return getSQLValue( _( VSCPDB_TABLE_COUNT_RANGE ), 
                                    wxStart, 
                                    wxEnd, 
                                    pCount  );
}

///////////////////////////////////////////////////////////////////////////////
// getSumValue
//
    
bool CVSCPTable::getSumValue( wxDateTime wxStart, wxDateTime wxEnd, double *pSum  )
{
    return getSQLValue( _( VSCPDB_TABLE_GET_SUM ), 
                                    wxStart, 
                                    wxEnd, 
                                    pSum  );
}


///////////////////////////////////////////////////////////////////////////////
// getMinValue
//
    
bool CVSCPTable::getMinValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMin  )
{
    return getSQLValue( _( VSCPDB_TABLE_GET_MIN ), 
                                    wxStart, 
                                    wxEnd, 
                                    pMin  );
}
    
///////////////////////////////////////////////////////////////////////////////
// getMaxValue
//
    
bool CVSCPTable::getMaxValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMax )
{
    return getSQLValue( _( VSCPDB_TABLE_GET_MAX ), 
                                    wxStart, 
                                    wxEnd, 
                                    pMax  );
}
    
///////////////////////////////////////////////////////////////////////////////
// getAverageValue
//

bool CVSCPTable::getAverageValue( wxDateTime wxStart, wxDateTime wxEnd, double *pAvarage ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_AVG ), 
                                    wxStart, 
                                    wxEnd, 
                                    pAvarage  );
}

////////////////////////////////////////////////////////////////////////////
// getMedianValue
//

bool CVSCPTable::getMedianValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMedian ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_MEDIAN ), 
                                    wxStart, 
                                    wxEnd, 
                                    pMedian  );
}

////////////////////////////////////////////////////////////////////////////
// getStdevValue
//

bool CVSCPTable::getStdevValue( wxDateTime wxStart, wxDateTime wxEnd, double *pStdev ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_STDDEV ), 
                                    wxStart, 
                                    wxEnd, 
                                    pStdev  );
}

////////////////////////////////////////////////////////////////////////////
// getVarianceValue
//

bool CVSCPTable::getVarianceValue( wxDateTime wxStart, wxDateTime wxEnd, double *pVariance ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_VARIANCE ), 
                                    wxStart, 
                                    wxEnd, 
                                    pVariance  );
}

////////////////////////////////////////////////////////////////////////////
// getModeValue
//

bool CVSCPTable::getModeValue( wxDateTime wxStart, wxDateTime wxEnd, double *pMode ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_MODE ), 
                                    wxStart, 
                                    wxEnd, 
                                    pMode  );
}


////////////////////////////////////////////////////////////////////////////
// getLowerQuartileValue
//

bool CVSCPTable::getLowerQuartileValue( wxDateTime wxStart, wxDateTime wxEnd, double *pLowerQuartile ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_LOWER_QUARTILE ), 
                                    wxStart, 
                                    wxEnd, 
                                    pLowerQuartile  );
}

////////////////////////////////////////////////////////////////////////////
// getUppeQuartileValue
//

bool CVSCPTable::getUppeQuartileValue( wxDateTime wxStart, wxDateTime wxEnd, double *pUpperQuartile ) 
{
    return getSQLValue( _( VSCPDB_TABLE_GET_UPPER_QUARTILE ), 
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
            
    // Database file must be open
    if ( NULL == m_dbTable ) {
        gpobj->logMsg( _("VSCP database file is not open.\n") );
        return false;
    }
    
    if ( bAll ) {
        sqlInsert = _( VSCPDB_TABLE_SELECT_STANDARD_RANGE );
    }
    else  {
        sqlInsert = _( VSCPDB_TABLE_SELECT_CUSTOM_RANGE );
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

bool CVSCPTable::prepareRangeOfData( sqlite3_stmt **ppStmt, bool bAll )
{
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    return prepareRangeOfData( wxStart, wxEnd, ppStmt, bAll );
}

///////////////////////////////////////////////////////////////////////////////
// prepareRangeOfData
//

bool CVSCPTable::prepareRangeOfData( wxString& sql, sqlite3_stmt **ppStmt )
{
    wxString wxstr;
            
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
    ;
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
// createTableFromString
//

bool CUserTableObjList::createTableFromString( const wxString &strCreate )
{
    wxString str;
    wxString strCreateXML = "<tables><table ";
    
    wxStringTokenizer tkz( strCreate, _(";"), wxTOKEN_RET_EMPTY_ALL );

    // name = "test_table2"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "name = \""  + str + "\" ";
    
    
    // benable = "true"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "benable = \""  + str + "\" ";
     
    
    // binmemory = "false"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "binmemory = \""  + str + "\" ";
     
    
    // type = "dynamic"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "type = \""  + str + "\" ";
    
 
    // size = "0"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "size = \""  + str + "\" ";
    

    // owner = "admin"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "owner = \""  + str + "\" ";
    
    
    // rights = "0x777"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "rights = \""  + str + "\" ";
    
    
    // title = "This is a title"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "title = \""  + str + "\" ";
    
    
    // xname = "This the x-lable"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "xname = \""  + str + "\" ";

        
    // yname = "This is the y-label"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "yname = \""  + str + "\" ";
      

    // note = "This is a note"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "note = \""  + str + "\" ";
    
    
    // sqlcreate = "SQL create expression"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "sqlcreate = \""  + str + "\" ";
    
    
    // sqlinsert = "SQL insert expression"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "sqlinsert = \""  + str + "\" ";
    

    // sqldelete = "SQL delete expression"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "sqldelete = \""  + str + "\" ";
    
    
    // description = "This is the description"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "description = \""  + str + "\" ";
    
    
    // vscpclass = "10"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "vscpclass = \""  + str + "\" ";
    
    // vscptype = "6"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "vscptype = \""  + str + "\" ";
    
    
    // sensorindex = "0"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "sensorindex = \""  + str + "\" ";
    
    
    // unit = "1"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "unit = \""  + str + "\" ";
    
    // zone = "0"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "zone = \""  + str + "\" ";
    
    // subzone = "0"
    if (!tkz.HasMoreTokens()) {
        return false;
    }
    
    str = tkz.GetNextToken();
    strCreateXML += "subzone = \""  + str + "\" ";
    
    strCreateXML += "/></tables>";
    
    return createTableFromXML( strCreateXML );
}


///////////////////////////////////////////////////////////////////////////////
// createTableFromXML
//

bool CUserTableObjList::createTableFromXML( const wxString &strCreateXML )
{
    bool bFail = false;
    
    wxString wxstr;
    wxString strName;
    bool bEnable = true;
    bool bInMemory = false;  
    vscpTableType type = VSCP_TABLE_DYNAMIC;
    uint32_t size = 0;
    wxString strOwner;
    uint16_t rights;
    wxString strTitle;
    wxString strXname;
    wxString strYname;   
    wxString strNote;
    wxString strSqlCreate;
    wxString strSqlInsert;
    wxString strSqlDelete;
    wxString strDescription;
    uint8_t vscpclass;
    uint8_t vscptype;
    uint8_t sensorindex = 0;
    uint8_t unit = 0;
    uint8_t zone = 255;
    uint8_t subzone = 255;
          
    char buf[32000];
    int dec_len;
    cs_base64_decode( (const unsigned char *)((const char *)strCreateXML.mbc_str() ), 
                        strlen( (const char *)strCreateXML.mbc_str()),
                        buf,
                        &dec_len );
    wxStringInputStream is( wxString::FromUTF8( buf, dec_len ) );
    wxXmlDocument doc;
    if (!doc.Load( is ) ) {
        gpobj->logMsg( _("createTableFromXML: Failed to create XML document.") );
        return false;
    }
    
    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("tables")) {
        gpobj->logMsg( _("createTableFromXML: <tables> item not found in XML file.") );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    if ( !child ) {
        return false;
    }
    
    while ( child ) {
        
        if ( child->GetName() == wxT("table") ) {
            
            // Get the table name
            strName = child->GetAttribute( _("name"), _("") );
            strName.Trim();
            if ( !strName.Length() ) {
                gpobj->logMsg( _("createTableFromXML: Table does not have a name. Skipped.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
            
            // Get enable flag
            wxstr = child->GetAttribute( _("benable"), _("true") );
            if ( wxNOT_FOUND != wxstr.Upper().Find("FALSE") ) {
                bEnable = false;
            }
            
            // Get in memory flag
            wxstr = child->GetAttribute( _("binmemory"), _("false") );
            if ( wxNOT_FOUND != wxstr.Upper().Find("TRUE") ) {
                bInMemory = true;
            }
            
            // Get table type
            wxstr = child->GetAttribute( _("type"), _("dynamic") );
            if ( wxNOT_FOUND != wxstr.Upper().Find( _("STATIC") ) ) {
                type = VSCP_TABLE_STATIC;
            }
            else if ( wxNOT_FOUND != wxstr.Upper().Find( _("DYNAMIC") ) ) {
                type = VSCP_TABLE_DYNAMIC;
            }
            else if ( wxNOT_FOUND != wxstr.Upper().Find( _("MAX") ) ) {
                type = VSCP_TABLE_MAX;
            }
            else {
                gpobj->logMsg( _("createTableFromXML: Table type is not known.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
            
            // Get table type
            wxstr = child->GetAttribute( _("size"), _("0") );
            size = vscp_readStringValue( wxstr );
            
            // Check if size value is valid for static table
            if ( ( VSCP_TABLE_STATIC == type ) && ( 0 == size ) ) {
                gpobj->logMsg( _("createTableFromXML: Invalid table size.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }   
    
            // Check if size value is valid for max table
            if ( ( VSCP_TABLE_MAX == type ) && ( 0 == size ) ) {
                gpobj->logMsg( _("createTableFromXML: Invalid table size.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
            
            // Get owner
            strOwner = child->GetAttribute( _("owner"), _("") );
            
            // Get rights
            wxstr = child->GetAttribute( _("rights"), _("0x744") );
            rights = vscp_readStringValue( wxstr );
            
            // Get title
            strTitle = child->GetAttribute( _("title"), _("") );
            
            // Get X label name
            strXname = child->GetAttribute( _("xname"), _("") );
            
            // Get Y label name
            strYname = child->GetAttribute( _("yname"), _("") );        
            
            // Get note
            strNote = child->GetAttribute( _("note"), _("") );
            
            // Get SQL create statement
            strSqlCreate = child->GetAttribute( _("sqlcreate"), _("") );
            
            // Get SQL insert statement
            strSqlInsert = child->GetAttribute( _("sqlinsert"), _("") );
            
            // Get SQL delete statement
            strSqlDelete = child->GetAttribute( _("sqldelete"), _("") );
            
            // Get table description
            strDescription = child->GetAttribute( _("description"), _("") );
            
            // Get VSCP class
            wxstr  = child->GetAttribute( _("vscpclass"), _("0") );
            vscpclass = vscp_readStringValue( wxstr );
            
            // Get VSCP type
            wxstr  = child->GetAttribute( _("vscptype"), _("0") );
            vscptype = vscp_readStringValue( wxstr );
            
            // Get VSCP sensorindex
            wxstr  = child->GetAttribute( _("sensorindex"), _("0") );
            sensorindex = vscp_readStringValue( wxstr );
            
            // Get VSCP unit
            wxstr  = child->GetAttribute( _("unit"), _("0") );
            unit = vscp_readStringValue( wxstr );
            
            // Get VSCP zone
            wxstr  = child->GetAttribute( _("zone"), _("0") );
            zone = vscp_readStringValue( wxstr );
            
            // Get VSCP subzone
            wxstr  = child->GetAttribute( _("subzone"), _("0") );
            subzone = vscp_readStringValue( wxstr );
            
            // Create the table 
            CVSCPTable *pTable = new CVSCPTable( gpobj->m_rootFolder + _("table/"),    
                                                    strName, 
                                                    bEnable,
                                                    bInMemory, 
                                                    type, 
                                                    size );
    
            if ( NULL == pTable ) {
                gpobj->logMsg( _("createTableFromXML: Failed to allocate new table.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
    
            // Set table info
            if ( !pTable->setTableInfo( strOwner,
                                            rights,
                                            strTitle,
                                            strXname,
                                            strYname,
                                            strNote,
                                            strSqlCreate,
                                            strSqlInsert,
                                            strSqlDelete,
                                            strDescription ) ) {
        
                delete pTable;
                gpobj->logMsg( _("createTableFromXML: Failed to set table-info.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
    
            // Set event info
            pTable->setTableEventInfo( vscpclass,
                                            vscptype,
                                            sensorindex,
                                            unit,
                                            zone,
                                            subzone );
    
            // Initialize the table
            if ( !pTable->init() ) {     
                delete pTable;
                gpobj->logMsg( _("createTableFromXML: Failed to initialize table in system.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;      
            }
            
            // Add the table to the database
            if ( !gpobj->m_userTableObjects.addTableToDB( *pTable ) ) {        
                delete pTable;        
                gpobj->logMsg( _("createTableFromXML: Failed to add table to database.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;
            }
    
            // Add the table to the system table
            if ( !gpobj->m_userTableObjects.addTable( pTable ) ) {       
                delete pTable;
                gpobj->logMsg( _("createTableFromXML: Failed to add table to system.") );
                bFail = true; // Signal failure and handle next record (if any)
                child = child->GetNext();
                continue;      
            }            
            
        }        
        
        child = child->GetNext();        
    }
    
    // If one one the parts failed bFaile will be set
    if ( bFail ) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// initAll
//

CVSCPTable *CUserTableObjList::getTable( wxString &name )
{
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ) {
        
        CVSCPTable *pTable = *iter;
        if ( ( NULL!= pTable ) && ( name == pTable->getTableName() ) ) {
            m_mutexTableList.Unlock();
            return pTable;
        }
        
    }
    m_mutexTableList.Unlock();
    return NULL;
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
// removeTable
//

bool CUserTableObjList::removeTable( wxString &name, bool bRemoveFile )
{
    m_mutexTableList.Lock();
    
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ) {
        CVSCPTable *pTable = *iter;
        if ( ( NULL!= pTable ) && ( name == pTable->getTableName() ) ) {
            pTable->cleanUpAndClose();  // Close the table
            delete pTable;
            //m_listTables.DeleteNode( iter.m_node );
            m_listTables.erase( iter );
            if ( bRemoveFile ) {
                wxRemoveFile(   gpobj->m_rootFolder + _("tables/") + name + _(".sqlite3") );
            }
            m_mutexTableList.Unlock();
            return true;
        }
    }
    
    m_mutexTableList.Unlock();
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// clearTable
//

void CUserTableObjList::clearTable( void )
{
    m_mutexTableList.Lock();
    
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ) {
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
    
    int cnt = 0;    
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
    char *zErrMsg = 0;
    
    // Database must be open
    // Database file must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("addTableToDB: VSCP database file is not open.\n") );
        return false;
    }
    
    // No need to add if there already
    if ( isTableInDB( table ) ) return false;
    
    char *psql = 
            sqlite3_mprintf( VSCPDB_TABLE_INSERT,          
                                table.isEnabled(),
                                table.isInMemory(),
                                (const char *)table.getTableName().mbc_str(),
                                table.getUserID(),
                                (int)table.getRights(),
                                (int)table.getType(),
                                table.getSize(),
            
                                (const char *)table.getLabelX().mbc_str(),          
                                (const char *)table.getLabelY().mbc_str(),
                                (const char *)table.getTitle().mbc_str(),
                                (const char *)table.getNote().mbc_str(),
                                (const char *)table.getSQLCreate().mbc_str(),
                                (const char *)table.getSQLInsert().mbc_str(),
                                (const char *)table.getSQLDelete().mbc_str(),
                                (const char *)table.getDescription().mbc_str(),
            
                                (int)table.getVSCPClass(),
                                (int)table.getVSCPType(),
                                (int)table.getVSCPSensorIndex(),
                                (int)table.getVSCPUnit(),
                                (int)table.getVSCPZone(),
                                (int)table.getVSCPSubZone() );
    
    
    if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon, 
                                        psql, NULL, NULL, &zErrMsg ) ) {            
        sqlite3_free(psql);
        gpobj->logMsg( wxString::Format( _("addTableToDB: Unable to insert "
                                           "record in table. [%s] Err=%s\n"), 
                                            psql, zErrMsg ) ); 
        return false;
    }
    
    sqlite3_free(psql);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateTableToDB
//

bool CUserTableObjList::updateTableToDB( CVSCPTable& table )
{
    wxString sql;
    char *zErrMsg = 0;
    
    // Database must be open
    // Database file must be open
    if ( NULL == gpobj->m_db_vscp_daemon ) {
        gpobj->logMsg( _("updateTableToDB: VSCP database file is not open.\n") );
        return false;
    }
    
    // No need to add if there already
    if ( !isTableInDB( table ) ) return addTableToDB( table );
        
    sql = wxString::Format( VSCPDB_TABLE_UPDATE,
                                table.isEnabled(),
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
                                (int)table.getVSCPSensorIndex(),
                                (int)table.getVSCPUnit(),
                                (int)table.getVSCPZone(),
                                (int)table.getVSCPSubZone() );
    
    if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon, 
                                        sql.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
        gpobj->logMsg( wxString::Format( _("updateTableToDB: Unable to update "
                                           "record in table. [%s] Err=%s\n"), 
                                            sql.mbc_str(), zErrMsg ) ); 
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadTablesFromDB
//

bool CUserTableObjList::loadTablesFromDB( void )
{
    wxString wxstr;
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
        wxstr = wxString::Format( _("Load tables: Error=%s"), 
                 sqlite3_errstr( sqlite3_errcode( gpobj->m_db_vscp_daemon ) ) );
        gpobj->logMsg( wxstr );
        m_mutexTableList.Unlock();
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        const char *p;
        bool bMemory;
        bool bEnable;
        wxString name;
        long link_to_user;
        CUserItem *pUserItem;
        uint16_t rights;
        vscpTableType type;
        uint32_t size;
        
        // link_to_user
        link_to_user = sqlite3_column_int( ppStmt, 
                                            VSCPDB_ORDINAL_TABLE_LINK_TO_USER );
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
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_TABLE_NAME ) ) ) {
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
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                              VSCPDB_ORDINAL_TABLE_XNAME ) ) ) {
            xname = wxString::FromUTF8( p );
        }
        
        // yname
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_TABLE_YNAME ) ) ) {
            yname = wxString::FromUTF8( p );
        }
        
        // title
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_TABLE_TITLE ) ) ) {
            title = wxString::FromUTF8( p );
        }
        
        // note
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_TABLE_NOTE ) ) ) {
            note = wxString::FromUTF8( p );
        }
        
        // SQL create
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_TABLE_SQL_CREATE ) ) ) {
            sql_create = wxString::FromUTF8( p );
        }
        
        // SQL insert
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_TABLE_SQL_INSERT ) ) ) {
            sql_insert = wxString::FromUTF8( p );
        }
        
        // SQL delete
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_TABLE_SQL_DELETE ) ) ) {
            sql_delete = wxString::FromUTF8( p );
        }
        
        // description
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_TABLE_DESCRIPTION ) ) ) {
            description = wxString::FromUTF8( p );
        }
        
        if ( !pTable->setTableInfo( pUserItem->getUserName(),
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
        
        // vscp_class
        vscp_class = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_CLASS ); 
  
        // vscp_type
        vscp_type = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_TYPE );   
        
        // sensor index
        vscp_sensorindex = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_SENSOR_INDEX );
        
        // unit
        vscp_unit = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_UNIT );
        
        // zone
        vscp_zone = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_ZONE );
        
        // sub zone
        vscp_subzone = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_TABLE_VSCP_SUBZONE );
        
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

///////////////////////////////////////////////////////////////////////////////
// getTableNames
//

bool CUserTableObjList::getTableNames( wxArrayString& arrayNames )
{
    // Empty
    arrayNames.Clear();
    
    // If no tables defined return empty list
    if ( 0 == m_listTables.GetCount() ) {
        return true;
    }
    
    listVSCPTables::iterator iter;
    for (iter = m_listTables.begin(); iter != m_listTables.end(); ++iter) {
        CVSCPTable *pTable = *iter;
        arrayNames.Add( pTable->getTableName() );
    }
            
    return true;
}