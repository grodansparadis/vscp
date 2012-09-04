// testDaemonDlg.cpp : implementation file
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
// $RCSfile: testDaemonDlg.cpp,v $                                       
// $Date: 2005/01/05 12:50:56 $                                  
// $Author: akhe $                                              
// $Revision: 1.10 $ 
//
// HISTORY:
//		021107 - AKHE	Started this file
//

#include "stdafx.h"
#include "../common/vscp.h"
#include "testDaemon.h"
#include "testDaemonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDaemonDlg dialog

CTestDaemonDlg::CTestDaemonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDaemonDlg::IDD, pParent)
{
	m_bOpen = false;

	//{{AFX_DATA_INIT(CTestDaemonDlg)
	m_EditStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDaemonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestDaemonDlg)
	DDX_Text(pDX, IDC_EDIT_STATUS, m_EditStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestDaemonDlg, CDialog)
	//{{AFX_MSG_MAP(CTestDaemonDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SENDTEST, OnButtonSendtest)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_MSG, OnButtonCheckMsg)
	ON_BN_CLICKED(IDC_BUTTON_GET_MSG, OnButtonGetMsg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_NET_BROADCAST, OnButtonNetBroadcast)
	ON_BN_CLICKED(IDC_BUTTON_SENDTEST_LEVEL1, OnButtonSendtestLevel1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDaemonDlg message handlers

BOOL CTestDaemonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// OnSysCommand
//

void CTestDaemonDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


/////////////////////////////////////////////////////////////////////////////
// OnPaint
//
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestDaemonDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
//

HCURSOR CTestDaemonDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonSendtest
//

void CTestDaemonDlg::OnButtonSendtest() 
{
	vscpEvent vscpevent;

	vscpevent.GUID[  0 ]  = 65;
	vscpevent.GUID[  1 ]  = 66;
	vscpevent.GUID[  2 ]  = 67;
	vscpevent.GUID[  3 ]  = 3;
	vscpevent.GUID[  4 ] = 0;
	vscpevent.GUID[  5 ] = 0;
	vscpevent.GUID[  6 ] = 0;
	vscpevent.GUID[  7 ] = 0;
	vscpevent.GUID[  8 ] = 0;
	vscpevent.GUID[  9 ] = 0;
	vscpevent.GUID[ 10 ] = 0;
	vscpevent.GUID[ 11 ] = 0;
	vscpevent.GUID[ 12 ] = 0;
	vscpevent.GUID[ 13 ] = 0;
	vscpevent.GUID[ 14 ] = 0;
	vscpevent.GUID[ 15 ] = 0;

	vscpevent.vscp_class = 32022;
	vscpevent.vscp_type = 48;

	vscpevent.sizeData = 400;
	vscpevent.pdata = new unsigned char[ vscpevent.sizeData ];
	if ( NULL != vscpevent.pdata ) {
		for ( int i= 0; i<vscpevent.sizeData; i++ ) {
			vscpevent.pdata[ i ] = i % 128;
		}

		if ( m_vscpif.doCmdSend( &vscpevent ) ) {
			m_EditStatus = "Message sent.\r\n" + m_EditStatus;
		}
		else {
			m_EditStatus = "Send command failed.\r\n" + m_EditStatus;
		}
	}
	else {
		m_EditStatus = "Unable to allocate room for data.\r\n" + m_EditStatus;
	}

	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonSendtestLevel1
//

void CTestDaemonDlg::OnButtonSendtestLevel1() 
{
	vscpEvent vscpevent;

	memset( &vscpevent, 0, sizeof( vscpevent ) );

	vscpevent.vscp_class = VSCP_CLASS_ALARM;
	vscpevent.vscp_type = 0;

	vscpevent.sizeData = 4;
	vscpevent.pdata = new unsigned char[ vscpevent.sizeData ];
	if ( NULL != vscpevent.pdata ) {
		for ( int i= 0; i<vscpevent.sizeData; i++ ) {
			vscpevent.pdata[ i ] = i+1;
		}

		if ( m_vscpif.doCmdSend( &vscpevent ) ) {
			m_EditStatus = "Message sent.\r\n" + m_EditStatus;
		}
		else {
			m_EditStatus = "Send command failed.\r\n" + m_EditStatus;
		}
	}
	else {
		m_EditStatus = "Unable to allocate room for data.\r\n" + m_EditStatus;
	}

	UpdateData( false );	 	
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonCheckMsg

void CTestDaemonDlg::OnButtonCheckMsg() 
{
	char buf[ 256 ];
	int datacnt;

	if ( 0 <= ( datacnt = m_vscpif.doCmdDataAvailable() ) )  {
		m_EditStatus = "Receive command executed successfully.\r\n" + m_EditStatus;
		sprintf( buf, "Messages in input buffer; %i\r\n", datacnt );
		m_EditStatus = buf + m_EditStatus;
	}
	else {
		m_EditStatus = "Check for data available failed.\r\n" + m_EditStatus;
	}

	UpdateData( false );

}

/////////////////////////////////////////////////////////////////////////////
// OnButtonGetMsg

void CTestDaemonDlg::OnButtonGetMsg() 
{
	vscpEvent vscpevent;

	if ( m_vscpif.doCmdDataAvailable() ) {
		if ( m_vscpif.doCmdReceive( &vscpevent ) ) {
			//writeDataToDialog( &vscpmsg );
			m_EditStatus = "Receive command executed successfully.\r\n" + m_EditStatus;
		}
		else {
			m_EditStatus = "Receive command failed.\r\n" + m_EditStatus;
		}

		// Must remove VSCP data
		if ( NULL != vscpevent.pdata ) {
			delete vscpevent.pdata;
		}
	}
	else {
		AfxMessageBox("There is no data to fetch!");
	}

	UpdateData( false );

}

/////////////////////////////////////////////////////////////////////////////
// OnButtonOpen

void CTestDaemonDlg::OnButtonOpen() 
{
	if ( m_bOpen ) {
		m_EditStatus = "Device is already open.\r\n" + m_EditStatus;
	}
	else {
		if ( m_vscpif.doCmdOpen( NULL, 1 ) ) {
			m_bOpen = true;
			//m_ctrlConnectionState.SetIcon( m_hIconOpen );	
			m_EditStatus = "Interface is open.\r\n" + m_EditStatus;
		}
		else {
			m_EditStatus = "Failed to open device.\r\n" + m_EditStatus;
		}
	}

	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonClose

void CTestDaemonDlg::OnButtonClose() 
{
	if ( m_bOpen ) {
		if ( m_vscpif.doCmdClose() ) {
			m_bOpen = false;
			//m_ctrlConnectionState.SetIcon( m_hIconClosed );	
			m_EditStatus = "Interface is closed.\r\n" + m_EditStatus;
		}
		else {
			m_EditStatus = "Failed to close device.\r\n" + m_EditStatus;
		}
	}
	else {
		m_EditStatus = "Device is already closed.\r\n" + m_EditStatus;
	}	

	
	// We close anyway
	m_bOpen = false;

	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonNetBroadcast

void CTestDaemonDlg::OnButtonNetBroadcast() 
{
	CAsyncSocket sendsock;

	if ( !sendsock.Create( 0, SOCK_DGRAM, NULL ) ) {
		AfxMessageBox("Unable to create the VSCP UDP send socket!");
	}

	int rflag = 0, flag =1, len = 0;
	if (!sendsock.SetSockOpt( SO_BROADCAST, &flag, sizeof(int) ) ) {
		AfxMessageBox( "failed to set broadcast options for socket" );
			
	}
	
	vscpEvent vscpevent;

	vscpevent.GUID[  0 ]  = 0;
	vscpevent.GUID[  1 ]  = 1;
	vscpevent.GUID[  2 ]  = 2;
	vscpevent.GUID[  3 ]  = 3;
	vscpevent.GUID[  4 ]  = 4;
	vscpevent.GUID[  5 ]  = 5;
	vscpevent.GUID[  6 ]  = 6;
	vscpevent.GUID[  7 ]  = 7;
	vscpevent.GUID[  8 ]  = 8;
	vscpevent.GUID[  9 ]  = 9;
	vscpevent.GUID[ 10 ] = 10;
	vscpevent.GUID[ 11 ] = 11;
	vscpevent.GUID[ 12 ] = 12;
	vscpevent.GUID[ 13 ] = 13;
	vscpevent.GUID[ 14 ] = 14;
	vscpevent.GUID[ 15 ] = 15;

	vscpevent.vscp_class = VSCP_CLASS_ALARM;
	vscpevent.vscp_type = 0;

	vscpevent.sizeData = 400;
	vscpevent.pdata = new unsigned char[ vscpevent.sizeData ];
	if ( NULL != vscpevent.pdata ) {
	
		for ( int i= 0; i<vscpevent.sizeData; i++ ) {
			vscpevent.pdata[ i ] = 0x55;
		}
	}

	vscpEvent *pEvent = &vscpevent;
	unsigned char buf[ 512 ];
	unsigned char *p;
	unsigned short size;

	size = pEvent->sizeData + 25;
	if ( size > 512 ) return;

	// Head
	p = (unsigned char *)&pEvent->vscp_type;
	buf[ VSCP_UDP_POS_HEAD + 0 ] = *(p+0);

	// VSCP class
	p = (unsigned char *)&pEvent->vscp_class;
	buf[ VSCP_UDP_POS_CLASS + 0 ] = *(p+0); 
	buf[ VSCP_UDP_POS_CLASS + 1 ] = *(p+1); 

	// VSCP type
	p = (unsigned char *)&pEvent->vscp_type;
	buf[ VSCP_UDP_POS_TYPE + 0 ] = *(p+0); 
	buf[ VSCP_UDP_POS_TYPE + 1 ] = *(p+1); 

	// Node address
	p = (unsigned char *)&pEvent->GUID;
	memcpy( &buf[ VSCP_UDP_POS_GUID + 0 ], p, 16 );

	// VSCP Size
	p = (unsigned char *)&pEvent->sizeData;
	buf[ VSCP_UDP_POS_SIZE + 0 ] = *(p+0); 
	buf[ VSCP_UDP_POS_SIZE + 1 ] = *(p+1);

	// Data Max 487 (512- 25) bytes
	// size = sz - command_byte - control_bytes 
	memcpy( &buf[ VSCP_UDP_POS_DATA + 0 ], pEvent->pdata, pEvent->sizeData );

	// VSCP CRC
	p = (unsigned char *)&pEvent->crc;
	buf[ VSCP_UDP_POS_CRC + 0 ] = *(p+0); 
	buf[ VSCP_UDP_POS_CRC + 1 ] = *(p+1);

	// Send the message out
	unsigned short pktsize;
	pktsize = sendsock.SendTo( buf, size, VSCP_LEVEL2_UDP_PORT );

	if ( !pktsize || ( pktsize == SOCKET_ERROR ) || ( pktsize != size ) ) { 
		AfxMessageBox("error send");
		int err = GetLastError();
		return;
	}

}


