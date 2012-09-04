#ifndef _WX_PIE_CTRL_MAIN_PAGE_H
#define _WX_PIE_CTRL_MAIN_PAGE_H

/**

\mainpage wxMySQL Overview
	<IMG SRC="wxMySQL.png">

      wxMySQL is a library for accessing MySQL databases from wxWidgets applications

	Classes:\n
      wxMySQLField\n      
      wxMySQLResult\n
	  wxMySQLRecordset\n
	  wxMySQL\n

	Header files:\n
      <b>wxMySQL.h:</b> Mandatory when using wxMySQL.\n

  <HR>
  <H3>Contents:</H3>
  \ref basics\n  
  \ref queries\n
  \ref enumdatabases\n
  \ref enumtables\n
  \ref createdatabase\n
  \ref fieldinfo\n
  \ref license\n
  \ref developers\n 
  \ref links\n  
  <HR>

  \section basics Connecting to MySQL server
    
\code
...
wxMySQL mysql;
...
wxString mess = "\r\nConnecting... ";
if(mysql.Connect("some_host_name", 3306, "username", "password", "some_db_name"))
{
	mess += "OK";
} else mess += mysql.GetLastErrorMessage();

\endcode

As you can see above, connecting to MySQL server is very simple. You just have to specify server's hostname, port number, username, password and database name to connect to.

\section queries Executing queries

After your connection to MySQL server has been established, you are able to execute queries

\code
...
wxMySQL mysql;
...
// Array for storing results
wxArrayString array;
wxString query = "SELECT * FROM testtable";
unsigned int i;
// Executing query
if(mysql.Execute(query, &result))
{
	//If everything is OK then retrieve result row-by-row
	while(result.Fetch(array))
	{			
		for(i = 0; i < array.Count(); i++)
		{
			// process results here
		}
	}
} 


\endcode

\section enumdatabases Enumerating database names

So, we have sucessfully connected to MySQL (see \ref basics) and now we need to enumerate all avaliable databases.
wxMySQL has EnumDatabases function which allows to retrieve all database names to wxArrayString:

\code
...
wxMySQL mysql;
...
wxTextCtrl * m_LOGTextCtrl;
...
// Array for storing results
wxArrayString array;
wxString mess = "\r\nList databases... ";	
// Enumerate databases
if(mysql.EnumDatabases(array))
{
	// If everything is OK then...
	mess += "OK";
	m_LOGTextCtrl->AppendText(mess);
	for(unsigned int i = 0; i < array.Count(); i++)
	{
		// Display results
		m_LOGTextCtrl->AppendText(wxString("\r\n\t")+array[i]);
	}
} 
else 
{
	// If error occured then display error message
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
}	

\endcode

\section enumtables Enumerating table names

You should use wxMySQL::EnumTables in case if you want to retrieve the names of all tables in database:

\code
...
wxMySQL mysql;
...
wxTextCtrl * m_LOGTextCtrl;
...
// Array for storing results
wxArrayString array;
wxString mess = "\r\nList tables... ";	
// Enumerate tables
if(mysql.EnumTables(array))
{
	// If everything is OK then...
	mess += "OK";
	m_LOGTextCtrl->AppendText(mess);
	for(unsigned int i = 0; i < array.Count(); i++)
	{
		// Display results
		m_LOGTextCtrl->AppendText(wxString("\r\n\t")+array[i]);
	}
} 
else 
{
	// If error occured then display error message
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
}
\endcode

\section createdatabase Creating new database and new table

\code
...
wxMySQL mysql;
...
wxTextCtrl * m_LOGTextCtrl;
...
wxString mess = "\r\nCreating test database... ";
// Create new database 'testdatabase'
if(!mysql.CreateDatabase("testdatabase"))
{
	// If error occured then display error message and exit
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	return;
}
// Open new database
if(!mysql.OpenDatabase("testdatabase"))
{
	// If error occured then display error message and exit
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	return;
}
// Create new table with 3 fields (ID, test_field, name)
if(!mysql.Execute("CREATE TABLE testtable (ID INTEGER NOT NULL, test_field INTEGER, name VARCHAR(12) NOT NULL)"))
{
	// If error occured then display error message and exit
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	return;
}
// Add 6 records to new table
for(int i = 1; i < 6; i++)
{
	if(!mysql.Execute(wxString::Format("INSERT INTO testtable (ID, test_field, name) VALUES (%i, %i, \'Name_%i\')",i,i,i)))
	{
		// If error occured then display error message and exit
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		return;
	}
}
mess += "OK";
m_LOGTextCtrl->AppendText(mess);
\endcode

\section fieldinfo Retrieving information about table fields

\code
...
wxMySQL mysql;
...
wxTextCtrl * m_LOGTextCtrl;
...
wxMySQLResult result;
...
// Array of fields
wxMySQLFieldArray array;
wxString mess = "\r\nFields info... ";
// Retrieve information about fields
if(!result.GetFieldsInfo(array))
{
	// If error occured then display error message and exit
	mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	return;
}
wxString value;
mess += "OK";
m_LOGTextCtrl->AppendText(mess);
// Display information about fields
for(unsigned int i = 0; i < array.Count(); i++)
{		
	value = wxString::Format("\r\n\r\n- - - - - - - - Field %u - - - - - - - \r\n", i);
	m_LOGTextCtrl->AppendText(value);	
	value = wxString::Format("\r\nName = %s", array[i].m_Name);
	m_LOGTextCtrl->AppendText(value);	
	value = wxString::Format("\r\nOriginal Name = %s", array[i].m_NameOrg);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nTable Name = %s", array[i].m_TableName);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nOriginal Table Name = %s", array[i].m_TableNameOrg);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nDatabase Name = %s", array[i].m_DBName);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nCatalog Name = %s", array[i].m_DBCatalog);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nDefault value = %s", array[i].m_DefaultValue);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nLength = %u", array[i].m_Length);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nMaxLength = %u", array[i].m_MaxLength);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nDecimals = %u", array[i].m_Decimals);
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nField type = %s", wxMySQLGetFieldTypeName(array[i].m_FieldType));
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsNotNULL = %u", array[i].IsNotNULL());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsPrimaryKeyField = %u", array[i].IsPrimaryKeyField());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsUniqueKeyField = %u", array[i].IsUniqueKeyField());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsNonUniqueKeyField = %u", array[i].IsNonUniqueKeyField());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsUnSigned = %u", array[i].IsUnSigned());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsZeroFill = %u", array[i].IsZeroFill());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsBinary = %u", array[i].IsBinary());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsAutoIncrement = %u", array[i].IsAutoIncrement());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsEnum = %u", array[i].IsEnum());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsSet = %u", array[i].IsSet());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsBLOB = %u", array[i].IsBLOB());
	m_LOGTextCtrl->AppendText(value);
	value = wxString::Format("\r\nIsTimeStamp = %u", array[i].IsTimeStamp());
	m_LOGTextCtrl->AppendText(value);
}
\endcode

\section license License

	wxMySQL is free for commercial and non-commercial use and are distributed under wxWidgets license. Feel free to modify its source code.

\section developers Developers
	
	You can contact <a href="&#109;&#97;i&#108;&#84;o&#58;t-rex&#64;mail&#46;zp&#46;ua">Volodymir (T-Rex) Tryapichko</a> in case if you have any questions about wxMySQL.

\section links wxMySQL homepage and other links
	
	Visit <A HREF="http://www.ttanalytics.nm.ru/wxcomponents/wxMySQL/index.html">project's homepage</A> to get more info and latest source\n
	You can download the latest version of wxMySQL <A HREF="http://www.ttanalytics.nm.ru/downloads.html">here</A>\n

*/

#endif