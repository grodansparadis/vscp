//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (https://www.vscp.org) 
//
// Copyright (C) 2000-2025 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: DocumentProperties.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_DOCUMENTPROPERTIES_H__7E37CBBA_9994_454B_847E_A9A481833D1F__INCLUDED_)
#define AFX_DOCUMENTPROPERTIES_H__7E37CBBA_9994_454B_847E_A9A481833D1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DocumentProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDocumentProperties dialog

class CDocumentProperties : public CDialog
{
// Construction
public:
	CDocumentProperties(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDocumentProperties)
	enum { IDD = IDD_DIALOG_DOC_PROPERTIES };
	CString	m_strName;
	CString	m_strPath;
	CString	m_strDeviceString;
	CString	m_strDeviceFlags;
	CString	m_strInBufSize;
	CString	m_strOutBufSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDocumentProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	/*!
		Filename for selected file without extension
	*/
	CString m_fileName;

	// Generated message map functions
	//{{AFX_MSG(CDocumentProperties)
	afx_msg void OnButtonBrowse();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCUMENTPROPERTIES_H__7E37CBBA_9994_454B_847E_A9A481833D1F__INCLUDED_)
