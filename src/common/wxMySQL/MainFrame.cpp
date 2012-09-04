#include "MainFrame.h"
#include "ConnectDlg.h"

#ifndef max
#define max(a,b) a > b ? a : b
#endif

#ifndef min
#define min(a,b) a < b ? a : b
#endif

#define IDC_CONNECT				10001
#define IDC_DISCONNECT			10002
#define IDC_OPEN_DATABASE		10003
#define IDC_LIST_TABLES			10004
#define IDC_LIST_DATABASES		10005
#define IDC_EXECUTE				10006
#define IDC_GET_FIELDS_INFO		10007
#define IDC_CREATE_TEST_DB		10008
#define IDC_DROP_DATABASE		10009
#define IDC_CHANGE_USER			10010

BEGIN_EVENT_TABLE(MainFrame, wxFrame)	
EVT_BUTTON(IDC_CONNECT, MainFrame::OnConnect)
EVT_BUTTON(IDC_LIST_DATABASES, MainFrame::OnListDBs)
EVT_BUTTON(IDC_OPEN_DATABASE, MainFrame::OnOpenDB)
EVT_BUTTON(IDC_DROP_DATABASE, MainFrame::OnDropDB)
EVT_BUTTON(IDC_LIST_TABLES, MainFrame::OnListTables)
EVT_BUTTON(IDC_EXECUTE, MainFrame::OnExecute)
EVT_BUTTON(IDC_CREATE_TEST_DB, MainFrame::OnCreateTestDB)
EVT_BUTTON(IDC_GET_FIELDS_INFO, MainFrame::OnFieldsInfo)
EVT_BUTTON(IDC_DISCONNECT, MainFrame::OnDisconnect)
EVT_BUTTON(IDC_CHANGE_USER, MainFrame::OnChangeUser)
END_EVENT_TABLE()


MainFrame::MainFrame()
: wxFrame(NULL, -1, "wxMySQL testing application", wxDefaultPosition, wxSize(600,450))
{
	wxBoxSizer * sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);
	wxPanel * panel = new wxPanel(this, -1, wxDefaultPosition, wxSize(100, 400), wxTAB_TRAVERSAL|wxSIMPLE_BORDER);
	wxBoxSizer * panelsizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(panelsizer);
	
	connectBtn = new wxButton(panel, IDC_CONNECT, "Connect");
	changeuserBtn = new wxButton(panel, IDC_CHANGE_USER, "Change user");
	createtestdbBtn = new wxButton(panel, IDC_CREATE_TEST_DB, "Create test DB");
	listdbBtn = new wxButton(panel, IDC_LIST_DATABASES, "List databases");
	opendbBtn = new wxButton(panel, IDC_OPEN_DATABASE, "Open database");	
	dropdbBtn = new wxButton(panel, IDC_DROP_DATABASE, "Drop database");	
	listtblBtn = new wxButton(panel, IDC_LIST_TABLES, "List tables");
	executeBtn = new wxButton(panel, IDC_EXECUTE, "Execute");
	fldinfoBtn = new wxButton(panel, IDC_GET_FIELDS_INFO, "Fields info");
	disconnectBtn = new wxButton(panel, IDC_DISCONNECT, "Disconnect");

	panelsizer->Add(connectBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(changeuserBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(createtestdbBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(listdbBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(opendbBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(dropdbBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(listtblBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(executeBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(fldinfoBtn, 0, wxGROW|wxALL, 5);
	panelsizer->Add(disconnectBtn, 0, wxGROW|wxALL, 5);
	EnableButtons();
	m_LOGTextCtrl = new wxTextCtrl(this, -1, "LOG Text:", wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP);
	wxFont font(8, wxFONTFAMILY_DEFAULT,wxNORMAL,wxNORMAL, false, "Courier New");
	m_LOGTextCtrl->SetFont(font);
	sizer->Add(panel, 0, wxGROW|wxALL, 2);
	sizer->Add(m_LOGTextCtrl, 1, wxEXPAND|wxALL, 2);
	Centre();	
}

MainFrame::~MainFrame()
{
	if(mysql.IsConnected()) mysql.Disconnect();
}

void MainFrame::EnableButtons()
{
	listdbBtn->Enable(mysql.IsConnected());
	createtestdbBtn->Enable(mysql.IsConnected());
	changeuserBtn->Enable(mysql.IsConnected());
	opendbBtn->Enable(mysql.IsConnected());
	dropdbBtn->Enable(mysql.IsConnected());
	listtblBtn->Enable(mysql.IsConnected());
	executeBtn->Enable(mysql.IsConnected());
	fldinfoBtn->Enable(mysql.IsConnected());
	disconnectBtn->Enable(mysql.IsConnected());
}

void MainFrame::OnConnect(wxCommandEvent & event)
{
	ConnectDlg dlg(this);	
	if(dlg.ShowModal() == wxID_OK)
	{
		wxString mess = "\r\nConnecting... ";
		if(mysql.Connect(dlg.m_Host, dlg.m_Port, dlg.m_User, dlg.m_Password, dlg.m_DBName))
		{
			mess += "OK";
		} else mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		EnableButtons();
	}
}

void MainFrame::OnListDBs(wxCommandEvent & event)
{
	wxArrayString array;
	wxString mess = "\r\nList databases... ";	
	if(mysql.EnumDatabases(array))
	{
		mess += "OK";
		m_LOGTextCtrl->AppendText(mess);
		for(unsigned int i = 0; i < array.Count(); i++)
		{
			m_LOGTextCtrl->AppendText(wxString("\r\n\t")+array[i]);
		}
	} 
	else 
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
	}	
	EnableButtons();
}

void MainFrame::OnOpenDB(wxCommandEvent & event)
{
	wxString dbname = wxGetTextFromUser("Open database", "Database name:", "testdatabase");
	wxString mess = wxString::Format("\r\nOpening database [%s]... ", dbname);
	if(mysql.OpenDatabase(dbname))
	{
		mess += "OK";
	} else mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	EnableButtons();
}

void MainFrame::OnDropDB(wxCommandEvent & event)
{
	wxString dbname = wxGetTextFromUser("Drop database", "Database name:", "testdatabase");
	wxString mess = wxString::Format("\r\nDropping database [%s]... ", dbname);
	if(mysql.Execute(wxString::Format("DROP DATABASE %s", dbname)))
	{
		mess += "OK";
	} else mess += mysql.GetLastErrorMessage();
	m_LOGTextCtrl->AppendText(mess);
	EnableButtons();
}

void MainFrame::OnListTables(wxCommandEvent & event)
{
	wxArrayString array;
	wxString mess = "\r\nList tables... ";	
	if(mysql.EnumTables(array))
	{
		mess += "OK";
		m_LOGTextCtrl->AppendText(mess);
		for(unsigned int i = 0; i < array.Count(); i++)
		{
			m_LOGTextCtrl->AppendText(wxString("\r\n\t")+array[i]);
		}
	} 
	else 
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
	}	
	EnableButtons();
}

void MainFrame::OnExecute(wxCommandEvent & event)
{
	wxArrayString array;
	wxMySQLFieldArray fldarray;
	wxString query = wxGetTextFromUser("Execute", "Query:", "SELECT * FROM testtable");
	wxString mess = wxString::Format("\r\nExecute [%s]... ",query);
	unsigned maxfieldlen = 0, i;
	wxString value;
	if(mysql.Execute(query, &result))
	{
		mess += "OK";
		m_LOGTextCtrl->AppendText(mess);		
		result.GetFieldsInfo(fldarray);
		for(i = 0; i < fldarray.Count(); i++)
		{
			maxfieldlen = max(maxfieldlen, fldarray[i].m_Length);
		}
		maxfieldlen += 2;
		value = "";
		for(i = 0; i < fldarray.Count(); i++)
		{
			value += fldarray[i].m_Name;
			while(value.Length() % maxfieldlen) value += " ";
		}
		value = wxString("\r\n") + value;
		m_LOGTextCtrl->AppendText(value);
		while(result.Fetch(array))
		{			
			value = "";
			for(i = 0; i < array.Count(); i++)
			{
				value += array[i];
				while(value.Length() % maxfieldlen) value += " ";
			}
			value = wxString("\r\n") + value;
			m_LOGTextCtrl->AppendText(value);
		}
		m_LOGTextCtrl->AppendText(wxString::Format("\r\nAffected rows = %u", mysql.GetAffectedRows()));
	} 
	else
	{		
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
	}	
	EnableButtons();
}

void MainFrame::OnCreateTestDB(wxCommandEvent & event)
{
	wxString mess = "\r\nCreating test database... ";
	if(!mysql.CreateDatabase("testdatabase"))
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		return;
	}
	if(!mysql.OpenDatabase("testdatabase"))
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		return;
	}
	if(!mysql.Execute("CREATE TABLE testtable (ID INTEGER NOT NULL, test_field INTEGER, name VARCHAR(12) NOT NULL)"))
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		return;
	}
	for(int i = 1; i < 6; i++)
	{
		if(!mysql.Execute(wxString::Format("INSERT INTO testtable (ID, test_field, name) VALUES (%i, %i, \'Name_%i\')",i,i,i)))
		{
			mess += mysql.GetLastErrorMessage();
			m_LOGTextCtrl->AppendText(mess);
			return;
		}
	}
	mess += "OK";
	m_LOGTextCtrl->AppendText(mess);
	EnableButtons();
}

void MainFrame::OnFieldsInfo(wxCommandEvent & event)
{
	wxMySQLFieldArray array;
	wxString mess = "\r\nFields info... ";
	if(!result.GetFieldsInfo(array))
	{
		mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		return;
	}
	wxString value;
	mess += "OK";
	m_LOGTextCtrl->AppendText(mess);
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
}

void MainFrame::OnDisconnect(wxCommandEvent & event)
{
	if(mysql.IsConnected()) mysql.Disconnect();
	m_LOGTextCtrl->AppendText("\r\nDisconnect... OK");
	EnableButtons();
}

void MainFrame::OnChangeUser(wxCommandEvent & event)
{
	ConnectDlg dlg(this);
	dlg.m_HostEdit->Enable(false);
	dlg.m_PortEdit->Enable(false);
	wxString mess = "\r\nChanging user... ";
	if(dlg.ShowModal() == wxID_OK)
	{
		if(mysql.ChangeUser(dlg.m_User, dlg.m_Password, dlg.m_DBName))
		{			
			mess += "OK";
		} else mess += mysql.GetLastErrorMessage();
		m_LOGTextCtrl->AppendText(mess);
		EnableButtons();
	}
}