// tables.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2014 
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

#include "vscp.h"
#include "vscphelper.h"
#include "variablecodes.h"
#include "tables.h"

//WX_DEFINE_LIST( listVscpTables );

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

// Constructor for nmormal table
CVSCPTable::CVSCPTable( const char * fileMain )
{
	m_pathMain = wxString::FromAscii( fileMain );
    
	m_type = VSCP_TABLE_NORMAL;
	
	memset( &m_mainFileHead, 0, sizeof( m_mainFileHead ) );
	strcpy( m_mainFileHead.m_nameTable, "Table" );
	strcpy( m_mainFileHead.m_nameXLabel, "X Label" );
	strcpy( m_mainFileHead.m_nameYLabel, "Y Label" );

	// Next pos to write at for a new static file
	m_mainFileHead.pos_static = sizeof( m_mainFileHead );

	m_ftMain = NULL;

	init();
}

// Constructor for static table
CVSCPTable::CVSCPTable(const char * fileMain, uint32_t size )
{
	m_pathMain = wxString::FromAscii( fileMain );
    
	m_type = VSCP_TABLE_STATIC;

	memset( &m_mainFileHead, 0, sizeof( m_mainFileHead ) );
	strcpy( m_mainFileHead.m_nameTable, "Table" );
	strcpy( m_mainFileHead.m_nameXLabel, "X Label" );
	strcpy( m_mainFileHead.m_nameYLabel, "Y Label" );

	// Next pos to write at for a new static file
	m_mainFileHead.pos_static = sizeof( m_mainFileHead );

	m_ftMain = NULL;

	// Timestamps for file limits
	m_timestamp_first = 0;
	m_timestamp_last = 0;
	m_number_of_records = 0;

	init();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPTable::~CVSCPTable( void )
{
	fclose( m_ftMain );
}


///////////////////////////////////////////////////////////////////////////////
// init
//

int CVSCPTable::init() 
{
	int rv;

	// Open/create main file
	if ( fileExists( m_pathMain.mbc_str() ) ) {
		m_ftMain = fopen( m_pathMain.mbc_str(), "r+b") ;	// binary Read Write
		if ( NULL == m_ftMain ) return errno;				// Failed to open file
	}
	else {
		// Create file
		m_ftMain = fopen( m_pathMain.mbc_str(), "w+b") ;	// binary Read Write		
		if ( NULL == m_ftMain ) return errno;				// Failed to create file
		
		if ( VSCP_TABLE_NORMAL == m_mainFileHead.type ) {
			m_mainFileHead.id[0] = 0x55;
			m_mainFileHead.id[0] = 0xAA;
		}
		else {
			m_mainFileHead.id[0] = 0xAA;
			m_mainFileHead.id[0] = 0x55;
		}
		
		// Write header
		if ( sizeof( m_mainFileHead ) != 
			fwrite( &m_mainFileHead , 1, sizeof(m_mainFileHead), m_ftMain ) ) return errno;
	}

	rv = writeMainHeader();

	long size = fdGetFileSize( m_pathMain.mbc_str() ) - sizeof( m_mainFileHead );

	return rv; 
}

///////////////////////////////////////////////////////////////////////////////
// setTableInfo
//

int CVSCPTable::setTableInfo( const char *tableName, 
									const char *xAxisLabel, 
									const char *yAxisLabel,
									uint16_t vscp_class, 
									uint16_t vscp_type,
									uint8_t vscp_unit )
{
	int rv;

	strncpy( m_mainFileHead.m_nameTable, tableName, sizeof( m_mainFileHead.m_nameTable ) );
	strncpy( m_mainFileHead.m_nameXLabel, xAxisLabel, sizeof( m_mainFileHead.m_nameXLabel ) );
	strncpy( m_mainFileHead.m_nameYLabel, yAxisLabel, sizeof( m_mainFileHead.m_nameYLabel) );
	m_mainFileHead.vscp_class = vscp_class;
	m_mainFileHead.vscp_type = vscp_type;
	m_mainFileHead.vscp_unit = vscp_unit;

	// Write header
	rv = writeMainHeader();		

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// fileExists
//

int CVSCPTable::fileExists( const char *path )
{
	struct stat buffer;
	return ( stat(path, &buffer) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// fdGetFileSize
//

long CVSCPTable::fdGetFileSize( const char *path )
{
	struct stat st;
	stat( path, &st);
	return st.st_size;
}

///////////////////////////////////////////////////////////////////////////////
// readMainHeader
//

int CVSCPTable::readMainHeader( void )
{
	int rv;

	rv = fseek( m_ftMain, 0, SEEK_SET );									// Go to beginning of file
	if ( rv ) return errno;
	rv = fread( &m_mainFileHead, 1, sizeof(m_mainFileHead), m_ftMain );		// Read structure
	if ( rv ) return errno;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeMainHeader
//

int CVSCPTable::writeMainHeader( void )
{
	int rv;

	rv = fseek( m_ftMain, 0, SEEK_SET );									// Go to beginning of file
	if ( rv ) return errno;
	rv = fwrite( &m_mainFileHead, 1, sizeof(m_mainFileHead), m_ftMain );	// Read structure
	if ( rv ) return errno;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

int CVSCPTable::logData( time_t timestamp, double measurement )
{
	int rv;
	struct _mainFileRecord record;

	record.timestamp = timestamp;
	record.measurement = measurement;

	if ( VSCP_TABLE_NORMAL == m_mainFileHead.type ) {

		// Go to end of main file
		rv = fseek( m_ftMain, 0, SEEK_END );
		if ( rv ) return errno;

		long pos =  ftell( m_ftMain );

		// Write record
		if ( sizeof( record ) != 
			fwrite( &record , 1, sizeof(record), m_ftMain ) ) return errno;

		m_number_of_records++;	// Another record
		if ( 0 == m_timestamp_first ) m_timestamp_first = timestamp; // If first 
		m_timestamp_last = timestamp;

	}
	else if ( VSCP_TABLE_STATIC == m_mainFileHead.type ) {
	
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeMainHeader
//

long CVSCPTable::GetRangeOfData( time_t from, time_t to, void *buf, uint16_t size )
{
	int rv;
	struct _mainFileRecord record;
	long startpos = sizeof(_mainFileHead);
	long searchpos;
	long nRecs = m_number_of_records / 2;
	
	// Set initial searchpos to middle of file.
	searchpos = sizeof(_mainFileHead) + nRecs * sizeof(_mainFileRecord);

	// If there is nothing to do do nothing
	if ( to < from ) return 0;

	if ( m_timestamp_first >= from ) {
		// We can start to read from the beginning
		rv = fseek( m_ftMain, sizeof(_mainFileHead), SEEK_SET );
	}
	else {
		if ( m_number_of_records > 25 ) {

			while ( 1 ) {

				// Seek the pos
				rv = fseek( m_ftMain, searchpos, SEEK_SET );
				
				// read record
				if ( sizeof(_mainFileRecord) != fread( &record, 1, sizeof(_mainFileRecord), m_ftMain ) ) {
					// We start from the beginning as something went wrong
					startpos = sizeof(_mainFileHead);
					break;
				}

				if ( record.timestamp == from  )  {
					// Exact match
					startpos = searchpos;
					break;
				}
				else if ( record.timestamp > from ) {
					// Go halfways back to beginning
					nRecs /= 2;
					searchpos = sizeof(_mainFileHead) + nRecs * sizeof(_mainFileRecord);					
				}
				else {
					// Test upper half
					long incRecs = nRecs /2;
					nRecs += incRecs;
					searchpos = sizeof(_mainFileHead) + nRecs * sizeof(_mainFileRecord);	
				}

			}
		}
		
	}

	return rv;
}