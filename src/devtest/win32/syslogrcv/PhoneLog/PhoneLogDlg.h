// PhoneLogDlg.h : header file
//

#if !defined(AFX_PHONELOGDLG_H__BE01B76F_E54B_4297_93A8_01F8F1093673__INCLUDED_)
#define AFX_PHONELOGDLG_H__BE01B76F_E54B_4297_93A8_01F8F1093673__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SysLogListner.h"

/////////////////////////////////////////////////////////////////////////////
// CPhoneLogDlg dialog

class CPhoneLogDlg : public CDialog
{
// Construction
public:
	CPhoneLogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPhoneLogDlg)
	enum { IDD = IDD_PHONELOG_DIALOG };
	CString	m_ctrlEditStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CSysLogListner m_listner;

	// Generated message map functions
	//{{AFX_MSG(CPhoneLogDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONELOGDLG_H__BE01B76F_E54B_4297_93A8_01F8F1093673__INCLUDED_)
