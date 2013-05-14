// NewDocument.cpp : implementation file
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
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
// $RCSfile: NewDocument.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#include "stdafx.h"
#include "CanalDiagnostic.h"
#include "NewDocument.h"
#include "documentproperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewDocument dialog


CNewDocument::CNewDocument(CWnd* pParent /*=NULL*/)
	: CDialog(CNewDocument::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewDocument)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_selidx = -1;

	for ( int i=0; i < 256; i++ ) {
		m_deviceList[ i ] = NULL;
	}
}

CNewDocument::~CNewDocument()
{
	for ( int i=0; i < 256; i++ ) {
		if ( NULL != m_deviceList[ i ] ) {
			delete m_deviceList[ i ];
			m_deviceList[ i ] = NULL;
		}
	}	
}

void CNewDocument::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewDocument)
	DDX_Control(pDX, IDC_LIST1, m_ctrlListInterfaces);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewDocument, CDialog)
	//{{AFX_MSG_MAP(CNewDocument)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DEVICE, OnButtonEditDevice)
	ON_BN_CLICKED(IDC_BUTTON_ADD_DEVICE, OnButtonAddDevice)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_DEVICE, OnButtonRemoveDevice)
	ON_BN_DOUBLECLICKED(IDC_BUTTON_ADD_DEVICE, OnDoubleclickedButtonAddDevice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewDocument message handlers

/////////////////////////////////////////////////////////////////////////////
// OnInitDialog

BOOL CNewDocument::OnInitDialog() 
{
	CDialog::OnInitDialog();	

	fillListBox();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////
// fillListBox
//

void CNewDocument::fillListBox( void )
{
	for ( int i = 0; i< 256; i++ ) {
		m_deviceList[ i ] = NULL;	
	}
	
	m_ctrlListInterfaces.AddString( "canal daemon interface" );
	m_ctrlListInterfaces.SetCurSel( 0 );

	loadRegistryDeviceData();

	for ( i = 0; i<256; i++ ) {
		if ( NULL != m_deviceList[ i ] ) {
			m_ctrlListInterfaces.AddString( (char *)m_deviceList[ i ]->name );	
		}
	}

}

//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CNewDocument::getDataValue( const char *szData )
{
	unsigned long val;
	char *nstop;

	if ( ( NULL != strchr( szData, 'x' ) ) ||
			( NULL != strchr( szData, 'X' ) ) ) {
		val = strtoul( szData, &nstop, 16 );
	}
	else {
		val = strtoul( szData, &nstop, 10 );
	}

	return val;	
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonEditDevice

void CNewDocument::OnButtonEditDevice() 
{
	int idx = m_ctrlListInterfaces.GetCurSel();

	if ( 0 == idx ) {
		AfxMessageBox("Noting to edit for this built in interface!");	
	}
	else if ( -1 != idx ) {
		
		CDocumentProperties dlg;

		dlg.m_strName = m_deviceList[ idx - 1 ]->name;
		dlg.m_strPath = m_deviceList[ idx - 1 ]->path;
		dlg.m_strDeviceString = m_deviceList[ idx - 1 ]->deviceStr;
		_ultoa( m_deviceList[ idx - 1 ]->flags, dlg.m_strDeviceFlags.GetBufferSetLength( 32 ), 10 );
		itoa( m_deviceList[ idx - 1 ]->inbufsize, dlg.m_strInBufSize.GetBufferSetLength( 32 ), 10 );
		itoa( m_deviceList[ idx - 1 ]->outbufsize, dlg.m_strOutBufSize.GetBufferSetLength( 32 ), 10 );
	
		if ( IDOK == dlg.DoModal() ) {
			
			// Get Data
			strncpy( (char *)m_deviceList[ idx - 1 ]->name, dlg.m_strName, sizeof( m_deviceList[ idx - 1 ]->name ) );
			strncpy( (char *)m_deviceList[ idx - 1 ]->path, dlg.m_strPath, MAX_PATH );
			strncpy( (char *)m_deviceList[ idx - 1 ]->deviceStr, dlg.m_strDeviceString, MAX_PATH );
			
			m_deviceList[ idx - 1 ]->inbufsize = atoi( dlg.m_strOutBufSize ); 
			m_deviceList[ idx - 1 ]->outbufsize = atoi( dlg.m_strInBufSize );
			m_deviceList[ idx - 1 ]->flags = atol( dlg.m_strDeviceFlags  );

			// Fix listbox
			m_ctrlListInterfaces.DeleteString( idx );
			m_ctrlListInterfaces.InsertString( idx, (char *)m_deviceList[ idx - 1 ]->name );
			m_ctrlListInterfaces.SetCurSel( idx );

			// Fix registry
			char buf[ 2048 ];
			char key[ 80 ];
			sprintf( buf, 
						"%s,%s,%s,%u,%u,%u", 
						m_deviceList[ idx - 1 ]->name, 
						m_deviceList[ idx - 1 ]->deviceStr, 
						m_deviceList[ idx - 1 ]->path, 
						m_deviceList[ idx - 1 ]->inbufsize, 
						m_deviceList[ idx - 1 ]->outbufsize, 
						m_deviceList[ idx - 1 ]->flags );  
			
			sprintf( key, "device%i", m_deviceList[ idx - 1 ]->id );

			HKEY hk;
			if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\canal\\diagnostic\\",
											NULL,
											KEY_ALL_ACCESS,				
											&hk ) ) {				
				RegSetValueEx( hk,
						key,
						NULL,
						REG_SZ,
						(CONST BYTE *)buf,
						sizeof( buf )		
						);

				RegCloseKey( hk );
			}
		}
	}	
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonAddDevice

void CNewDocument::OnButtonAddDevice() 
{
	int highRegIdx = 0;
	int highLbIdx = 0;

	CDocumentProperties dlg;
	 
	if ( IDOK == dlg.DoModal() ) {

		// Find unused device devicelist entry
		bool bfoundDevId = false;
		for ( int i=0; i < 256; i++ ) {
			if ( !bfoundDevId && ( NULL == m_deviceList[ i ] ) ) {
				highLbIdx = i;
				bfoundDevId = true;
			}

			if ( NULL != m_deviceList[ i ] ) {
				if ( m_deviceList[ i ]->id > highRegIdx ) {
					highRegIdx = m_deviceList[ i ]->id;
				}
			}
		}

		// Next available is old highest + one
		if ( 0 != highRegIdx )  highRegIdx++;

		devItem *pDev = new devItem;

		if ( NULL != pDev ) {
		
			m_deviceList[ highLbIdx ] = pDev;
			m_deviceList[ highLbIdx ]->id = highRegIdx;		
			
			// Get Data
			strncpy( (char *)m_deviceList[ highLbIdx ]->name, dlg.m_strName, sizeof( m_deviceList[ highLbIdx ]->name ) );
			strncpy( (char *)m_deviceList[ highLbIdx ]->path, dlg.m_strPath, MAX_PATH );
			strncpy( (char *)m_deviceList[ highLbIdx ]->deviceStr, dlg.m_strDeviceString, MAX_PATH );
			m_deviceList[ highLbIdx ]->inbufsize = atoi( dlg.m_strOutBufSize );
			m_deviceList[ highLbIdx ]->outbufsize = atoi( dlg.m_strInBufSize );
			m_deviceList[ highLbIdx ]->flags = atol( dlg.m_strDeviceFlags  );

			// Fix listbox
			int idx = m_ctrlListInterfaces.AddString( (char *)m_deviceList[ highRegIdx ]->name );
			if ( -1 != idx ) {
				m_ctrlListInterfaces.SetCurSel( idx );
			}

			// Fix registry
			char buf[ 2048 ];
			char key[ 80 ];
			sprintf( buf, 
						"%s,%s,%s,%u,%u,%u", 
						m_deviceList[ highRegIdx ]->name, 
						m_deviceList[ highRegIdx ]->deviceStr, 
						m_deviceList[ highRegIdx ]->path, 
						m_deviceList[ highRegIdx ]->inbufsize, 
						m_deviceList[ highRegIdx ]->outbufsize, 
						m_deviceList[ highRegIdx ]->flags );  
			
			sprintf( key, "device%i", m_deviceList[ highRegIdx ]->id );

			HKEY hk;
			if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
												"software\\canal\\diagnostic\\",
												NULL,
												KEY_ALL_ACCESS,				
												&hk ) ) {				
				RegSetValueEx( hk,
								key,
								NULL,
								REG_SZ,
								(CONST BYTE *)buf,
								sizeof( buf ) );

				RegCloseKey( hk );

			}
		}
		else {
			AfxMessageBox("Unable to add new data. Possible memory problem!");
		}
	}		
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonRemoveDevice

void CNewDocument::OnButtonRemoveDevice() 
{
	int idx = m_ctrlListInterfaces.GetCurSel();

	if ( 0 == idx ) {
		AfxMessageBox("This item can not be removed!");	
	}
	else if ( -1 != idx ) {
		if ( IDYES == AfxMessageBox("Are you sure that this item should be deleted?", 
				MB_YESNO ) ) {			
			devItem *pDev = m_deviceList[ idx - 1 ];
			m_deviceList[ idx - 1 ] = NULL;	

			m_ctrlListInterfaces.DeleteString( idx );

			if ( NULL != pDev ) {
				char key[ 80 ];
				
				sprintf( key, "device%i", pDev->id );
	
				HKEY hk;
				if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
														"software\\canal\\diagnostic\\",
														NULL,
														KEY_ALL_ACCESS,				
														&hk ) ) {				
					RegDeleteValue( hk, key );
					RegCloseKey( hk );

				}

				delete pDev;
			}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
// OnDoubleclickedButtonAddDevice

void CNewDocument::OnDoubleclickedButtonAddDevice() 
{
	// TODO: Add your control notification handler code here
	
}

/////////////////////////////////////////////////////////////////////////////
// loadRegistryDeviceData

void CNewDocument::loadRegistryDeviceData()	
{
	int i, idx;
	HKEY hk;
	DWORD lv;
	DWORD type;
	char szDeviceKey[ 32 ];
	unsigned char buf[2048];
	
	idx = 0;

	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\canal\\diagnostic\\",
											NULL,
											KEY_ALL_ACCESS,
											&hk ) ) {
				
		// A maximum of 256 devices
		for( i=0; i<256; i++ ) {
			sprintf( szDeviceKey, "device%i", i );
			lv = sizeof( buf );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												szDeviceKey,
												NULL,
												&type,
												buf,
												&lv ) ) {
				if ( ( REG_SZ == type ) && ( 1 != lv ) ) {
					char *p;
					int inBufSize = 0;
					int outBufSize = 0;
					unsigned long flags = 0;

					char *pName = strtok( (char *)buf, "," );
					char *pDevice = strtok( NULL, "," );
					char *pDLL = strtok( NULL, "," );

					// inbufsize
					p = strtok( NULL, "," );
					if ( NULL != p ) inBufSize = atoi( p );
					
					// outbufsize
					p = strtok( NULL, "," );
					if ( NULL != p ) outBufSize = atoi( p );
					
					// flags
					p = strtok( NULL, "," );
					if ( NULL != p ) flags = atol( p );

					// Add the device
					devItem *pDev = new devItem;
					if ( NULL != pDev ) {
						
						pDev->id = i+1;
						strncpy( (char *)pDev->name, pName, sizeof( pDev->name ) );   
						strncpy( (char *)pDev->deviceStr, pDevice, MAX_PATH ); 
						strncpy( (char *)pDev->path, pDLL, MAX_PATH ); 
						pDev->flags = flags;
						pDev->inbufsize = inBufSize;
						pDev->outbufsize = outBufSize;

						m_deviceList[ idx++ ] = pDev;
					}				
				}
			}
		}

		RegCloseKey( hk );

	}
}

/////////////////////////////////////////////////////////////////////////////
// getDeviceProfile

bool CNewDocument::getDeviceProfile( devItem *pDev )
{
	bool rv = true;

	// Must be a valid device pointer
	if ( NULL == pDev ) return false;
	
	// ust be a selected item
	if ( -1 == m_selidx ) {
		return false;
	}
	else if ( 0 == m_selidx ) {
		pDev->id = 0;		
	}
	else {
		memcpy( pDev, m_deviceList[ m_selidx - 1 ], sizeof( devItem ) );	
	}
	
	return rv;
}

/////////////////////////////////////////////////////////////////////////////
// OnOK

void CNewDocument::OnOK() 
{
	m_selidx = m_ctrlListInterfaces.GetCurSel();
	
	CDialog::OnOK();
}
