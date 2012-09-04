/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 */

#if !defined(AFX_DRVDLG_H__C20B80E0_C848_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_DRVDLG_H__C20B80E0_C848_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// drvdlg.h : header file
//

#include "globals.h"

#include "winlirc.h"
#include "trayicon.h"
#include "irdriver.h"
#include "irconfig.h"

class CIRDriver;
class CIRConfig;

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg dialog

class Cdrvdlg : public CDialog
{
// Construction
public:
	Cdrvdlg(CWnd* pParent = NULL);   // standard constructor
	
	// Pointer to the class that listens on the serial port
	CIRDriver driver;
	CIRConfig config;

	bool initialized;
	bool AllowTrayNotification;
	bool DoInitializeDaemon();
	bool InitializeDaemon();
	void GoGreen();
	void GoBlue();	//turns the tray icon blue to indicate a transmission
	
// Dialog Data
	//{{AFX_DATA(Cdrvdlg)
	enum { IDD = IDD_DIALOG };
	CComboBox	m_IrCodeEditCombo;
	CComboBox	m_remote_DropDown;
	CString	m_ircode_edit;
	CString	m_remote_edit;
	int	m_reps_edit;
	//}}AFX_DATA

	CTrayIcon ti;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cdrvdlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Cdrvdlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnToggleWindow();
	afx_msg void OnConfig();
	afx_msg void OnHideme();
	afx_msg void OnExitLirc();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnSendcode();
	afx_msg LRESULT OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnDropdownIrcodeEdit();
	//}}AFX_MSG
	afx_msg LRESULT OnPowerBroadcast(WPARAM uPowerEvent, LPARAM lP);
	void UpdateRemoteComboLists();
	void UpdateIrCodeComboLists();
	LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRVDLG_H__C20B80E0_C848_11D2_8C7F_004005637418__INCLUDED_)
