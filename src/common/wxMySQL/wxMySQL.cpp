/////////////////////////////////////////////////////////////////////////////
// Name:        wxMySQL.cpp
// Purpose:     wxMyQSL (v0.1.2)
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by:
// Created:     Feb-19-2005
// RCS-ID:      $Id:
// Copyright:   (c) Volodymir (T-Rex) Tryapichko
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////

#include "wxMySQL.h"
#include <wx/arrimpl.cpp>
#include <algorithm>

#ifdef __WXMSW__
#include <windef.h>
#endif

WX_DEFINE_OBJARRAY(wxMySQLFieldArray);

wxString wxMySQLGetFieldTypeName(wxMySQLFieldType type)
{
	switch(type)
	{
	case wxMYSQL_TYPE_DECIMAL: return _("DECIMAL");
	case wxMYSQL_TYPE_TINY: return _("TINYINT");
	case wxMYSQL_TYPE_SHORT: return _("SMALLINT");
	case wxMYSQL_TYPE_LONG: return _("INTEGER");
	case wxMYSQL_TYPE_FLOAT: return _("FLOAT");
	case wxMYSQL_TYPE_DOUBLE: return _("DOUBLE");
	case wxMYSQL_TYPE_NULL: return _("NULL");
	case wxMYSQL_TYPE_TIMESTAMP: return _("TIMESTAMP");
	case wxMYSQL_TYPE_LONGLONG: return _("BIGINT");
	case wxMYSQL_TYPE_INT24: return _("MEDIUMINT");
	case wxMYSQL_TYPE_DATE: return _("DATE");
	case wxMYSQL_TYPE_TIME: return _("TIME");
	case wxMYSQL_TYPE_DATETIME: return _("DATETIME");
	case wxMYSQL_TYPE_YEAR: return _("YEAR");
	case wxMYSQL_TYPE_NEWDATE: return _("NEWDATE");
	case wxMYSQL_TYPE_ENUM: return _("ENUM");
	case wxMYSQL_TYPE_SET: return _("SET");
	case wxMYSQL_TYPE_TINY_BLOB: return _("TINYBLOB");
	case wxMYSQL_TYPE_MEDIUM_BLOB: return _("MEDIUMBLOB");
	case wxMYSQL_TYPE_LONG_BLOB: return _("LONGBLOB");
	case wxMYSQL_TYPE_BLOB: return _("BLOB");
	case wxMYSQL_TYPE_VAR_STRING: return _("VARCHAR");
	case wxMYSQL_TYPE_STRING: return _("CHAR");
	case wxMYSQL_TYPE_GEOMETRY: return _("GEOMETRY");
	}
	return _("Unknown type");
}

//----------------------------------------------------------------
bool wxMySQLField::IsNotNULL()
{
	return m_Flags & NOT_NULL_FLAG;
}

bool wxMySQLField::IsPrimaryKeyField()
{
	return m_Flags & PRI_KEY_FLAG;
}

bool wxMySQLField::IsUniqueKeyField()
{
	return m_Flags & UNIQUE_KEY_FLAG;
}

bool wxMySQLField::IsNonUniqueKeyField()
{
	return m_Flags & MULTIPLE_KEY_FLAG;
}

bool wxMySQLField::IsUnSigned()
{
	return m_Flags & UNSIGNED_FLAG;
}

bool wxMySQLField::IsZeroFill()
{
	return m_Flags & ZEROFILL_FLAG;
}

bool wxMySQLField::IsBinary()
{
	return m_Flags & BINARY_FLAG;
}

bool wxMySQLField::IsAutoIncrement()
{
	return m_Flags & AUTO_INCREMENT_FLAG;
}

bool wxMySQLField::IsEnum()
{
	return m_Flags & ENUM_FLAG;
}

bool wxMySQLField::IsSet()
{
	return m_Flags & SET_FLAG;
}

bool wxMySQLField::IsBLOB()
{
	return m_Flags & BLOB_FLAG;
}

bool wxMySQLField::IsTimeStamp()
{
	return m_Flags & TIMESTAMP_FLAG;
}
//----------------------------------------------------------------

wxMySQLResult::wxMySQLResult() 
: m_Res(NULL) 
{
}

wxMySQLResult::~wxMySQLResult()
{ 
	if(m_Res) Clear(); 
}

void wxMySQLResult::Clear() 
{ 
	mysql_free_result(m_Res);
	m_Res = NULL;
}

void wxMySQLResult::SetHandle(MYSQL_RES * res) 
{ 
	m_Res = res; 
}

unsigned int wxMySQLResult::GetFieldCount() 
{ 
	return mysql_num_fields(m_Res);
}

unsigned long wxMySQLResult::GetRowCount()
{
	return mysql_num_rows(m_Res);
}

bool wxMySQLResult::Fetch(wxStringList & list)
{
	list.Clear();
	if(!m_Res) return false;
	MYSQL_ROW row;
	unsigned int i;
	if(row = mysql_fetch_row(m_Res)) 
	{
		for(i = 0; i < GetFieldCount(); i++)
		{			
			list.Add(row[i]);
		}
	}	
	return (row != NULL);
}

bool wxMySQLResult::Fetch(wxArrayString & array)
{
	array.Clear();
	if(!m_Res) return false;
	MYSQL_ROW row;
	unsigned int i;
	if(row = mysql_fetch_row(m_Res)) 
	{
		for(i = 0; i < GetFieldCount(); i++)
		{			
			array.Add(row[i]);
		}
	}	
	return (row != NULL);
}

bool wxMySQLResult::GetFieldInfo(wxMySQLField * field, unsigned int fieldnum)
{
	if(GetFieldCount() <= fieldnum) return false;
	MYSQL_FIELD * fld = mysql_fetch_field_direct(m_Res, fieldnum);
	if(!fld || !field) return false;
	field->m_Name = fld->name;
	field->m_NameOrg = fld->org_name;
	field->m_TableName = fld->table;
	field->m_TableNameOrg = fld->org_table;
	field->m_DBName = fld->db;
	field->m_DBCatalog = fld->catalog;
	field->m_DefaultValue = fld->def;
	field->m_Length = fld->length;
	field->m_MaxLength = fld->max_length;
	field->m_Decimals = fld->decimals;
	field->m_FieldType = (wxMySQLFieldType)fld->type;
	field->m_Flags = fld->flags;
	return true;
}

bool wxMySQLResult::GetFieldsInfo(wxMySQLFieldArray & array)
{
	wxMySQLField field;
	for(unsigned int i = 0; i < GetFieldCount(); i++)
	{
		if(!GetFieldInfo(&field, i)) return false;
		array.Add(field);
	}
	return true;
}

void wxMySQLResult::SeekRow(unsigned long index)
{
	mysql_data_seek(m_Res, index);
}

//--------------------------------------------------------------------------------
wxMySQLRecordset::wxMySQLRecordset()
: m_Stmt(NULL), m_ColDefs(NULL), m_HasResult(false)
{	
}

void wxMySQLRecordset::SetHandle(MYSQL_STMT * stmt)
{
	m_Stmt = stmt;
	m_HasResult = false;	
	if(m_Stmt)
	{	
		printf("MYSQL_STMT_RESULT_METADATA\r\n");
		MYSQL_RES * metadata_result = mysql_stmt_result_metadata(m_Stmt);
		if(metadata_result)
		{				
			m_HasResult = true;
			SetFieldCount(mysql_num_fields(metadata_result));
			mysql_free_result(metadata_result);
		} else SetFieldCount(mysql_stmt_param_count(m_Stmt));
	}
}

bool wxMySQLRecordset::IsOk()
{
	return (m_Stmt != NULL);
}

void wxMySQLRecordset::SetFieldCount(unsigned int value)
{
	if(m_ColDefs) delete [] m_ColDefs;
	m_ColDefs = NULL;
	if(value) m_ColDefs = new MYSQL_BIND[value];
	if(m_ColDefs)
	{
		memset(m_ColDefs, 0, value * sizeof(MYSQL_BIND));
		m_FieldCount = value;
	} else m_FieldCount = 0;
}

void wxMySQLRecordset::SetFieldType(unsigned int index, wxMySQLFieldType type)
{
	if(index >= GetFieldCount())
	{
		printf("SetFieldType error in index\r\n");
		return;	
	}
	m_ColDefs[index].buffer_type = (enum_field_types)type;	
}

void wxMySQLRecordset::SetFieldBuffer(unsigned int index, void * buffer)
{
	if(index >= GetFieldCount())
	{
		printf("SetFieldBuffer error in index\r\n");
		return;
	}
	m_ColDefs[index].buffer = buffer;	
}

void wxMySQLRecordset::SetFieldBufferLength(unsigned int index, unsigned long * length)
{
	if(index >= GetFieldCount())
	{
		printf("SetFieldBufferLength error in index\r\n");
		return;
	}
	m_ColDefs[index].length = length;	
}

bool wxMySQLRecordset::BindFields()
{	
	if(!GetFieldCount())
	{
		printf("Error binding fields !!!\r\n");
		return false;	
	}
	printf("MYSQL_STMT_BIND_PARAM\r\n");
	return !mysql_stmt_bind_param(m_Stmt, m_ColDefs);		
}

bool wxMySQLRecordset::Execute()
{
	printf("MYSQL_STMT_EXECUTE\r\n");
	bool res = !mysql_stmt_execute(m_Stmt);
	if(res)
	{		
		if(m_HasResult)
		{
			mysql_stmt_store_result(m_Stmt);			
		}
	}
	return res;
}

void wxMySQLRecordset::Seek(unsigned int rowindex)
{
	unsigned int topindex(GetRowCount()-1);		
	printf("MYSQL_STMT_DATA_SEEK\r\n");
	mysql_stmt_data_seek(m_Stmt, std::max((int)0, (int)std::min(topindex, rowindex)));
}

bool wxMySQLRecordset::Close()
{
	printf("MYSQL_STMT_CLOSE\r\n");
	bool res = (m_Stmt == NULL ? true : !mysql_stmt_close(m_Stmt));
	m_Stmt = NULL;
	if(m_ColDefs)
	{
		delete [] m_ColDefs;
		m_ColDefs = NULL;
	}
	return res;
}

wxString wxMySQLRecordset::GetLastErrorMessage()
{
	return mysql_stmt_error(m_Stmt);
}

unsigned int wxMySQLRecordset::GetFieldCount()
{
	return m_FieldCount;
}

unsigned int wxMySQLRecordset::GetRowCount()
{
	return mysql_stmt_num_rows(m_Stmt);
}

bool wxMySQLRecordset::Fetch()
{
	printf("MYSQL_STMT_FETCH\r\n");
	return !mysql_stmt_fetch(m_Stmt);
}

//--------------------------------------------------------------------------------

wxMySQL::wxMySQL()
: m_pDB(NULL), m_Hostname("localhost"), m_Port(3306), m_Username(""), m_Password("")
{	
}

wxMySQL::wxMySQL(wxString hostname, int port, wxString username, wxString password, wxString dbname)
: m_pDB(NULL)
{
	Connect(hostname, port, username, password);	
}

wxMySQL::~wxMySQL()
{
	if(IsConnected()) Disconnect();
}

bool wxMySQL::Connect(wxString hostname, int port, wxString username, wxString password, wxString dbname)
{
	m_pDB = mysql_init(NULL);
	bool failed(false);
	do
	{
		if(!m_pDB) break;
		if(!mysql_real_connect(	m_pDB, 
							hostname, 
							username, 
							password, 
							dbname, 
							port, 
							NULL, 
							0)) 
		{			
			failed = true;
			break;
		}
		m_Hostname = hostname;
		m_Port = port;
		m_Username = username;
		m_Password = password;
	}
	while(false);
	m_LastErrorMessage = mysql_error(m_pDB);
	if(failed) 
	{
		mysql_close(m_pDB);
		m_pDB = NULL;
	}
	return !failed;
}

bool wxMySQL::ChangeUser(wxString username, wxString password, wxString dbname)
{
	bool res = !mysql_change_user(m_pDB, username, password, dbname);
	if(res)
	{
		m_Username = username;
		m_Password = password;
		m_DatabaseName = dbname;
	}
	m_LastErrorMessage = mysql_error(m_pDB);
	return res;
}

wxString wxMySQL::GetLastErrorMessage()
{
	return m_LastErrorMessage;
}

bool wxMySQL::IsConnected()
{
	return (m_pDB != NULL);
}

bool wxMySQL::EnumDatabases(wxStringList & list)
{
	MYSQL_RES * res;
	MYSQL_ROW row;
	list.Clear();
	do
	{
		res = mysql_list_dbs(m_pDB,NULL);
		if(!res) break;
		while(row = mysql_fetch_row(res))
		{
			list.Add(row[0]);			
		}
		mysql_free_result(res);
	}
	while(false);
	m_LastErrorMessage = mysql_error(m_pDB);
	return true;
}

bool wxMySQL::EnumDatabases(wxArrayString & array)
{
	MYSQL_RES * res;
	MYSQL_ROW row;
	array.Clear();
	do
	{
		res = mysql_list_dbs(m_pDB,NULL);
		if(!res) break;
		while(row = mysql_fetch_row(res))
		{
			array.Add(wxString(row[0]));			
		}
		mysql_free_result(res);
	}
	while(false);
	m_LastErrorMessage = mysql_error(m_pDB);
	return true;
}

void wxMySQL::Disconnect()
{
	mysql_close(m_pDB);
	m_pDB = NULL;
	m_LastErrorMessage = "";
}

bool wxMySQL::CreateDatabase(wxString dbname)
{
	if(!IsConnected()) return false;
#ifdef USE_OLD_FUNCTIONS
	bool res = mysql_create_db(m_pDB, dbname);
	m_LastErrorMessage = mysql_error(m_pDB);
#else
	bool res = !Execute(wxString::Format("CREATE DATABASE %s",dbname));
#endif
	return !res;
}

bool wxMySQL::OpenDatabase(wxString dbname)
{
	if(!IsConnected()) return false;
	if(!Connect(m_Hostname, m_Port, m_Username, m_Password, dbname)) return false;
	return true;
}

bool wxMySQL::DropDatabase(wxString dbname)
{
#ifdef USE_OLD_FUNCTIONS
	bool res = mysql_drop_db(m_pDB, dbname);
	m_LastErrorMessage = mysql_error(m_pDB);
#else
	bool res = !Execute(wxString::Format("DROP DATABASE %s",dbname));
#endif	
	return !res;
}

unsigned int wxMySQL::GetTableCount()
{
	MYSQL_RES * res = mysql_list_tables(m_pDB, NULL);
	return mysql_num_rows(res);
}

bool wxMySQL::EnumTables(wxStringList & list, wxString wildchar)
{
	MYSQL_RES * res = mysql_list_tables(m_pDB, wildchar);
	MYSQL_ROW row;
	bool success(true);
	list.Clear();	
	do
	{
		if(!res) 
		{
			success = false;
			break;
		}
		while(row = mysql_fetch_row(res)) list.Add(row[0]);
		mysql_free_result(res);
	}
	while(false);
	m_LastErrorMessage = mysql_error(m_pDB);
	return success;
}

bool wxMySQL::EnumTables(wxArrayString & array, wxString wildchar)
{
	MYSQL_RES * res = mysql_list_tables(m_pDB, wildchar);
	MYSQL_ROW row;
	bool success(true);
	array.Clear();	
	do
	{
		if(!res) 
		{
			success = false;
			break;
		}
		while(row = mysql_fetch_row(res)) array.Add(row[0]);
		mysql_free_result(res);
	}
	while(false);
	m_LastErrorMessage = mysql_error(m_pDB);
	return success;
}

bool wxMySQL::Execute(wxString query, wxMySQLResult * result)
{	
	if(mysql_query(m_pDB, query))
	{
		m_LastErrorMessage = mysql_error(m_pDB);		
		return false;
	}
	if(result)
	{
		MYSQL_RES * res = mysql_store_result(m_pDB);
		if(!res)
		{
			m_LastErrorMessage = mysql_error(m_pDB);
			return false;
		}
		result->SetHandle(res);		
	}
	m_LastErrorMessage = mysql_error(m_pDB);
	return true;
}

unsigned long wxMySQL::GetAffectedRows()
{
	return mysql_affected_rows(m_pDB);
}

bool wxMySQL::CreateRecordset(wxString query, wxMySQLRecordset * result)
{	
	if(result->IsOk())
	{		
		if(!result->Close())
		{
			m_LastErrorMessage = result->GetLastErrorMessage();
			return false;
		}
	}
	printf("MYSQL_STMT_INIT\r\n");
	MYSQL_STMT * stmt = mysql_stmt_init(m_pDB);
	if(!stmt)
	{
		m_LastErrorMessage = mysql_error(m_pDB);
		wxLogTrace(wxTraceMask(), "CreateRecordset error !stmt -> %s", m_LastErrorMessage);
		return false;
	}
	printf("MYSQL_STMT_PREPARE\r\n");
	if (mysql_stmt_prepare(stmt, query, (unsigned long)query.Length()))
	{
		m_LastErrorMessage = mysql_stmt_error(stmt);		
		wxLogTrace(wxTraceMask(), "CreateRecordset mysql_stmt_prepare error -> %s", m_LastErrorMessage);
		return false;
	}	
	result->SetHandle(stmt);		
	return true;
}
