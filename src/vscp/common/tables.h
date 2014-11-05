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


#if !defined(VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPTABLE_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

// Table types
enum  {
	VSCP_TABLE_NORMAL = 0,
	VSCP_TABLE_STATIC
};

// Thuis structure is located at the start of the main file.
struct _vscpFileHead {
	uint8_t id[2];			    // File id (main: 0x55,0xaa  index 0xaa,0x55
	uint8_t type;			    // VSCP_TABLE_NORMAL/VSCP_TABLE_STATIC
	char nameTable[64];		    // Name of table - Used to reference it
	char descriptionTable[512];	// Description of table
	char nameXLabel[128];	    // Label for X-axis
	char nameYLabel[128];	    // Label for Y-axis
	uint16_t vscp_class;	    // Should be 10 but in future can be 11/12/13...		
	uint16_t vscp_type;		    // Measurement type: temp, current etc
	uint8_t vscp_unit;		    // Measurement unit: e.g. Celsius(1)/Fahrenheit(2)/Kelvin(0) for temperature class=10/Type=6

	long unsigned int posStaticRead;	// Next read position for static file
	long unsigned int posStaticWrite;	// Next write position for static file

	long staticSize;		// Number of records in static file
};

// All data is written with this record type
struct _vscpFileRecord {
	uint64_t timestamp;
	double measurement;
};

struct _vscptableInfo {
	double minValue;
	double maxValue;
	uint64_t minTime;
	uint64_t maxTime;
	double meanValue;
	uint32_t nRecords;
};


// Class that holds one VSCP table

class CVSCPTable {
public:

	/// Constructor dtable
	CVSCPTable( const char *file = NULL, int type = VSCP_TABLE_NORMAL, uint32_t size = 0 );

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
		Get size of file
		@param fd File descriptor
		@return Size of file.
	*/
	long fdGetFileSize(const char *path);

	/*!
		Read file header
		@return VSCP_ERROR_ERROR on success.
	*/
	int readMainHeader( void );

	/*!
		Write file header
		@return VSCP_ERROR_ERROR on success.
	*/
	int writeMainHeader( void );


	/*!
		Log data
		@param time Log time (since epoch)
		@param measurement Measurement data
		@return VSCP_ERROR_ERROR on success
	*/
	int logData( uint64_t timestamp, double measurement );

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
	long GetRangeOfData( uint64_t start, uint64_t end, void *buf = NULL, uint16_t size = 0 );


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
    long GetRangeOfData( wxDateTime& wxStart, wxDateTime& wxEnd, void *buf = NULL, uint16_t size = 0 );


	/*!
		Get static dataset
		@param buf Buffer that holds the result
		@param size of buffer
		@return Number of records in buffer or zero on error
	*/
	long GetStaticData( void *buf, uint16_t size );

	/*!
		Get required buffer size for static file
		@return Returns the number of bytes needed to hold the
				static file, -1 on error.
	*/
	long GetStaticRequiredBuffSize( void );

	/*!
		Get statistics for a range for a range
	*/
	int getInfo( struct _vscptableInfo *pInfo, uint64_t from = 0, uint64_t to = 0 );

	/*!
		Calculate mean over a range
	*/
	double calculatMean( uint64_t from, uint64_t to );

    // Get time for firts logged entry
    time_t getTimeStampStart( void ) { return m_timestamp_first; };

    // Get time for last logged entry
    time_t getTimeStampEnd( void ) { return m_timestamp_last; };

public:
    
    // Protector for this table
    wxMutex m_mutexThisTable;

	/// Main file head structure
	struct _vscpFileHead m_vscpFileHead;


private:
	
	/// File handel for mainfile
	FILE * m_ft;

	/// Path to main file
	wxString m_path;	

	/// Timestamp for first record
	uint64_t m_timestamp_first;

	/// Timestamp for last record
	uint64_t m_timestamp_last;

	/// Current number of records in database
	long m_number_of_records;
};


WX_DECLARE_LIST( CVSCPTable, listVSCPTables );



#endif
