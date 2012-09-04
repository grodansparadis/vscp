#include "Main.h"
#include <stdio.h>
#include <conio.h>

int main()
{
	wxMySQL mysql;	
	if(!mysql.Connect("localhost", 3306, "root", ""))
	{
		printf("Connection error\r\n");
		getch();
		return 1;
	}
	wxArrayString dbnames;
	if(!mysql.EnumDatabases(dbnames))
	{
		printf("Enum DBs error\r\n");
		getch();
		return 1;
	}
	unsigned int i;	
	for(i = 0; i < dbnames.Count(); i++)
	{		
		if(dbnames[i] == "recordsettestdb")
		{
			if(!mysql.Execute("DROP DATABASE recordsettestdb"))
			{
				printf("Error dropping DB\r\n");
				getch();
				return 1;
			}
			break;
		}
	}
	if(!mysql.Execute("CREATE DATABASE RecordSetTestDB"))
	{
		printf("Error creating DB [%s]\r\n", mysql.GetLastErrorMessage());
		getch();
		return 1;
	}
	if(!mysql.OpenDatabase("RecordSetTestDB"))
	{
		printf("Error opening db\r\n");
		getch();
		return 1;
	}
	if(!mysql.Execute("CREATE TABLE RecSetTestTable(ID INT, Name VARCHAR(10))"))
	{
		printf("Error creating table\r\n");
		getch();
		return 1;
	}
	wxMySQLRecordset recset;
	//printf("Creating recordet for INSERT... ");
	if(!mysql.CreateRecordset("INSERT INTO RecSetTestTable (ID, Name) VALUES(?, ?)", &recset))
	{
		printf("Error creating recordset for INSERT\r\n");
		getch();
		return 1;
	}
	//printf("OK\r\n");
	int id;
	char name[1024];
	unsigned long id_length, name_length;
	id = 1;
	strcpy(name, "TestName1");
	name_length = strlen(name);
	recset.SetFieldType(0, wxMYSQL_TYPE_LONG);
	recset.SetFieldBuffer(0, &id);
	recset.SetFieldBufferLength(0, NULL);	
	recset.SetFieldType(1, wxMYSQL_TYPE_VAR_STRING);
	recset.SetFieldBuffer(1, name);
	recset.SetFieldBufferLength(1, &name_length);	
	if(!recset.BindFields())
	{
		printf("Error binding data [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}
	printf("\tInserting values (%i, %s)...\r\n", id, name);
	if(!recset.Execute())
	{
		printf("Error executing INSERT query [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}
	//printf("OK\r\n");
	id = 2;
	strcpy(name, "TestName2");
	name_length = strlen(name);
	printf("\tInserting values (%i, %s)...\r\n", id, name);
	if(!recset.Execute())
	{
		printf("Error executing INSERT query [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}
	//printf("OK\r\n");
	id = 3;
	strcpy(name, "TestName3");
	name_length = strlen(name);
	printf("\tInserting values (%i, %s)...\r\n", id, name);
	if(!recset.Execute())
	{
		printf("Error executing INSERT query [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}
	//printf("OK\r\n");
	//printf("Closing recordset... ");
	if(!recset.Close())
	{
		printf("Error closing recordset\r\n");
		getch();
		return 1;
	}
	//printf("OK\r\n");
	printf("- - - - - - - - -\r\n");
	id = 0;
	sprintf(name, "");
	//printf("Creating recordet for SELECT... ");
	if(!mysql.CreateRecordset("SELECT id, name FROM RecSetTestTable", &recset))
	{
		printf("Error creating recordset for SELECT\r\n");
		getch();
		return 1;
	}
	//printf("OK\r\n");
	//printf("Executing query... ");
	if(!recset.Execute())
	{
		printf("Error executing SELECT query [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}
	//printf("OK\r\n");

	printf("\tNumber of fields = %u\r\n", recset.GetFieldCount());	
	printf("\tNumber of records = %u\r\n", recset.GetRowCount());

	recset.SetFieldType(0, wxMYSQL_TYPE_LONG);
	recset.SetFieldBuffer(0, &id);
	recset.SetFieldBufferLength(0, &id_length);	
	recset.SetFieldType(1, wxMYSQL_TYPE_VAR_STRING);
	recset.SetFieldBuffer(1, name);
	recset.SetFieldBufferLength(1, &name_length);	

	if(!recset.BindFields())
	{
		printf("Error binding data [%s]\r\n", recset.GetLastErrorMessage());
		getch();
		return 1;
	}

	//recset.Seek(1);
	//printf("Fetching the data... ");
	while(recset.Fetch())
	{
		printf("\tResult = (%u,%s)\r\n", id, name);
	}

	//printf("Closing recordset... ");
	if(!recset.Close())
	{
		printf("Error closing recordset\r\n");
		getch();
		return 1;
	}
	//printf("OK\r\n");

	mysql.Disconnect();	
	printf("Finished, Yo!\r\n");
	getch();
	return 0;
}