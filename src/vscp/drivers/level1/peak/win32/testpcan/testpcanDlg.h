// testpcanDlg.h : header file
//

#if !defined(AFX_TESTPCANDLG_H__AA79E934_D2BF_4FF6_9D10_A7FFC8F28311__INCLUDED_)
#define AFX_TESTPCANDLG_H__AA79E934_D2BF_4FF6_9D10_A7FFC8F28311__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../common/peakobj.h"

/////////////////////////////////////////////////////////////////////////////
// CTestpcanDlg dialog

class CTestpcanDlg : public CDialog
{
// Construction
public:
	CTestpcanDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestpcanDlg)
	enum { IDD = IDD_TESTPCAN_DIALOG };
	CString	m_Status;
	long	m_flags;
	CString	m_DeviceString;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestpcanDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CPeakObj m_peakObj;

	// Generated message map functions
	//{{AFX_MSG(CTestpcanDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPCANDLG_H__AA79E934_D2BF_4FF6_9D10_A7FFC8F28311__INCLUDED_)
