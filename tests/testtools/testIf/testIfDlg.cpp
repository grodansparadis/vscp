///////////////////////////////////////////////////////////////////////////////
// testPipeIfDlg.cpp : implementation file
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
//  (http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: testIfDlg.cpp,v $
// $Date: 2005/01/05 12:16:21 $
// $Author: akhe $
// $Revision: 1.2 $



#include "stdafx.h"
#include "testIf.h"
#include "testIfDlg.h"

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
// CTestPipeIfDlg dialog

CTestPipeIfDlg::CTestPipeIfDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestPipeIfDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestPipeIfDlg)
	m_EditStatus = _T("");
	m_MsgId = _T("");
	m_Data0 = _T("");
	m_Data1 = _T("");
	m_Data2 = _T("");
	m_Data3 = _T("");
	m_Data4 = _T("");
	m_Data5 = _T("");
	m_Data6 = _T("");
	m_Data7 = _T("");
	m_szOption1 = _T("");
	m_szOption2 = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bOpen = false;

}

CTestPipeIfDlg::~CTestPipeIfDlg()
{
	;
}

void CTestPipeIfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestPipeIfDlg)
	DDX_Control(pDX, IDC_CONECTION_STATE, m_ctrlConnectionState);
	DDX_Control(pDX, IDC_STATIC_OPTION2_LABEL, m_ctrlOption2Label);
	DDX_Control(pDX, IDC_STATIC_OPTION1_LABEL, m_ctrlOption1Label);
	DDX_Control(pDX, IDC_EDIT_OPTION2, m_ctrlEditOption2);
	DDX_Control(pDX, IDC_CHECK_ERROR, m_ctrlCheckError);
	DDX_Control(pDX, IDC_CHECK_RTR, m_ctrlCheckRTR);
	DDX_Control(pDX, IDC_CHECK_EXTENDED, m_ctrlExtended);
	DDX_Control(pDX, IDC_EDIT_OPTION, m_ctrlEditOption);
	DDX_Control(pDX, IDC_COMBO_MSG_SELECT, m_ctrlComboMsgSelect);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_ctrBtnOpen);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_EditStatus);
	DDX_Text(pDX, IDC_EDIT_MSGID, m_MsgId);
	DDX_Text(pDX, IDC_EDIT_DATABYTE0, m_Data0);
	DDX_Text(pDX, IDC_EDIT_DATABYTE1, m_Data1);
	DDX_Text(pDX, IDC_EDIT_DATABYTE2, m_Data2);
	DDX_Text(pDX, IDC_EDIT_DATABYTE3, m_Data3);
	DDX_Text(pDX, IDC_EDIT_DATABYTE4, m_Data4);
	DDX_Text(pDX, IDC_EDIT_DATABYTE5, m_Data5);
	DDX_Text(pDX, IDC_EDIT_DATABYTE6, m_Data6);
	DDX_Text(pDX, IDC_EDIT_DATABYTE7, m_Data7);
	DDX_Text(pDX, IDC_EDIT_OPTION2, m_szOption2);
	DDX_Text(pDX, IDC_EDIT_OPTION, m_szOption1);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestPipeIfDlg, CDialog)
	//{{AFX_MSG_MAP(CTestPipeIfDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND_MSG, OnButtonSendMsg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_CBN_SELCHANGE(IDC_COMBO_MSG_SELECT, OnSelchangeComboMsgSelect)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_DATA, OnButtonClearData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfDlg message handlers

BOOL CTestPipeIfDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if ( pSysMenu != NULL ) {
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if ( !strAboutMenu.IsEmpty()  ) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	loadRegistryData();

	m_ctrlComboMsgSelect.SetCurSel( COMBO_OPEN );

	return TRUE;  // return TRUE  unless you set the focus to a control
}


/////////////////////////////////////////////////////////////////////////////
//

void CTestPipeIfDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( (nID & 0xFFF0) == IDM_ABOUTBOX ) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialog::OnSysCommand(nID, lParam);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestPipeIfDlg::OnPaint()
{
	if (IsIconic()) {
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
	else {
		CDialog::OnPaint();
	}
}

//////////////////////////////////////////////////////////////////////////////
// OnQueryDragIcon
//
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
//

HCURSOR CTestPipeIfDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//////////////////////////////////////////////////////////////////////////////
// OnCancel
//

void CTestPipeIfDlg::OnCancel()
{
	saveRegistryData();
	CDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////////
// OnButtonOpen
//

void CTestPipeIfDlg::OnButtonOpen()
{
	unsigned char *p;
	unsigned char a,b,c,d;
	unsigned long flags = 0x00ff0000;

	p = (unsigned char *)&flags;
	a = *p;
	b = *(p+1);
	c = *(p+2);
	d = *(p+3);

	canalMsg Msg;
	getDataValues( &Msg );
}

//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CTestPipeIfDlg::getDataValue( const char *szData )
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


//////////////////////////////////////////////////////////////////////////////
// getAllData
//

bool CTestPipeIfDlg::getDataValues( canalMsg *pMsg )
{
	bool rv = false;
	unsigned char nData = 0;

	// Initialize message structure
	pMsg->flags = 0;
	pMsg->id = 0;
	pMsg->obid = 0;
	pMsg->sizeData = 0;
	pMsg->timestamp = 0;

	for ( int i=0; i<8; i++ ) {
		pMsg->data[ i ] = 0;
	}

	// Must have somewhere to store data
	if ( NULL == pMsg ) return rv;

	UpdateData( true );

	m_ctrlCheckError.SetCheck( 0 );

	pMsg->id = getDataValue( m_MsgId );

	// Is the ID valid?
	if ( pMsg->id  > 0x1FFFFFFF ) return false;

	pMsg->data[ 7 ] = (unsigned char)getDataValue( m_Data7 );
	if ( m_Data7.GetLength() > 0 ) nData = 8;

	pMsg->data[ 6 ] = (unsigned char)getDataValue( m_Data6 );
	if ( ( 0 == nData ) && ( m_Data6.GetLength() > 0  ) ) nData = 7;

	pMsg->data[ 5 ] = (unsigned char)getDataValue( m_Data5 );
	if ( ( 0 == nData ) && ( m_Data5.GetLength() > 0  ) ) nData = 6;

	pMsg->data[ 4 ] = (unsigned char)getDataValue( m_Data4 );
	if ( ( 0 == nData ) && ( m_Data4.GetLength() > 0  ) ) nData = 5;

	pMsg->data[ 3 ] = (unsigned char)getDataValue( m_Data3 );
	if ( ( 0 == nData ) && ( m_Data3.GetLength() > 0  ) ) nData = 4;

	pMsg->data[ 2 ] = (unsigned char)getDataValue( m_Data2 );
	if ( ( 0 == nData ) && ( m_Data2.GetLength() > 0  ) ) nData = 3;

	pMsg->data[ 1 ] = (unsigned char)getDataValue( m_Data1 );
	if ( ( 0 == nData ) && ( m_Data1.GetLength() > 0  ) ) nData = 2;

	pMsg->data[ 0 ] = (unsigned char)getDataValue( m_Data0 );
	if ( ( 0 == nData ) && ( m_Data0.GetLength() > 0  ) ) nData = 1;

	pMsg->sizeData = nData;

	// Extended message
	pMsg->flags = 0;

	if ( m_ctrlExtended.GetCheck() ) {
		pMsg->flags |= CANAL_IDFLAG_EXTENDED;
	}

	// Remote Frame
	if ( m_ctrlCheckRTR.GetCheck() ) {
		pMsg->flags |= CANAL_IDFLAG_RTR;
	}

	// Write back data to dialog
	writeDataToDialog( pMsg );

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

void CTestPipeIfDlg::writeDataToDialog( canalMsg *pMsg )
{

	ultoa( pMsg->id, m_MsgId.GetBufferSetLength( 32 ), 16 );
	m_MsgId = "0x" + m_MsgId;


	// Extended flag
	if ( pMsg->flags & CANAL_IDFLAG_EXTENDED ) {
		m_ctrlExtended.SetCheck( 1 );
	}
	else {
		m_ctrlExtended.SetCheck( 0 );
	}

	if ( pMsg->id  > 0x7ff ) {
		m_ctrlExtended.SetCheck( 1 );
	}

	// RTR flag
	if ( pMsg->flags & CANAL_IDFLAG_RTR ) {
		m_ctrlCheckRTR.SetCheck( 1 );
	}
	else {
		m_ctrlCheckRTR.SetCheck( 0 );
	}

	// Error flag
	if ( pMsg->flags & CANAL_IDFLAG_ERROR ) {
		m_ctrlCheckError.SetCheck( 1 );
	}
	else {
		m_ctrlCheckError.SetCheck( 0 );
	}

	// Clear all data fields
	m_Data0 = "";
	m_Data1 = "";
	m_Data2 = "";
	m_Data3 = "";
	m_Data4 = "";
	m_Data5 = "";
	m_Data6 = "";
	m_Data7 = "";


	if ( pMsg->sizeData > 0 ) {
		m_Data0 = "";
		ultoa( pMsg->data[ 0 ], m_Data0.GetBufferSetLength( 32 ), 16 );
		m_Data0 = "0x" + m_Data0;
	}

	if ( pMsg->sizeData > 1 ) {
		m_Data1 = "";
		ultoa( pMsg->data[ 1 ], m_Data1.GetBufferSetLength( 32 ), 16 );
		m_Data1 = "0x" + m_Data1;
	}

	if ( pMsg->sizeData > 2 ) {
		m_Data2 = "";
		ultoa( pMsg->data[ 2 ], m_Data2.GetBufferSetLength( 32 ), 16 );
		m_Data2 = "0x" + m_Data2;
	}

	if ( pMsg->sizeData > 3 ) {
		m_Data3 = "";
		ultoa( pMsg->data[ 3 ], m_Data3.GetBufferSetLength( 32 ), 16 );
		m_Data3 = "0x" + m_Data3;
	}

	if ( pMsg->sizeData > 4 ) {
		m_Data4 = "";
		ultoa( pMsg->data[ 4 ], m_Data4.GetBufferSetLength( 32 ), 16 );
		m_Data4 = "0x" + m_Data4;
	}

	if ( pMsg->sizeData > 5 ) {
		m_Data5 = "";
		ultoa( pMsg->data[ 5 ], m_Data5.GetBufferSetLength( 32 ), 16 );
		m_Data5 = "0x" + m_Data5;
	}

	if ( pMsg->sizeData > 6 ) {
		m_Data6 = "";
		ultoa( pMsg->data[ 6 ], m_Data6.GetBufferSetLength( 32 ), 16 );
		m_Data6 = "0x" + m_Data6;
	}

	if ( pMsg->sizeData > 7 ) {
		m_Data7 = "";
		ultoa( pMsg->data[ 7 ], m_Data7.GetBufferSetLength( 32 ), 16 );
		m_Data7 = "0x" + m_Data7;
	}

	UpdateData( false );
}

//////////////////////////////////////////////////////////////////////////////
// OnSelchangeComboMsgSelect
//

void CTestPipeIfDlg::OnSelchangeComboMsgSelect()
{
	switch ( m_ctrlComboMsgSelect.GetCurSel() ) {

		case COMBO_NOOP:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_OPEN:
			m_ctrlEditOption.ShowWindow( SW_SHOW );
			m_ctrlOption1Label.SetWindowText("Devicename");
			m_ctrlOption1Label.ShowWindow( SW_SHOW );
			m_ctrlEditOption2.ShowWindow( SW_SHOW );
			m_ctrlOption2Label.SetWindowText("Flags");
			m_ctrlOption2Label.ShowWindow( SW_SHOW );
			break;

		case COMBO_CLOSE:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_SEND:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_RECEIVE:
			m_ctrlOption1Label.SetWindowText("ID-Filter");
			m_ctrlEditOption.ShowWindow( SW_SHOW );
			m_ctrlOption1Label.ShowWindow( SW_SHOW );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_DATA_AVAILABLE:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_STATUS:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_STATISTICS:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_FILTER:
			m_ctrlOption1Label.SetWindowText("Filter");
			m_ctrlEditOption.ShowWindow( SW_SHOW );
			m_ctrlOption1Label.ShowWindow( SW_SHOW );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_MASK:
			m_ctrlOption1Label.SetWindowText("Mask");
			m_ctrlEditOption.ShowWindow( SW_SHOW );
			m_ctrlOption1Label.ShowWindow( SW_SHOW );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_BAUDRATE:
			m_ctrlOption1Label.SetWindowText("Baudrate");
			m_ctrlEditOption.ShowWindow( SW_SHOW );
			m_ctrlOption1Label.ShowWindow( SW_SHOW );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_VERSION:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_DLLVERSION:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_VENDORSTRING:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		case COMBO_LEVEL:
			m_ctrlEditOption.ShowWindow( SW_HIDE );
			m_ctrlOption1Label.ShowWindow( SW_HIDE );
			m_ctrlEditOption2.ShowWindow( SW_HIDE );
			m_ctrlOption2Label.ShowWindow( SW_HIDE );
			break;

		default:
			m_ctrlComboMsgSelect.SetCurSel( COMBO_OPEN );
			m_EditStatus = "Invalid item in comman combo\r\n" + m_EditStatus;
			break;
	}

	UpdateData( false );
}


//////////////////////////////////////////////////////////////////////////////
// OnButtonSendMsg
//

void CTestPipeIfDlg::OnButtonSendMsg()
{
	unsigned long filter, mask, baudrate, version, level;
	canalMsg canmsg;
	canalStatistics statistics;
	canalStatus status;
	char buf[ 256 ];
	char wrkbuf[80 ];
	int datacnt;

	UpdateData( true );

	switch ( m_ctrlComboMsgSelect.GetCurSel() ) {

		/// * * * * *  NOOP  * * * * *

		case COMBO_NOOP:
			if ( m_canalif.doCmdNOOP() ) {
				m_EditStatus = "NOOP command executed successfully.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "NOOP command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  O P E N  * * * * *

		case COMBO_OPEN:
			if ( m_bOpen ) {
				m_EditStatus = "Device is already open.\r\n" + m_EditStatus;
			}
			else {
				if ( m_szOption1.GetLength() > 0	) {
					unsigned long opt2;
					opt2 = getDataValue( m_szOption2 );
					if ( m_canalif.doCmdOpen( m_szOption1.GetBufferSetLength( m_szOption1.GetLength() ), 1 ) ) {
						m_bOpen = true;
						m_ctrlConnectionState.SetIcon( m_hIconOpen );
						m_EditStatus = "Interface is open.\r\n" + m_EditStatus;
					}
					else {
						m_EditStatus = "Failed to open device.\r\n" + m_EditStatus;
					}
				}
				else {
					m_EditStatus = "A device name must be given.\r\n" + m_EditStatus;
				}
			}
			break;

		/// * * * * *  C L O S E  * * * * *

		case COMBO_CLOSE:
			if ( m_bOpen ) {
				if ( m_canalif.doCmdClose() ) {
					m_bOpen = false;
					m_ctrlConnectionState.SetIcon( m_hIconClosed );
					m_EditStatus = "Interface is closed.\r\n" + m_EditStatus;
				}
				else {
					m_EditStatus = "Failed to close device.\r\n" + m_EditStatus;
				}
			}
			else {
				m_EditStatus = "Device is already closed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  S E N D  * * * * *

		case COMBO_SEND:
			getDataValues( &canmsg );
			if ( m_canalif.doCmdSend( &canmsg ) ) {
				m_EditStatus = "Message sent.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "Send command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  R E C E I V E  * * * * *

		case COMBO_RECEIVE:
			if ( m_canalif.doCmdReceive( &canmsg ) ) {
				writeDataToDialog( &canmsg );
				m_EditStatus = "Receive command executed successfully.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "Receive command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  D A T A  A V A I L A B L E  * * * * *

		case COMBO_DATA_AVAILABLE:
			if ( 0 <= ( datacnt = m_canalif.doCmdDataAvailable() ) )  {
				m_EditStatus = "Receive command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "Messages in input buffer; %i\r\n", datacnt );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "Check for data available failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  S T A T U S  * * * * *

		case COMBO_STATUS:
			if ( m_canalif.doCmdStatus( &status ) ) {
				m_EditStatus = "State command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "Status; %x\r\n", status );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "State command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  S T A T I S T I C S  * * * * *

		case COMBO_STATISTICS:
			if ( m_canalif.doCmdStatistics( &statistics ) ) {
				m_EditStatus = "Statistics command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "Transmitted frames; %d\r\n", statistics.cntTransmitFrames );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Transmitted data; %d\r\n", statistics.cntTransmitData );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Received frames; %d\r\n", statistics.cntReceiveFrames );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Received data; %d\r\n", statistics.cntReceiveData );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Overruns; %d\r\n", statistics.cntOverruns );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Bus Warnings; %d\r\n", statistics.cntBusWarnings );
				m_EditStatus = buf + m_EditStatus;
				sprintf( buf, "Bus Off's; %d\r\n", statistics.cntBusOff );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "Statistics command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  F I L T E R  * * * * *

		case COMBO_FILTER:
			filter = getDataValue( m_szOption2 );
			if ( m_canalif.doCmdFilter( filter ) ) {
				m_EditStatus = "Filter command executed successfully.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "Filter command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  M A S K  * * * * *

		case COMBO_MASK:
			mask = getDataValue( m_szOption2 );
			if ( m_canalif.doCmdMask( mask ) ) {
				m_EditStatus = "Mask command executed successfully.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "Mask command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  B A U D R A T E  * * * * *

		case COMBO_BAUDRATE:
			baudrate = getDataValue( m_szOption1 );
			if ( m_canalif.doCmdBaudrate( baudrate ) ) {
				m_EditStatus = "Baudrate command executed successfully.\r\n" + m_EditStatus;
			}
			else {
				m_EditStatus = "Baudrate command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  V E R S I O N  * * * * *

		case COMBO_VERSION:
			if ( m_canalif.doCmdVersion( &version ) ) {
				m_EditStatus = "Version command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "CANAL Version = %i.%i.%i\r\n",
								((version & 0xff000000) >> 24),
								((version & 0x00ff0000) >> 16),
								((version & 0x0000ff00) >> 8) );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "Version command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  D L L  - V E R S I O N  * * * * *

		case COMBO_DLLVERSION:
			if ( m_canalif.doCmdDLLVersion( &version ) ) {
				m_EditStatus = "DLL Version command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "CANAL DLL Version = %i.%i.%i\r\n",
								((version & 0xff000000) >> 24),
								((version & 0x00ff0000) >> 16),
								((version & 0x0000ff00) >> 8) );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "Version command failed.\r\n" + m_EditStatus;
			}
			break;

		/// * * * * *  V E N D O R - S T R I N G  * * * * *

		case COMBO_VENDORSTRING:
      {
			  *buf = 0;
 
			  if ( m_canalif.doCmdVendorString( buf, sizeof( buf ) ) ) {
				  sprintf( wrkbuf, "CANAL Vendor string = %s\r\n", buf );
				  m_EditStatus = "\r\nVendor String command executed successfully.\r\n" + m_EditStatus;
				  m_EditStatus = wrkbuf + m_EditStatus;
			  }
			  else {
				  m_EditStatus = "Vendor String command failed.\r\n" + m_EditStatus;
			  }
      }
			break;

		/// * * * * *  L E V E L  * * * * *

		case COMBO_LEVEL:
			if ( m_canalif.doCmdGetLevel( &level ) ) {
				m_EditStatus = "Get Level command executed successfully.\r\n" + m_EditStatus;
				sprintf( buf, "Level bita-array = %X\r\n", level );
				m_EditStatus = buf + m_EditStatus;
			}
			else {
				m_EditStatus = "Level command failed.\r\n" + m_EditStatus;
			}
			break;

		default:
			m_EditStatus = "Invalid command\r\n" + m_EditStatus;
			break;
	}

	UpdateData( false );
}


/////////////////////////////////////////////////////////////////////////////
// loadRegistryData
//

void CTestPipeIfDlg::loadRegistryData()
{
	HKEY hk;
	DWORD val;
	DWORD lv;
	DWORD type;
	char nDataBytes = 0;
	unsigned char buf[ 512 ];

	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\canal\\testpipeif",
											NULL,
											KEY_READ,
											&hk ) ) {

		// * * * canid * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"canid",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {

		}
		else {
			val = 0;
		}

		ultoa( val, m_MsgId.GetBufferSetLength( 32 ), 16 );
		m_MsgId = "0x" + m_MsgId;

		// * * * CAN Data bytes * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
													"nDataBytes",
													NULL,
													&type,
													(unsigned char *)&val,
													&lv ) ) {
			nDataBytes = (char)val;
		}

		// * * * Data 0 * * *
		if ( nDataBytes > 0 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data0",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data0.GetBufferSetLength( 32 ), 16 );
				m_Data0 = "0x" + m_Data0;
			}
			else {
				m_Data0 = "";
			}
		}

		// * * * Data 1 * * *
		if ( nDataBytes > 1 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data1",
														NULL,
														&type,
														(unsigned char *)&val,
																							&lv ) ) {
				ultoa( val, m_Data1.GetBufferSetLength( 32 ), 16 );
				m_Data1 = "0x" + m_Data1;
			}
			else {
				m_Data1 = "";
			}
		}

		// * * * Data 2 * * *
		if ( nDataBytes > 2 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data2",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data2.GetBufferSetLength( 32 ), 16 );
				m_Data2 = "0x" + m_Data2;
			}
			else {
				m_Data2 = "";
			}
		}

		// * * * Data 3 * * *
		if ( nDataBytes > 3 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data3",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data3.GetBufferSetLength( 32 ), 16 );
				m_Data3 = "0x" + m_Data3;
			}
			else {
				m_Data3 = "";
			}
		}

		// * * * Data 4 * * *
		if ( nDataBytes > 4 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,																							"data4",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data4.GetBufferSetLength( 32 ), 16 );
				m_Data4 = "0x" + m_Data4;
			}
			else {
				m_Data4 = "";
			}
		}

		// * * * Data 5 * * *
		if ( nDataBytes > 5 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data5",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data5.GetBufferSetLength( 32 ), 16 );
				m_Data5 = "0x" + m_Data5;
			}
			else {
				m_Data5 = "";
			}
		}

		// * * * Data 6 * * *
		if ( nDataBytes > 6 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data6",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data6.GetBufferSetLength( 32 ), 16 );
				m_Data6 = "0x" + m_Data6;
			}
			else {
				m_Data6 = "";
			}
		}

		// * * * Data 7 * * *
		if ( nDataBytes > 7 ) {
			lv = sizeof( DWORD );
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
														"data7",
														NULL,
														&type,
														(unsigned char *)&val,
														&lv ) ) {
				ultoa( val, m_Data7.GetBufferSetLength( 32 ), 16 );
				m_Data7 = "0x" + m_Data7;
			}
			else {
				m_Data7 = "";
			}
		}

		// * * * extended id check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bExtended",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_ctrlExtended.SetCheck( 1 ) ;
		}

		// * * * RTR check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bRTR",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_ctrlCheckRTR.SetCheck( 1 ) ;
		}

		// * * * Option1 * * *
		lv = sizeof( buf );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"Option1",
												NULL,
												&type,
												buf,
												&lv ) ) {
			m_szOption1 = buf;
		}

		// * * * Option1 * * *
		lv = sizeof( buf );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"Option2",
												NULL,
												&type,
												buf,
												&lv ) ) {
			m_szOption2 = buf;
		}

		RegCloseKey( hk );

	}

	UpdateData( false );
}


//////////////////////////////////////////////////////////////////////////////
// saveRegistryData
//
// Write persistant data to the registry
//

void CTestPipeIfDlg::saveRegistryData()
{
	canalMsg canmsg;
	HKEY hk;
	char buf[ 512 ];


	// Get screen data
	getDataValues( &canmsg );

	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\canal",
											NULL,
											KEY_ALL_ACCESS,
											&hk ) )
	{
		HKEY hk_opt;
		DWORD disp;

		RegCreateKeyEx( hk, "testpipeif", 0L,
			REG_OPTION_NON_VOLATILE,
			NULL,
			KEY_ALL_ACCESS,
			NULL,
			&hk_opt,
			&disp );

		DWORD val;

		// CAN id
		val =  canmsg.id;
		RegSetValueEx( hk_opt,
						"canid",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )
						);

		// CAN Data bytes
		for ( int i = 0; i< 8; i++ ) {
			val =  canmsg.data[ i ];
			sprintf( (char *)buf, "data%i", i );
			RegSetValueEx( hk_opt,
						(const char *)buf,
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )
						);
		}

		// # of data bytes
		val =  canmsg.sizeData;
		RegSetValueEx( hk_opt,
										"nDataBytes",
										NULL,
										REG_DWORD,
										(CONST BYTE *)&val,
										sizeof( val )	);

		// Option1
		strcpy( buf, m_szOption1 );
		RegSetValueEx( hk_opt,
										"option1",
										NULL,
										REG_SZ,
										(const unsigned char *)buf,
										sizeof( val )	);

		// Option2
		strcpy( buf, m_szOption2 );
		RegSetValueEx( hk_opt,
										"option2",
										NULL,
										REG_SZ,
										(const unsigned char *)buf,
										sizeof( val )	);

		// extended id check box
		val =  m_ctrlExtended.GetCheck();
		RegSetValueEx( hk_opt,
										"bExtended",
										NULL,
										REG_DWORD,
										(CONST BYTE *)&val,
										sizeof( val )	);

		// RTR check box
		val =  m_ctrlCheckRTR.GetCheck();
		RegSetValueEx( hk_opt,
										"bRTR",
										NULL,
										REG_DWORD,
										(CONST BYTE *)&val,
										sizeof( val )	);

		RegCloseKey( hk );
		RegCloseKey( hk_opt );
	}
}


//////////////////////////////////////////////////////////////////////////////
// OnButtonClearData

void CTestPipeIfDlg::OnButtonClearData()
{
	m_Data0 = "";
	m_Data1 = "";
	m_Data2 = "";
	m_Data3 = "";
	m_Data4 = "";
	m_Data5 = "";
	m_Data6 = "";
	m_Data7 = "";
	UpdateData( false );
}
