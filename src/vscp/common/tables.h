// tables.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014
// Ake Hedman, eurosource, <akhe@eurosource.se>
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

/*
http://cplus.about.com/od/learningc/ss/files.htm

Mode Type of file Read Write Create Truncate
--------------------------------------------
r		text Read
rb+		binary Read
r+		text Read Write
r+b		binary Read Write
rb+		binary Read Write
w		text Write Create Truncate
wb		binary Write Create Truncate
w+		text Read Write Create Truncate
w+b		binary Read Write Create Truncate
wb+		binary Read Write Create Truncate
a		text Write Create
ab		binary Write Create
a+		text Read Write Create
a+b		binary Write Create
ab+		binary Write Create
*/



// Table types
enum {
	VSCP_TABLE_NORMAL = 0,
	VSCP_TABLE_STATIC
};


// Thuis structure is located at the start of the main file.
struct _mainFileHead {
	uint8_t id[2];			// File id (mail: 0x55,0xaa  index 0xaa,0x55
	uint8_t type;			// VSCP_TABLE_NORMAL/VSCP_TABLE_STATIC/VSCP_TABLE_INDEX
	char m_nameTable[64];	// NAme of table
	char m_nameXLabel[64];	// Label for X-axis
	char m_nameYLabel[64];	// Label for Y-axis
	uint16_t vscp_class;	// Should be 10 but in future can be 11/12/13...		
	uint16_t vscp_type;		// Measurement type: temp, current etc
	uint8_t vscp_unit;		// Measurement unit: Celsius/Fahrenheit/Kelvin
	fpos_t pos_static;		// Next write position for static file
};

// All data is written with this record type
struct _mainFileRecord {
	time_t timestamp;
	double measurement;
};


// Class that holds one VSCP table

class CVSCPTable {
public:

	/// Constructor dynamic table
	CVSCPTable( const char *fileMain );

	/// Constructor for static table
	CVSCPTable( const char *fileMain, uint32_t size );

	// Destructor
	virtual ~CVSCPTable(void);

	/*!
		Open/create file and get ready to work with it.
		@return 0 on success, <0 on error.
	*/
	int init();

	/*!
		Set table info
		@param tableName Name of table
		@param xAxisLabel Name of X-axis
		@param yAxisLabel Name of Y axis
		@return zero on succss
	*/
	int setTableInfo( const char * tableName, 
						const char *  xAxisLabel, 
						const char * yAxisLabel,
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
		Get size of file
		@param fd File descriptor
		@return Size of file.
	*/
	long fdGetFileSize(const char *path);

	/*!
		Read file header
		@return 0 on success.
	*/
	int readMainHeader( void );

	/*!
		Write file header
		@return 0 on success.
	*/
	int writeMainHeader( void );


	/*!
		Log data
		@param time Log time (since epoch)
		@param measurement Measurement data
		@return zero on success
	*/
	int logData( time_t timestamp, double measurement );

	/*!
		Get a data range
		@param from date/time from which data should be fetched
		@param to date/time to which data should be fetched
		@param buf Buffer where result will be placed. If buffer is NULL
				no data will be filled only the number of records will be
				returned.
		@param size Size of bugger in bytes
		@return Number of records read.
	*/
	long GetRangeOfData( time_t from, time_t to, void *buf, uint16_t size );

private:
	
	/// File handel for mainfile
	FILE * m_ftMain;

	/// Table type
	int m_type;

	/// Size for round robin table
	uint32_t m_sizeStaticTable;

	/// Path to main file
	wxString m_pathMain;

	/// Path to index file
	wxString m_pathIndex;

	/// Main file head structure
	struct _mainFileHead m_mainFileHead;

	/// Timestamp for first record
	time_t m_timestamp_first;

	/// Timestamp for last record
	time_t m_timestamp_last;

	/// Current number of records in database
	long m_number_of_records;
};
