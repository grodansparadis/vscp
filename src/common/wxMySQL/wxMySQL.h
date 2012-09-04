/////////////////////////////////////////////////////////////////////////////
// Name:        wxMySQL.h
// Purpose:     wxMyQSL (v0.1.2)
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by:
// Created:     Feb-19-2005
// RCS-ID:      $Id:
// Copyright:   (c) Volodymir (T-Rex) Tryapichko
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MYSQL_H
#define _WX_MYSQL_H

#include <wx/wx.h>
#include <wx/list.h>
#ifdef __WXMSW__
	#include <windows.h>
#endif
#include <mysql.h>

enum wxMySQLFieldType 
{	
	wxMYSQL_TYPE_DECIMAL, 
	wxMYSQL_TYPE_TINY,
	wxMYSQL_TYPE_SHORT,  
	wxMYSQL_TYPE_LONG,
	wxMYSQL_TYPE_FLOAT,  
	wxMYSQL_TYPE_DOUBLE,
	wxMYSQL_TYPE_NULL,   
	wxMYSQL_TYPE_TIMESTAMP,
	wxMYSQL_TYPE_LONGLONG,
	wxMYSQL_TYPE_INT24,
	wxMYSQL_TYPE_DATE,   
	wxMYSQL_TYPE_TIME,
	wxMYSQL_TYPE_DATETIME, 
	wxMYSQL_TYPE_YEAR,
	wxMYSQL_TYPE_NEWDATE,
	wxMYSQL_TYPE_ENUM=247,
	wxMYSQL_TYPE_SET=248,
	wxMYSQL_TYPE_TINY_BLOB=249,
	wxMYSQL_TYPE_MEDIUM_BLOB=250,
	wxMYSQL_TYPE_LONG_BLOB=251,
	wxMYSQL_TYPE_BLOB=252,
	wxMYSQL_TYPE_VAR_STRING=253,
	wxMYSQL_TYPE_STRING=254,
	wxMYSQL_TYPE_GEOMETRY=255
};

wxString wxMySQLGetFieldTypeName(wxMySQLFieldType type);


// ========================================================================
//  wxMySQLField
// ------------------------------------------------------------------------
/// holds the data about the recordset field: name, data, length
class wxMySQLField
{
public:	
	wxString m_Name; ///< Stores the field's name
	wxString m_NameOrg; ///< Stores the field's original name
	wxString m_TableName; ///< Stores the name of the table that contains the field
	wxString m_TableNameOrg; ///< Stores an original name of the table that contains the field
	wxString m_DBName; ///< Stores the name of database
	wxString m_DBCatalog; ///< Stores the name of database catalog
	wxString m_DefaultValue; ///< Stores default value
	unsigned long m_Length; ///< Stores field's length
	unsigned long m_MaxLength; ///< Stores field's maximal length
	unsigned int m_Decimals; ///< Stores the number of decimal digits for numeric field
	unsigned int m_Flags; ///< Stores the field's style
	wxMySQLFieldType m_FieldType; ///< Stores the field's type
	bool IsNotNULL(); ///< Indicates if the does the field allow NULL-values
	bool IsPrimaryKeyField(); ///< Indicates if the field is a part of the primary key
	bool IsUniqueKeyField(); ///< Indicates is the field allows only unique values
	bool IsNonUniqueKeyField(); ///< Indicates is the field allows non-unique values
	bool IsUnSigned(); ///< Indicates if the field value is unsigned
	bool IsZeroFill(); ///< Indicates if the field is zero-filled
	bool IsBinary(); ///< Indicates if the field contains binary data
	bool IsAutoIncrement(); ///< Indicates if the field is auto-incremental
	bool IsEnum(); ///< Indicates if the field can store only enumerated values
	bool IsSet(); ///< Indicates if the field can store only the values of the set
	bool IsBLOB(); ///< Indicates if the field stores BLOB values
	bool IsTimeStamp();	///< Indicates if the field stores timestamps
};

// ========================================================================
//  wxMySQLFieldArray
// ------------------------------------------------------------------------
/// Dynamic array that contains wxMySQLField objects
WX_DECLARE_OBJARRAY(wxMySQLField, wxMySQLFieldArray);


// ========================================================================
//  wxMySQLResult
// ------------------------------------------------------------------------
/// It is used for maintenance of data selection operations.
class wxMySQLResult
{		
	MYSQL_RES * m_Res;	
public:	
	/// Default constructor
	wxMySQLResult();
	/// Destructor
	~wxMySQLResult();
	/// Assigns the handle of wxMySQLResult
	void SetHandle(MYSQL_RES * res);
	/// Releases the handle
	void Clear();	
	/// Returns the number of the fields
	unsigned int GetFieldCount();
	/// Returns the number of selected rows
	unsigned long GetRowCount();
	/// Fetches the row to the list of strings
	/*!
      \sa Fetch(wxArrayString & array)
      \param list the list to receive all field values. The number of list items is set automatically
    */
	bool Fetch(wxStringList & list);
	/// Fetches the row to the array of strings
	/// Fetches the row to the list of strings
	/*!
      \sa Fetch(wxStringList & list)
      \param array the array to receive all field values. The length of array is set automatically
    */
	bool Fetch(wxArrayString & array);
	/// Retrieves information about the field with specified index
	/*!
	  \param field the pointer to wxMySQLField object to store information about specified field
	  \param fieldnum field index
	*/
	bool GetFieldInfo(wxMySQLField * field, unsigned int fieldnum);
	/// Retrieves information about all fields
	bool GetFieldsInfo(wxMySQLFieldArray & array);
	/// Moves the cursor to the specified row
	void SeekRow(unsigned long index);
};

// ========================================================================
//  wxMySQLRecordset
// ------------------------------------------------------------------------
/// Incapsulates most frequently used functions for work with a data set
class wxMySQLRecordset
{
	MYSQL_STMT * m_Stmt;	
	MYSQL_BIND * m_ColDefs;
	bool m_HasResult;	
	unsigned int m_FieldCount;
	void SetFieldCount(unsigned int value);
public:
	/// Default constructor
	wxMySQLRecordset();
	MYSQL_STMT * GetSTMT() {return m_Stmt;}
	/// Returns the number of fields in recordset
	unsigned int GetFieldCount();
	/// Returns the number of rows in recordset
	unsigned int GetRowCount();
	/// Assigns the handle of the recordset
	void SetHandle(MYSQL_STMT * stmt);	
	/// Returns type of a field with the specified index
	void SetFieldType(unsigned int index, wxMySQLFieldType type);
	/// Assigns the temporary buffer for storing the data of a field with the specified index
	void SetFieldBuffer(unsigned int index, void * buffer);
	/// Sets the length of temporary buffer for storing the data of a field with the specified index
	void SetFieldBufferLength(unsigned int index, unsigned long * length);
	/// Binds the fields to their temporary buffers
	bool BindFields();
	/// Executes the query
	bool Execute();
	/*!
	  \param rowindex is a row number and should be in the range from 0 to GetRowCount()-1. 
	*/
	void Seek(unsigned int rowindex);
	/// Fetches the data from current row
	bool Fetch();
	/// Closes the recordset
	bool Close();
	/// Indicates if the handle is set
	bool IsOk();
	/// Prepares the transaction
	/*!
      \sa Prepare(wxString query)
      \param query the SQL query
    */
	bool Prepare(wxString query);	
	/// Returns the last error message
	wxString GetLastErrorMessage();	
};

// ========================================================================
//  wxMySQL
// ------------------------------------------------------------------------
/// Provides the connection to MySQL server
class wxMySQL
{
	wxString m_Hostname;
	int m_Port;
	wxString m_Username;
	wxString m_Password;
	wxString m_DatabaseName;
	MYSQL * m_pDB;
	wxString m_LastErrorMessage;
public:
	/// Default constructor
	wxMySQL();
	/// Parametrized constructor
	/*!
	  \sa wxMySQL()
	  \param hostname contains the host name of the machine with installed MySQL server.
	  The value of hostname parameter may be either a hostname or an IP address. If hostname is NULL or the string "localhost", a connection to the local host is assumed. If the OS supports sockets (Unix) or named pipes (Windows), they are used instead of TCP/IP to connect to the server.
	  \param port contains the port for MySQL connection. If port is not 0, the value will be used as the port number for the TCP/IP connection. 
	  \param username contains the user's MySQL login ID. If username is NULL or the empty string "", the current user is assumed. Under Unix, this is the current login name. Under Windows ODBC, the current username must be specified explicitly.
	  \param password contains the password for user. If password is NULL, only entries in the user table for the user that have a blank (empty) password field will be checked for a match.
	  \param dbname is the database name. If dbname is not NULL, the connection will set the default database to this value.
	*/
	wxMySQL(wxString hostname, int port, wxString username, wxString password, wxString dbname = _(""));
	/// Destructor
	~wxMySQL();
	/// Opens MySQL connection
	/*!	  
	  \param hostname contains the host name of the machine with installed MySQL server.
	  The value of hostname parameter may be either a hostname or an IP address. If hostname is NULL or the string "localhost", a connection to the local host is assumed. If the OS supports sockets (Unix) or named pipes (Windows), they are used instead of TCP/IP to connect to the server.
	  \param port contains the port for MySQL connection. If port is not 0, the value will be used as the port number for the TCP/IP connection. 
	  \param username contains the user's MySQL login ID. If username is NULL or the empty string "", the current user is assumed. Under Unix, this is the current login name. Under Windows ODBC, the current username must be specified explicitly.
	  \param password contains the password for user. If password is NULL, only entries in the user table for the user that have a blank (empty) password field will be checked for a match.
	  \param dbname is the database name. If dbname is not NULL, the connection will set the default database to this value.
	*/
	bool Connect(wxString hostname, int port, wxString username, wxString password, wxString dbname = _(""));
	bool ChangeUser(wxString username, wxString password, wxString dbname = _(""));
	bool IsConnected();
	bool EnumDatabases(wxStringList & list);
	bool EnumDatabases(wxArrayString & array);
	bool EnumTables(wxStringList & list, wxString wildchar = _(""));
	bool EnumTables(wxArrayString & array, wxString wildchar = _(""));
	bool CreateDatabase(wxString dbname);
	bool OpenDatabase(wxString dbname);
	bool DropDatabase(wxString dbname);
	bool Execute(wxString query, wxMySQLResult * result = NULL);
	bool CreateRecordset(wxString query, wxMySQLRecordset * result = NULL);
	unsigned long GetAffectedRows();
	unsigned int GetTableCount();		
	wxString GetLastErrorMessage();
	void Disconnect();	
};

#if wxMYSQL_BACKWARD_COMPATIBILITY
#define DoQuery Execute
#endif

#endif

