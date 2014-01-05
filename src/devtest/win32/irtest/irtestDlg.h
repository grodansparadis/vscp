// irtestDlg.h : header file
//

#if !defined(AFX_IRTESTDLG_H__0EA027ED_4DAA_4883_8932_095DB970EC58__INCLUDED_)
#define AFX_IRTESTDLG_H__0EA027ED_4DAA_4883_8932_095DB970EC58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIrtestDlg dialog

class CIrtestDlg : public CDialog
{
// Construction
public:
	CIrtestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIrtestDlg)
	enum { IDD = IDD_IRTEST_DIALOG };
	CString	m_EditCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIrtestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIrtestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IRTESTDLG_H__0EA027ED_4DAA_4883_8932_095DB970EC58__INCLUDED_)
