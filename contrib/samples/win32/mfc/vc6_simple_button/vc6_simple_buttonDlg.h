// vc6_simple_buttonDlg.h : header file
//

#if !defined(AFX_VC6_SIMPLE_BUTTONDLG_H__1B084D11_1B6D_483E_A60D_55E680A60551__INCLUDED_)
#define AFX_VC6_SIMPLE_BUTTONDLG_H__1B084D11_1B6D_483E_A60D_55E680A60551__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\..\..\common\vscptcpif.h"

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonDlg dialog

class CVc6_simple_buttonDlg : public CDialog
{
// Construction
public:
	CVc6_simple_buttonDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CVc6_simple_buttonDlg)
	enum { IDD = IDD_VC6_SIMPLE_BUTTON_DIALOG };
	CButton	m_ctrlBtnConnect;
	CButton	m_ctrlBtnOff;
	CButton	m_ctrlBtnOn;
	CString	m_szHostName;
	int		m_nPort;
	CString	m_szUserName;
	CString	m_szPassword;
	int		m_zone;
	int		m_subzone;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVc6_simple_buttonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

    // Flag for cnnection
    bool m_bConnected;

    // The TCP/IP
    VscpTcpIf m_tcpif;

	// Generated message map functions
	//{{AFX_MSG(CVc6_simple_buttonDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnect();
	afx_msg void OnButtonTest();
	afx_msg void OnButtonOn();
	afx_msg void OnButtonOff();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VC6_SIMPLE_BUTTONDLG_H__1B084D11_1B6D_483E_A60D_55E680A60551__INCLUDED_)
