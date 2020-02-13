//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (https://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: SendBurst.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_SENDBURST_H__9F0C4AF5_83B0_4552_A21F_89B2450113A2__INCLUDED_)
#define AFX_SENDBURST_H__9F0C4AF5_83B0_4552_A21F_89B2450113A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendBurst.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SendBurst dialog

class SendBurst : public CDialog
{
// Construction
public:
	SendBurst(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SendBurst)
	enum { IDD = IDD_DIALOG_SEND_BURST };
	CSpinButtonCtrl	m_spinInterPacketDelay;
	CSpinButtonCtrl	m_spinNumberOfPackets;
	CString	m_numberOfPackets;
	CString	m_interPacketDelay;
	BOOL	m_bAutoIncId;
	BOOL	m_bAutoIncData0;
	BOOL	m_bAutoIncData1;
	BOOL	m_bAutoIncData2;
	BOOL	m_bAutoIncData;
	BOOL	m_bAutoIncData4;
	BOOL	m_bAutoIncData5;
	BOOL	m_bAutoIncData6;
	BOOL	m_bAutoIncData7;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SendBurst)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SendBurst)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDBURST_H__9F0C4AF5_83B0_4552_A21F_89B2450113A2__INCLUDED_)
