#ifndef _CONNECT_DLG_H
#define _CONNECT_DLG_H

#include "Main.h"

class ConnectDlg : public wxDialog
{
public:
	wxString m_Host;
	wxString m_User;
	wxString m_Password;
	wxString m_DBName;
	int m_Port;
	ConnectDlg(wxWindow * parent);
	wxTextCtrl* m_HostEdit;
	wxTextCtrl* m_PortEdit;
	wxTextCtrl* m_DBEdit;
};

#endif