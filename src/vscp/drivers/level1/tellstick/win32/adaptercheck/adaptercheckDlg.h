// adaptercheckDlg.h : header file
//

#if !defined(AFX_ADAPTERCHECKDLG_H__67021ED8_0BC7_4F5B_8D47_E9564DC985E3__INCLUDED_)
#define AFX_ADAPTERCHECKDLG_H__67021ED8_0BC7_4F5B_8D47_E9564DC985E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAdaptercheckDlg dialog

class CAdaptercheckDlg : public CDialog
{
// Construction
public:
	CAdaptercheckDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAdaptercheckDlg)
	enum { IDD = IDD_ADAPTERCHECK_DIALOG };
	CString	m_status;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdaptercheckDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAdaptercheckDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADAPTERCHECKDLG_H__67021ED8_0BC7_4F5B_8D47_E9564DC985E3__INCLUDED_)
