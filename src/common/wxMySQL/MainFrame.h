#ifndef _MAINFRAME_H
#define _MAINFRAME_H

#include "Main.h"

class MainFrame : public wxFrame
{
	wxTextCtrl * m_LOGTextCtrl;
	wxButton * connectBtn;
	wxButton * changeuserBtn;
	wxButton * createtestdbBtn;
	wxButton * listdbBtn;
	wxButton * opendbBtn;
	wxButton * dropdbBtn;
	wxButton * listtblBtn;
	wxButton * executeBtn;
	wxButton * fldinfoBtn;
	wxButton * disconnectBtn;
	wxMySQL mysql;
	wxMySQLResult result;
	void OnConnect(wxCommandEvent & event);
	void OnChangeUser(wxCommandEvent & event);
	void OnDisconnect(wxCommandEvent & event);
	void OnListDBs(wxCommandEvent & event);
	void OnOpenDB(wxCommandEvent & event);
	void OnDropDB(wxCommandEvent & event);
	void OnListTables(wxCommandEvent & event);
	void OnExecute(wxCommandEvent & event);
	void OnCreateTestDB(wxCommandEvent & event);
	void OnFieldsInfo(wxCommandEvent & event);
public:
	MainFrame();
	~MainFrame();
	void EnableButtons();
	DECLARE_EVENT_TABLE();
};

#endif
