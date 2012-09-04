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
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#if !defined(AFX_CONFDLG_H__768417C3_C8E5_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_CONFDLG_H__768417C3_C8E5_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// confdlg.h : header file
//

#include "globals.h"

#include "irconfig.h"
#include "drvdlg.h"

/////////////////////////////////////////////////////////////////////////////
// Cconfdlg dialog

class Cconfdlg : public CDialog
{
// Construction
public:
	Cconfdlg(Cdrvdlg *nparent, CWnd* pParent = NULL);
	void WriteSettingsToParent();		

	Cdrvdlg *parent;

// Dialog Data
	//{{AFX_DATA(Cconfdlg)
	enum { IDD = IDD_CONFIG };
	CString	m_port;
	CString	m_filename;
	BOOL	m_animax;
	BOOL	m_notrayicon;
	int		m_devicetype;
	CString	m_speed;
	int		m_virtpulse;
	int		m_transmitterpin;
	BOOL	m_hardcarrier;
	BOOL	m_inverted;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cconfdlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Cconfdlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnLearn();
	afx_msg void OnAnalyze();
	afx_msg void OnRaw();
	afx_msg void OnRadiorx();
	afx_msg void OnRadiodcd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFDLG_H__768417C3_C8E5_11D2_8C7F_004005637418__INCLUDED_)
