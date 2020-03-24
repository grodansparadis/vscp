// serverDlg.h : header file
//

#if !defined(AFX_SERVERDLG_H__65947048_3D4C_4EF6_9DE0_B5039E1D7715__INCLUDED_)
#define AFX_SERVERDLG_H__65947048_3D4C_4EF6_9DE0_B5039E1D7715__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog

class CServerDlg : public CDialog
{
// Construction
public:
	CServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CServerDlg)
	enum { IDD = IDD_SERVER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__65947048_3D4C_4EF6_9DE0_B5039E1D7715__INCLUDED_)
