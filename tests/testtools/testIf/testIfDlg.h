// testPipeIfDlg.h : header file
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part is part of CANAL (CAN Abstraction Layer)
// (https://www.vscp.org)
//
// Copyright (C) 2000-2025 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: testIfDlg.h,v $                                       
// $Date: 2005/01/05 12:16:21 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_TESTPIPEIFDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_)
#define AFX_TESTPIPEIFDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/canalshmem_level1_win32.h"

#define COMBO_NOOP				0
#define COMBO_OPEN				1
#define COMBO_CLOSE				2
#define COMBO_SEND				3
#define COMBO_RECEIVE			4	
#define COMBO_DATA_AVAILABLE	5
#define COMBO_STATUS			6
#define COMBO_STATISTICS		7
#define COMBO_FILTER			8
#define COMBO_MASK				9
#define COMBO_BAUDRATE			10
#define COMBO_VERSION			11
#define COMBO_DLLVERSION		12
#define COMBO_VENDORSTRING		13
#define COMBO_LEVEL				14

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfDlg dialog

class CTestPipeIfDlg : public CDialog
{
// Construction
public:
	
	/// Standard constructor
	CTestPipeIfDlg(CWnd* pParent = NULL);	
	
	/// Destructor
	~CTestPipeIfDlg();

	/**
		Get a string data value (hex or decimal).

		@param Data to convert.
		@return unsigned long representing the value.
	*/
	unsigned long getDataValue( const char *szData );

	/**
		Get data from all message fields

		@param CAN message struct
		@return true for cuccess, false otherwise.
	*/
	bool getDataValues( canalMsg *pMsg );

	/**
		Write data from a CANAL structure to the dialog.
		
		@param CAN message struct
	*/
	void writeDataToDialog( canalMsg *pMsg );

	/**
		Load persistent data from the registry
	*/
	void loadRegistryData( void )	;

	/**
		Save persistent data to the registry
	*/
	void saveRegistryData( void )	;

	/// Handle for Open connection Icon
	HICON m_hIconOpen;
	
	/// Handle for Closed connection Icon
	HICON m_hIconClosed;

// Dialog Data
	//{{AFX_DATA(CTestPipeIfDlg)
	enum { IDD = IDD_TESTPIPEIF_DIALOG };
	CStatic	m_ctrlConnectionState;
	CStatic	m_ctrlOption2Label;
	CStatic	m_ctrlOption1Label;
	CEdit	m_ctrlEditOption2;
	CButton	m_ctrlCheckError;
	CButton	m_ctrlCheckRTR;
	CButton	m_ctrlExtended;
	CEdit	m_ctrlEditOption;
	CComboBox	m_ctrlComboMsgSelect;
	CButton	m_ctrBtnOpen;
	CComboBox	m_ctrlComboMsg;
	CString	m_EditStatus;
	CString	m_MsgId;
	CString	m_Data0;
	CString	m_Data1;
	CString	m_Data2;
	CString	m_Data3;
	CString	m_Data4;
	CString	m_Data5;
	CString	m_Data6;
	CString	m_Data7;
	CString	m_szOption2;
	CString	m_szOption1;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestPipeIfDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Flag for open channel
	bool m_bOpen;

	// The canal pipe i/f
	CanalSharedMemLevel1 m_canalif;

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestPipeIfDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	afx_msg void OnButtonSendMsg();
	afx_msg void OnButtonOpen();
	afx_msg void OnSelchangeComboMsgSelect();
	afx_msg void OnButtonClearData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPIPEIFDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_)
