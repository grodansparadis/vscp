// DocumentProperties.cpp : implementation file
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
// $RCSfile: DocumentProperties.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include <pch.h>
#include "CanalDiagnostic.h"
#include "DocumentProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DocumentProperties dialog


CDocumentProperties::CDocumentProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDocumentProperties::IDD, pParent)
{
	m_fileName = _T("");
	
	//{{AFX_DATA_INIT(CDocumentProperties)
	m_strName = _T("");
	m_strPath = _T("");
	m_strDeviceString = _T("");
	m_strDeviceFlags = _T("");
	m_strInBufSize = _T("");
	m_strOutBufSize = _T("");
	//}}AFX_DATA_INIT
}


void CDocumentProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDocumentProperties)
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
	DDX_Text(pDX, IDC_EDIT_DEVICE_STRING, m_strDeviceString);
	DDX_Text(pDX, IDC_EDIT_DEVICE_FLAGS, m_strDeviceFlags);
	DDX_Text(pDX, IDC_EDIT_INBUF_SIZE, m_strInBufSize);
	DDX_Text(pDX, IDC_EDIT_OUTBUF_SIZE, m_strOutBufSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDocumentProperties, CDialog)
	//{{AFX_MSG_MAP(CDocumentProperties)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocumentProperties message handlers

void CDocumentProperties::OnButtonBrowse() 
{
	CFileDialog dlg( true, 
						NULL, 
						NULL, 
						OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						"driver (*.dll) | *.dll|All Files (*.*)|*.*||" );

	if ( IDOK == dlg.DoModal() ) {
	
		m_strPath = dlg.GetPathName();			

		if ( 0 == m_strName.GetLength()  ) {
			m_fileName = dlg.GetFileTitle ();
			m_strName = dlg.GetFileTitle ();
		}

		UpdateData( false );
   }
}

BOOL CDocumentProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateData( false );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDocumentProperties::OnOK() 
{
	UpdateData( true );	
	CDialog::OnOK();
}
