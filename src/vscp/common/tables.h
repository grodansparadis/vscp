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

#include <clientlist.h>

// Table types
enum vscpTableType {
    VSCP_TABLE_DYNAMIC = 0,
    VSCP_TABLE_STATIC
} ;

// Class that holds one VSCP table

class CVSCPTable {
public:

    /// Constructor dtable
    CVSCPTable( const char *dbpath = NULL, int vscpTableType = VSCP_TABLE_DYNAMIC, uint32_t size = 0 );

    // Destructor
    virtual ~CVSCPTable(void);

    /*!
        Open/create file and get ready to work with it.
        @return true on success, false on error.
    */
    bool init();
    
    // * * * Database file * * *
    
    /*!
     *  Set path to database
     *  @param dbpath Pointer to null terminated string with path to 
     *          database location or where database should b created.
     *  @return true on success, false on failure.
     */
    bool setPathToDB( wxString &dbpath ) { m_pathDB.Assign( dbpath ); };
    
    // Get path to database
    wxString getPathToDB( void ) { return m_pathDB.GetFullPath(); };

    // Return true if database is open
    bool isOpen( void ) { return 0 ? true : false; };

    /*!
        Set table info
        @param tableName Name of table
        @param xAxisLabel Name of X-axis
        @param yAxisLabel Name of Y axis
        @return zero on succss
    */
    int setTableInfo( const char *path,
                        uint8_t type,
                        const char *tableName, 
                        const char *tableDescription,
                        const char *xAxisLabel, 
                        const char *yAxisLabel,
                        uint32_t size,
                        uint16_t vscp_class, 
                        uint16_t vscp_type,
                        uint8_t vscp_unit );         

    /*!
        Check if a file exists and returns true if it does
        @param path Full path to file
        @return nonzero if file exist, zero if not
    */
    int fileExists( const char *path);
    
    /*!
     * Log data
     */
    long logData( unsigned long time, double value );


    /*!
        Get a data range
        @param from date/time from which data should be fetched
        @param to date/time to which data should be fetched
        @param buf Buffer where result will be placed. If buffer is NULL
                no data will be filled only the number of records will be
                returned.
        @param size Size of bugger in bytes
        @return Number of records read or to read or -1 if error.
    */
    long getRangeOfData( uint64_t start, uint64_t end, void *buf = NULL, uint16_t size = 0 );


    /*!
        Get a data range
        @param from date/time from which data should be fetched
        @param to date/time to which data should be fetched
        @param buf Buffer where result will be placed. If buffer is NULL
                no data will be filled only the number of records will be
                returned.
        @param size Size of bugger in bytes
        @return Number of records read or to read or -1 if error.
    */
    long getRangeOfData( wxDateTime& wxStart, wxDateTime& wxEnd, void *buf = NULL, uint16_t size = 0 );


    /*! 
        Get range of data
        @param starpos Record number to start to fecth data at
        @param count nu,ber of records to fetch
        @param buf, Buffer that will be filled with read data
    */
    long getRangeOfData( uint32_t startpos, uint16_t count, struct _vscpFileRecord *buf );


    /*!
        Get static dataset
        @param buf Buffer that holds the result
        @param size of buffer
        @return Number of records in buffer or zero on error
    */
    long getStaticData( void *buf, uint16_t size );

    /*!
        Get required buffer size for static file
        @return Returns the number of bytes needed to hold the
                static file, -1 on error.
    */
    long getStaticRequiredBuffSize( void );

    /*!
        Calculate mean over a range
    */
    double calculatMean( uint64_t from, uint64_t to );

    // Get time for first logged entry
    uint64_t getTimeStampStart( void );

    // Get time for last logged entry
    uint64_t getTimeStampEnd( void );

    // Get number of records in database
    long getNumberOfRecords( void );

    

public:
    
    // Protector for this table
    wxMutex m_mutexThisTable;


private:
    

    /// Path to database file
    wxFileName m_pathDB;	
    
    // Name for table.
    wxString m_tableName;
    
    // Table description
    wxString m_tableDescription;
    
    // Label for table header
    wxString m_labelHeader;
    
    // Label for table note
    wxString m_labelNote;

    // Label for X-axis
    wxString m_labelX;
    
    // Label for y-axis
    wxString m_labelY;
    
    // Size for table with constant size (zero for ever growing)
    uint32_t m_size;
    
    // SQL to create table.
    wxString m_sqlCreate;
    
    // SQL to insert value.
    wxString m_sqlInsert;
    
    // SQL to delete table.
    wxString m_sqlDelete;
    
    // Owner
    CClientItem *m_pClientItem;

};


WX_DECLARE_LIST( CVSCPTable, listVSCPTables );



#endif
