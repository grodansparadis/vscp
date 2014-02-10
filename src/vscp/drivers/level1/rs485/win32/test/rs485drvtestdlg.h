// can232drvTestDlg.h : header file
//
// Copyright (C) 2000-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

#if !defined(AFX_CAN232DRVTESTDLG_H__89C80B00_D749_40AE_9AB7_6DA29FD82BA7__INCLUDED_)
#define AFX_CAN232DRVTESTDLG_H__89C80B00_D749_40AE_9AB7_6DA29FD82BA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../common/can232obj.h"

/////////////////////////////////////////////////////////////////////////////
// CCan232drvTestDlg dialog

class CCan232drvTestDlg : public CDialog
{
// Construction
public:
	CCan232drvTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCan232drvTestDlg)
	enum { IDD = IDD_CAN232DRVTEST_DIALOG };
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
	//{{AFX_VIRTUAL(CCan232drvTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CCAN232Obj m_drvobj;

	// Generated message map functions
	//{{AFX_MSG(CCan232drvTestDlg)
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

#endif // !defined(AFX_CAN232DRVTESTDLG_H__89C80B00_D749_40AE_9AB7_6DA29FD82BA7__INCLUDED_)
