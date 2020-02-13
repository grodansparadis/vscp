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
// $RCSfile: NewDocument.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#if !defined(AFX_NEWDOCUMENT_H__CBBC3D00_03F3_40F5_AF81_E2F40295409C__INCLUDED_)
#define AFX_NEWDOCUMENT_H__CBBC3D00_03F3_40F5_AF81_E2F40295409C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewDocument.h : header file
//

#include "../../../../common/canal.h"

/////////////////////////////////////////////////////////////////////////////
// CNewDocument dialog

class CNewDocument : public CDialog
{
// Construction
public:
	CNewDocument(CWnd* pParent = NULL);   // standard constructor
	~CNewDocument();

	/*!
		Get a numerical data value (hex or decimal) from a string
		@param szData Strng containing value in string form
		@return The converted number
	*/
	unsigned long getDataValue( const char *szData );


	/*!
		Fill the device listbox with availabe devices
	*/
	void fillListBox( void );


	/*!
		Load device data from the registry
	*/
	void loadRegistryDeviceData( void );


	/*!
		Get device profile
		@param pDev Device item structure
		@return true on success.
	*/
	bool getDeviceProfile( devItem *pDev );

// Dialog Data
	//{{AFX_DATA(CNewDocument)
	enum { IDD = IDD_DIALOG_NEW_DOCUMENT };
	CListBox	m_ctrlListInterfaces;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDocument)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	

// Implementation
protected:

	/*!
		List with device profiles from the registry
	*/
	devItem *m_deviceList[ 256 ];

	/*!
		Selected index when closing 
	*/
	int m_selidx;

	// Generated message map functions
	//{{AFX_MSG(CNewDocument)
	afx_msg void OnButtonEditDevice();
	afx_msg void OnButtonAddDevice();
	afx_msg void OnButtonRemoveDevice();
	afx_msg void OnDoubleclickedButtonAddDevice();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWDOCUMENT_H__CBBC3D00_03F3_40F5_AF81_E2F40295409C__INCLUDED_)
