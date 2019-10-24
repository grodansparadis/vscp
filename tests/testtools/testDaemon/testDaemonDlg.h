// testDaemonDlg.h : header file
//
// FILE: vscp.h 
//
// Copyright (C) 2002-2011 Ake Hedman akhe@eurosource.se 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: testDaemonDlg.h,v $                                       
// $Date: 2005/01/05 12:50:56 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 
//
// HISTORY:
//		021107 - AKHE	Started this file
//

#if !defined(AFX_TESTDAEMONDLG_H__D17630DF_5192_4A9D_A276_24E86EC81F64__INCLUDED_)
#define AFX_TESTDAEMONDLG_H__D17630DF_5192_4A9D_A276_24E86EC81F64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../common/canalshmem_level2_win32.h"

/////////////////////////////////////////////////////////////////////////////
// CTestDaemonDlg dialog

class CTestDaemonDlg : public CDialog
{
// Construction
public:
	CTestDaemonDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestDaemonDlg)
	enum { IDD = IDD_TESTDAEMON_DIALOG };
	CString	m_EditStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDaemonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	bool m_bOpen;

	CanalSharedMemLevel2 m_vscpif;

	// Generated message map functions
	//{{AFX_MSG(CTestDaemonDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonSendtest();
	afx_msg void OnButtonCheckMsg();
	afx_msg void OnButtonGetMsg();
	afx_msg void OnButtonOpen();
	afx_msg void OnButtonClose();
	afx_msg void OnButtonNetBroadcast();
	afx_msg void OnButtonSendtestLevel1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDAEMONDLG_H__D17630DF_5192_4A9D_A276_24E86EC81F64__INCLUDED_)
