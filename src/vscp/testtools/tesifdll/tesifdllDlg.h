// tesifdllDlg.h : header file
//

#if !defined(AFX_TESIFDLLDLG_H__61C50471_601D_4349_8CEC_A6E8E462A5EE__INCLUDED_)
#define AFX_TESIFDLLDLG_H__61C50471_601D_4349_8CEC_A6E8E462A5EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/dllwrapper.h"

/////////////////////////////////////////////////////////////////////////////
// CTesifdllDlg dialog

class CTesifdllDlg : public CDialog
{
// Construction
public:
	CTesifdllDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTesifdllDlg)
	enum { IDD = IDD_TESIFDLL_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTesifdllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CDllWrapper m_vscpDll;

	// Generated message map functions
	//{{AFX_MSG(CTesifdllDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESIFDLLDLG_H__61C50471_601D_4349_8CEC_A6E8E462A5EE__INCLUDED_)
