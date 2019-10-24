// testtcpinterfaceDlg.h : header file
//

#if !defined(AFX_TESTTCPINTERFACEDLG_H__8C20A660_BED3_4959_9DBC_8EBFFE11581C__INCLUDED_)
#define AFX_TESTTCPINTERFACEDLG_H__8C20A660_BED3_4959_9DBC_8EBFFE11581C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class wxSocketClient;

/////////////////////////////////////////////////////////////////////////////
// CTesttcpinterfaceDlg dialog

class CTesttcpinterfaceDlg : public CDialog
{
// Construction
public:
	CTesttcpinterfaceDlg(CWnd* pParent = NULL);	// standard constructor

	void getResponse( wxSocketClient& sock );
	void writeLog( LPCTSTR str );

// Dialog Data
	//{{AFX_DATA(CTesttcpinterfaceDlg)
	enum { IDD = IDD_TESTTCPINTERFACE_DIALOG };
	CEdit	m_ctrlLogWnd;
	CString	m_strLog;
	CString	m_Hostname;
	int		m_Port;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTesttcpinterfaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	HICON m_hIcon;

	bool m_bResponse;

	// Generated message map functions
	//{{AFX_MSG(CTesttcpinterfaceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonRun();
	afx_msg void OnButtonBinaryTest();
	afx_msg void OnButtonBinaryReceive();
	afx_msg void OnButtonStandardReceive();
	afx_msg void OnButtonMultiOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTTCPINTERFACEDLG_H__8C20A660_BED3_4959_9DBC_8EBFFE11581C__INCLUDED_)
