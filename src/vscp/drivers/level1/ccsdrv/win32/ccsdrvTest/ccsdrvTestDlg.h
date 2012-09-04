// ccsdrvTestDlg.h : header file
//

#if !defined(AFX_CCSDRVTESTDLG_H__DB07F138_0A91_4C67_9915_A72470AD5FED__INCLUDED_)
#define AFX_CCSDRVTESTDLG_H__DB07F138_0A91_4C67_9915_A72470AD5FED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../common/ccsobj.h"

/////////////////////////////////////////////////////////////////////////////
// CCcsdrvTestDlg dialog

class CCcsdrvTestDlg : public CDialog
{
// Construction
public:
	CCcsdrvTestDlg(CWnd* pParent = NULL);	// standard constructor


	CCSObj m_drvobj;

// Dialog Data
	//{{AFX_DATA(CCcsdrvTestDlg)
	enum { IDD = IDD_CCSDRVTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
		CString	m_strStatus;
		CString	m_strMask;
		CString	m_strCode;
		CString	m_strData0;
		CString	m_strData1;
		CString	m_strData2;
		CString	m_strData3;
		CString	m_strData4;
		CString	m_strData5;
		CString	m_strData6;
		CString	m_strData7;
		CString	m_strDLC;
		CString	m_strCANID;
		BOOL	m_bExteded;
		CString	m_strOpenArg;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCcsdrvTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCcsdrvTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
		afx_msg void OnButtonOpen();
	afx_msg void OnButtonClose();
	afx_msg void OnButtonSend();
	afx_msg void OnButtonPoll();
	afx_msg void OnButtonFilter();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonStatus();
	afx_msg void OnButtonCheckData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCSDRVTESTDLG_H__DB07F138_0A91_4C67_9915_A72470AD5FED__INCLUDED_)
