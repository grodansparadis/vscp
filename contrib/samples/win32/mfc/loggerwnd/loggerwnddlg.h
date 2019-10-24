// loggerWndDlg.h : header file
//

#if !defined(AFX_LOGGERWNDDLG_H__07D16A89_51B0_4585_861A_6FFD5D1AD780__INCLUDED_)
#define AFX_LOGGERWNDDLG_H__07D16A89_51B0_4585_861A_6FFD5D1AD780__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/// List colors
#define LIST_FONT_COLOR			RGB( 0,0,255 )
#define LIST_BACK_COLOR			RGB( 240,255,255 )

/// Column sizes
#define LIST_WIDTH_TIME				150
#define LIST_WIDTH_TIMESTAMP		75
#define LIST_WIDTH_FLAGS			75
#define LIST_WIDTH_ID				75

/// Max number of lits items
#define MAX_LISTITEMS				1000

#include "../common/canalshmem_level1_win32.h"

/////////////////////////////////////////////////////////////////////////////
// CLoggerWndDlg dialog

class CLoggerWndDlg : public CDialog
{
// Construction
public:
	CLoggerWndDlg(CWnd* pParent = NULL);	// standard constructor

	void fillData( void );

	void insertItem( canalMsg *pMsg, int idx );

// Dialog Data
	//{{AFX_DATA(CLoggerWndDlg)
	enum { IDD = IDD_LOGGERWND_DIALOG };
	CStatic	m_ctrFrame;
	CEdit	m_ctrlLog;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoggerWndDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	HICON m_hIcon;

	// The list window
	CListCtrl	m_ctrlList;

	// The canal pipe i/f
	CanalSharedMemLevel1 m_canalif;

	int m_idx;

	UINT m_uTimerID;

	// Generated message map functions
	//{{AFX_MSG(CLoggerWndDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnFilter();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGGERWNDDLG_H__07D16A89_51B0_4585_861A_6FFD5D1AD780__INCLUDED_)
