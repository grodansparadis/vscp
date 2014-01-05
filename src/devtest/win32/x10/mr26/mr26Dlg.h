// mr26Dlg.h : header file
//

#if !defined(AFX_MR26DLG_H__AFF9E5A4_1A70_4D43_B27E_038534687CC8__INCLUDED_)
#define AFX_MR26DLG_H__AFF9E5A4_1A70_4D43_B27E_038534687CC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "comm.h"

/////////////////////////////////////////////////////////////////////////////
// CMr26Dlg dialog

class CMr26Dlg : public CDialog
{
// Construction
public:
	CMr26Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMr26Dlg)
	enum { IDD = IDD_MR26_DIALOG };
	CString	m_StrStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMr26Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CComm m_comm;

	// Generated message map functions
	//{{AFX_MSG(CMr26Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonNewline();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MR26DLG_H__AFF9E5A4_1A70_4D43_B27E_038534687CC8__INCLUDED_)
