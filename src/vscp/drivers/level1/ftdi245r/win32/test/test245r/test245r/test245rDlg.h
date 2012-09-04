// test245rDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ftd2xx.h"

// Ctest245rDlg dialog
class Ctest245rDlg : public CDialog
{
// Construction
public:
	Ctest245rDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TEST245R_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

    void writeValue( void );

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButton1();
    CButton m_btnOpen;
    CButton m_chkD0;
    CButton m_chkD1;
    CButton m_chkD2;
    CButton m_chkD3;
    CButton m_chkD4;
    CButton m_chkD5;
    CButton m_chkD6;
    CButton m_chkD7;
    
    // Flag for open channel
    bool m_bOpen;

    // Handle for channel 
    FT_HANDLE m_ftHandle;
    unsigned char m_output;
    afx_msg void OnBnClickedCheck1();
    afx_msg void OnBnClickedCheck2();
    afx_msg void OnBnClickedCheck3();
    afx_msg void OnBnClickedCheck4();
    afx_msg void OnBnClickedButton2();
};
