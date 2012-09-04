#include "ConnectDlg.h"

enum 
{
	ID_HOSTEDIT = 10001,
	ID_USEREDIT,
	ID_PASSEDIT,
	ID_PORTEDIT
};

ConnectDlg::ConnectDlg(wxWindow * parent)
: wxDialog(parent, -1, "Connect", wxDefaultPosition, wxDefaultSize, 
		   wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX),
		   m_Host("localhost"), m_User("root"), m_Port(3306)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    wxBoxSizer* horSizer1 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer1, 0, wxGROW|wxALL, 5);

    wxStaticText* label1 = new wxStaticText( this, wxID_STATIC, _("Host:"), wxDefaultPosition, wxSize(50, -1), 0 );
    horSizer1->Add(label1, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HostEdit = new wxTextCtrl( this, ID_HOSTEDIT, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    horSizer1->Add(m_HostEdit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* horSizer2 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer2, 0, wxGROW|wxALL, 5);

    wxStaticText* label2 = new wxStaticText( this, wxID_STATIC, _("User:"), wxDefaultPosition, wxSize(50, -1), 0 );
    horSizer2->Add(label2, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* Edit2 = new wxTextCtrl( this, ID_USEREDIT, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    horSizer2->Add(Edit2, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* horSizer3 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* label3 = new wxStaticText( this, wxID_STATIC, _("Password:"), wxDefaultPosition, wxSize(50, -1), 0 );
    horSizer3->Add(label3, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* Edit3 = new wxTextCtrl( this, ID_PASSEDIT, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD );
    horSizer3->Add(Edit3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* horSizer4 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer4, 0, wxGROW|wxALL, 5);

    wxStaticText* label4 = new wxStaticText( this, wxID_STATIC, _("Port:"), wxDefaultPosition, wxSize(50, -1), 0 );
    horSizer4->Add(label4, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_PortEdit = new wxTextCtrl( this, ID_PORTEDIT, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    horSizer4->Add(m_PortEdit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* horSizer5 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer5, 0, wxGROW|wxALL, 5);

    wxStaticText* label5 = new wxStaticText( this, wxID_STATIC, _("Database:"), wxDefaultPosition, wxSize(50, -1), 0 );
    horSizer5->Add(label5, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_DBEdit = new wxTextCtrl( this, ID_PORTEDIT, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    horSizer5->Add(m_DBEdit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* horSizer6 = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horSizer6, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* okBtn = new wxButton( this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    horSizer6->Add(okBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* cancelBtn = new wxButton( this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    horSizer6->Add(cancelBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	sizer->Fit(this);
    sizer->SetSizeHints(this);
	Centre();

    m_HostEdit->SetValidator( wxGenericValidator(& m_Host) );
    Edit2->SetValidator( wxGenericValidator(& m_User) );
    Edit3->SetValidator( wxGenericValidator(& m_Password) );
    m_PortEdit->SetValidator( wxGenericValidator(& m_Port) );
	m_DBEdit->SetValidator( wxGenericValidator(& m_DBName) );
}