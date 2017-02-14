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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

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
                            const bool bInMemory,
                            const vscpTableType type,
                            uint32_t size )
{
    m_pOwner = NULL;    // No owner yet
    m_dbTable = NULL;   // No database handle yet
    
    m_bValueFirst = true;   // %f is placed before %s in insert
    
    // Initialize event data
    m_vscp_class = 0;
    m_vscp_type = 0;
    m_vscp_sensorindex = 0;
    m_vscp_unit = 0;
    m_vscp_zone = 0;
    m_vscp_subzone = 0;
    
    m_folder = folder;          // save table root folder
    m_tableName = name;         // save table name
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
    m_rights = rights;
    m_labelTitle = title;
    m_labelX = xname;
    m_labelY = yname;
    m_labelNote = note;
    m_sqlCreate = sqlcreate;
    m_sqlInsert = sqlinsert;
    m_sqlDelete = sqldelete;
    m_tableDescription = description;
    
    // Get useritem
    m_pOwner = gpobj->m_userList.getUser( owner );
    if ( NULL == m_pOwner ) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setTableInfo
//

bool CVSCPTable::setTableInfo( CUserItem *pUserItem,
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
    // Check pointer
    if ( NULL == pUserItem ) return NULL;
    
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
    m_sqlCreate = sqlcreate;
    m_sqlInsert = sqlinsert;
    m_sqlDelete = sqldelete;
    m_tableDescription = description;
    
    // Get useritem
    m_pOwner = pUserItem;
    
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
    // Must be initialized
    if ( NULL == m_pOwner ) return false; 
    
    m_dbFile.Assign( gpobj->m_rootFolder + _("tables/"), m_tableName, _("sqlite3") );
     
    // Check filename 
    if ( m_dbFile.IsOk() && m_dbFile.FileExists() ) {

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

        if ( m_dbFile.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            wxString str;
            gpobj->logMsg( "VSCP table database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_dbFile.GetFullPath().mbc_str() );
            gpobj->logMsg( (const char *)str.mbc_str() );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_dbFile.GetFullPath().mbc_str(),
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
// CleanUpAndClose
//

void CVSCPTable::CleanUpAndClose( void )
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
// getRangeOfDataCount
//

long CVSCPTable::getRangeOfDataCount( wxDateTime& wxStart, wxDateTime& wxEnd )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// getRangeOfData
//

long CVSCPTable::getRangeOfData( wxDateTime& wxStart, wxDateTime& wxEnd, void *buf, uint16_t size )
{
    return 0;
}




///////////////////////////////////////////////////////////////////////////////
// getStaticData
//

long CVSCPTable::getStaticData( void *buf, uint32_t size )
{

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// getStaticRequiredBuffSize
//

long CVSCPTable::getStaticRequiredBuffSize( void )
{
    return 0;
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
    wxString strInsert;
    
    // Nothing to do if the database is closed
    if ( NULL != m_dbTable ) return false;
    
    if ( m_bValueFirst ) {
        strInsert.Format( m_sqlInsert, value, time.FormatISOCombined() );
    }
    else {
        strInsert.Format( m_sqlInsert, time.FormatISOCombined(), value );
    }
    
    return doInsert( strInsert );
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

bool CVSCPTable::logData( wxDateTime &time, double value, const wxString &sqlInsert )
{        
    wxString strInsertMod;
    
    if ( m_bValueFirst ) {
        strInsertMod.Format( sqlInsert, value, time.FormatISOCombined() );
    }
    else {
        strInsertMod.Format( sqlInsert, time.FormatISOCombined(), value );
    }
    
    return doInsert( strInsertMod );
}

///////////////////////////////////////////////////////////////////////////////
// doInsert
//

bool CVSCPTable::doInsert( const wxString strInsert )
{
    char *zErrMsg = 0;
    
    if ( SQLITE_OK != sqlite3_exec( gpobj->m_db_vscp_daemon, 
                                        strInsert.mbc_str(), NULL, NULL, &zErrMsg ) ) {            
        gpobj->logMsg( wxString::Format( _("Add table item: Unable to insert table item in db. [%s] Err=%s\n"), 
                                            strInsert.mbc_str(), zErrMsg ) );
        return false;
    }
    
    return true;
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
                                            "SELECT * FROM table WHERE bEnable=1;",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        const char *p;
        bool bMemory;
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
        
        // bmemory
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
                                                name, bMemory, type, size );
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
        
        if ( !pTable->setTableInfo( pUserItem,
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