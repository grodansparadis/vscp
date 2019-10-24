// testifdllDlg.h : header file
//

#if !defined(AFX_TESTIFDLLDLG_H__C71AF0B9_6A8F_4646_919B_7885148B4041__INCLUDED_)
#define AFX_TESTIFDLLDLG_H__C71AF0B9_6A8F_4646_919B_7885148B4041__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CTestifdllDlg dialog

class CTestifdllDlg : public CDialog
{
// Construction
public:
	CTestifdllDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestifdllDlg)
	enum { IDD = IDD_TESTIFDLL_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestifdllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON m_hIcon;

 
	// Generated message map functions
	//{{AFX_MSG(CTestifdllDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTIFDLLDLG_H__C71AF0B9_6A8F_4646_919B_7885148B4041__INCLUDED_)
