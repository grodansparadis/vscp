// testtcpinterfaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../../../vscp/common/vscp.h"


#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include "winsock2.h"

#include "testtcpinterface.h"
#include "testtcpinterfaceDlg.h"

#include "wx/socket.h"
#include "wx/datetime.h"

#include "../../common/vscptcpif.h"

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
// CTesttcpinterfaceDlg dialog

CTesttcpinterfaceDlg::CTesttcpinterfaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTesttcpinterfaceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTesttcpinterfaceDlg)
	m_strLog = _T("");
	m_Hostname = _T("localhost");
	m_Port = VSCP_LEVEL2_TCP_PORT;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	//m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CTesttcpinterfaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTesttcpinterfaceDlg)
	DDX_Control(pDX, IDC_EDIT_LIST, m_ctrlLogWnd);
	DDX_Text(pDX, IDC_EDIT_LIST, m_strLog);
	DDX_Text(pDX, IDC_EDIT_HOSTNAME, m_Hostname);
	DDX_Text(pDX, IDC_EDIT_PORT, m_Port);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTesttcpinterfaceDlg, CDialog)
	//{{AFX_MSG_MAP(CTesttcpinterfaceDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RUN, OnButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_FAST_TEST, OnButtonBinaryTest)
	ON_BN_CLICKED(IDC_BUTTON_FAST_RECEIVE, OnButtonBinaryReceive)
	ON_BN_CLICKED(IDC_BUTTON_STANDARD_RECEIVE, OnButtonStandardReceive)
	ON_BN_CLICKED(IDC_BUTTON_MULTI_OPEN, OnButtonMultiOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()





/////////////////////////////////////////////////////////////////////////////
// CTesttcpinterfaceDlg message handlers

BOOL CTesttcpinterfaceDlg::OnInitDialog()
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
	
	m_bResponse = false;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}





///////////////////////////////////////////////////////////////////////////////
//  OnSysCommand
//

void CTesttcpinterfaceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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





///////////////////////////////////////////////////////////////////////////////
//  OnPaint
//

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTesttcpinterfaceDlg::OnPaint() 
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





///////////////////////////////////////////////////////////////////////////////
//  OnQueryDragIcon
//

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTesttcpinterfaceDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}





///////////////////////////////////////////////////////////////////////////////
//  
//

void CTesttcpinterfaceDlg::OnButtonRun() 
{
	int i;
	wxString wxstr;
	wxSocketClient socket;

	wxIPV4address addr;
	

	VscpTcpIf tcpif;

	UpdateData( true );
	

  addr.Hostname( wxString( m_Hostname ) );
	addr.Service( m_Port );

	socket.Connect( addr, false );

  m_strLog = "";
	UpdateData( false );
	

	writeLog("Waiting for connection");
	
	if ( socket.WaitOnConnect( 30 ) ) {	
		if ( socket.IsConnected() ) {
			writeLog("Connected");
		}
		else {
			AfxMessageBox("Failed to open channel");
			return;
		}
	}
	else {
		writeLog( "Failed to connect" );
		return;
	}
  UpdateData( false );

	getResponse( socket );

	
	writeLog("Test, multicommand");
	wxstr = "NOOP     \r\n NOOP  \r\nNOOP   \r\n    NOOP\r\n";
	socket.Write( wxstr, wxstr.Length() );

	Sleep( 500 );
	getResponse( socket );
  UpdateData( false );



	writeLog("Username");
	wxstr = "USER admin\r\n";
	socket.Write( wxstr, wxstr.Length() );

	getResponse( socket );
  UpdateData( false );




	writeLog("Password");
	wxstr = "PASS secret\r\n";
	socket.Write( wxstr, wxstr.Length() );

	getResponse( socket );

	// Return if not valid login
	if ( !m_bResponse ) return;

	// 
	writeLog("Write full GUID event");
	wxstr = "SEND 0,11,12,0,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,1,2,3,4,5\r\n";
	socket.Write( wxstr, wxstr.Length() );
	
	getResponse( socket );



	// 
	writeLog("Write Short form GUID event");
	wxstr = "SEND 0,11,12,0,0,-,1,2,3,4,5\r\n";
	socket.Write( wxstr, wxstr.Length() );
	
	getResponse( socket );



	// 
	writeLog("Write 200 events");
	for ( i=0; i<200; i++ ) {
		
		wxstr = wxstr.Format("SEND 0,11,12,0,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,1,2,3,4,%d\r\n", i % 256 );
		socket.Write( wxstr, wxstr.Length() );
		if ( socket.LastCount() != wxstr.Length() ) {
			m_strLog = m_strLog + "\r\nWrong number of bytes written.";
		}
		
		getResponse( socket );

	}

	
	// NOOP
	writeLog("NOOP - No opertion");
	wxstr = "NOOP\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	// CDTA
	writeLog("CDTA - Check if there are some messages in the queue");
	wxstr = "CDTA\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	// STAT
	writeLog("STAT - Statistics");
	wxstr = "STAT\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	// INFO
	writeLog("INFO - Information");
	wxstr = "INFO\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	//  CHID
	writeLog("CHID - Get channel");
	wxstr = " CHID\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	//  VERS
	writeLog("VERS - Get daemon version");
	wxstr = "VERS\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	//   GGID
	writeLog("GGID - Get GUID");
	wxstr = "GGID\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );




	//   SGID
	writeLog("SGID - Set GUID");
	wxstr = "SGID 4:6:6:5:7:4:1:3:4:3:0:1:2:3:4:5\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );



	//   GGID
	writeLog("GGID - Get GUID");
	wxstr = "GGID\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );



	// QUIT
	writeLog("QUIT");
	wxstr = "QUIT\r\n";
	socket.Write( wxstr, wxstr.Length() );
	getResponse( socket );


	socket.Close();
 
	writeLog("Socket closed");
	
	// * * * Test the programatic interface * * *

	// Open channel
  wxString str = _("admin;secret;");
  str += m_Hostname;
  str += _("\0");
	tcpif.doCmdOpen( str );

	vscpEvent event; 
	vscpEventEx eventex;


	event.head = VSCP_PRIORITY_3; 
	event.obid = 0;
	event.timestamp = 0;
	event.vscp_class = 10;
	event.vscp_type = 22;

	event.GUID[0] = 0x99;
	event.GUID[1] = 0x02;
	event.GUID[2] = 0xff;
	event.GUID[3] = 0xff;
	event.GUID[4] = 0xff;
	event.GUID[5] = 0xff;
	event.GUID[6] = 0xff;
	event.GUID[7] = 0xff;
	event.GUID[8] = 0xff;
	event.GUID[9] = 0xff;
	event.GUID[10] = 0xff;
	event.GUID[11] = 0xff;
	event.GUID[12] = 0xff;
	event.GUID[13] = 0xff;
	event.GUID[14] = 0xff;
	event.GUID[15] = 0xff;

	event.pdata = new unsigned char[ 6 ];
	if ( NULL != event.pdata ) {

		event.sizeData = 6;
		event.pdata[ 0 ] = 11;
		event.pdata[ 1 ] = 22;
		event.pdata[ 2 ] = 33;
		event.pdata[ 3 ] = 44;
		event.pdata[ 4 ] = 55;
		event.pdata[ 5 ] = 66;
		
		tcpif.doCmdSend( &event );

		delete [] event.pdata;
	}

	event.head = VSCP_PRIORITY_3; 
	event.obid = 0;
	event.timestamp = 0;
	event.vscp_class = 11;
	event.vscp_type = 23;

	event.GUID[0] = 0x98;
	event.GUID[1] = 0x02;
	event.GUID[2] = 0xff;
	event.GUID[3] = 0xff;
	event.GUID[4] = 0xff;
	event.GUID[5] = 0xff;
	event.GUID[6] = 0xff;
	event.GUID[7] = 0xff;
	event.GUID[8] = 0xff;
	event.GUID[9] = 0xff;
	event.GUID[10] = 0xff;
	event.GUID[11] = 0xff;
	event.GUID[12] = 0xff;
	event.GUID[13] = 0xff;
	event.GUID[14] = 0xff;
	event.GUID[15] = 0xff;

	event.pdata = new unsigned char[6];
	if ( NULL != event.pdata ) {

		event.sizeData = 6;
		event.pdata[ 0 ] = 11;
		event.pdata[ 1 ] = 22;
		event.pdata[ 2 ] = 33;
		event.pdata[ 3 ] = 44;
		event.pdata[ 4 ] = 55;
		event.pdata[ 5 ] = 66;
		
		tcpif.doCmdSend( &event );

		delete [] event.pdata;
	}


	eventex.head = VSCP_PRIORITY_3; 
	eventex.obid = 0;
	eventex.timestamp = 0;
	eventex.vscp_class = 11;
	eventex.vscp_type = 23;

	eventex.GUID[0] = 0x97;
	eventex.GUID[1] = 0x02;
	eventex.GUID[2] = 0xff;
	eventex.GUID[3] = 0xff;
	eventex.GUID[4] = 0xff;
	eventex.GUID[5] = 0xff;
	eventex.GUID[6] = 0xff;
	eventex.GUID[7] = 0xff;
	eventex.GUID[8] = 0xff;
	eventex.GUID[9] = 0xff;
	eventex.GUID[10] = 0xff;
	eventex.GUID[11] = 0xff;
	eventex.GUID[12] = 0xff;
	eventex.GUID[13] = 0xff;
	eventex.GUID[14] = 0xff;
	eventex.GUID[15] = 0xff;

	eventex.sizeData = 4;
	eventex.data[ 0 ] = 01;
	eventex.data[ 1 ] = 02;
	eventex.data[ 2 ] = 03;
	eventex.data[ 3 ] = 04;
		
	tcpif.doCmdSendEx( &eventex );

	// Close channel
	tcpif.doCmdClose();


	writeLog("Done.\r\n");
}




///////////////////////////////////////////////////////////////////////////////
//  getResponse
//

void CTesttcpinterfaceDlg::getResponse( wxSocketClient& sock )
{
	char buf[ 2048 ];
	Sleep( 10 );
	memset( buf, 0, sizeof( buf ) );
	sock.Read( buf, sizeof( buf ) );
	if ( '-' == *buf ) {
		m_bResponse = false;
	}
	else if ( '+' == *buf ) {
		m_bResponse = true;
	}
	m_strLog = m_strLog + "\r\n";
	m_strLog = m_strLog + buf;
	UpdateData( false );
}




///////////////////////////////////////////////////////////////////////////////
//  writeLog
//

void CTesttcpinterfaceDlg::writeLog( LPCTSTR str )
{
	m_strLog = m_strLog + "\r\n";
	m_strLog = m_strLog + str;
	UpdateData( false );
  m_ctrlLogWnd.RedrawWindow();
}



///////////////////////////////////////////////////////////////////////////////
//  OnButtonBinaryTest
//

void CTesttcpinterfaceDlg::OnButtonBinaryTest() 
{
	VscpTcpIf tcpif;
	vscpEvent event; 
	vscpEventEx eventex;

	m_strLog = "";
	UpdateData( false );
	
	// Open channel
  wxString str(_("admin;secret;"));
  str += m_Hostname;
  str += _("\0");
	tcpif.doCmdOpen( str );

	// * * * BINARY * * *
	if ( tcpif.doCmdBinary() ) {
	
		event.head = VSCP_PRIORITY_3; 
		event.obid = 0;
		event.timestamp = 0;
		event.vscp_class = 100;
		event.vscp_type = 22;

		event.GUID[0] = 0x01;
		event.GUID[1] = 0x02;
		event.GUID[2] = 0xff;
		event.GUID[3] = 0xff;
		event.GUID[4] = 0xff;
		event.GUID[5] = 0xff;
		event.GUID[6] = 0xff;
		event.GUID[7] = 0xff;
		event.GUID[8] = 0xff;
		event.GUID[9] = 0xff;
		event.GUID[10] = 0xff;
		event.GUID[11] = 0xff;
		event.GUID[12] = 0xff;
		event.GUID[13] = 0xff;
		event.GUID[14] = 0xff;
		event.GUID[15] = 0xff;

		event.pdata = new unsigned char[ 6 ];
		
		if ( NULL != event.pdata ) {

			event.sizeData = 6;
			event.pdata[ 0 ] = 11;
			event.pdata[ 1 ] = 22;
			event.pdata[ 2 ] = 33;
			event.pdata[ 3 ] = 44;
			event.pdata[ 4 ] = 55;
			event.pdata[ 5 ] = 66;
			
			if ( tcpif.doBinarySend( &event ) ) {
				writeLog("doBinarySend success.\r\n");
			}
			else {
				writeLog("doBinarySend failure.\r\n");
			}

			delete [] event.pdata;
		}
		
	
		eventex.head = VSCP_PRIORITY_3; 
		eventex.obid = 0;
		eventex.timestamp = 0;
		eventex.vscp_class = 110;
		eventex.vscp_type = 23;

		eventex.GUID[0] = 0x01;
		eventex.GUID[1] = 0x32;
		eventex.GUID[2] = 0xff;
		eventex.GUID[3] = 0xff;
		eventex.GUID[4] = 0xff;
		eventex.GUID[5] = 0xff;
		eventex.GUID[6] = 0xff;
		eventex.GUID[7] = 0xff;
		eventex.GUID[8] = 0xff;
		eventex.GUID[9] = 0xff;
		eventex.GUID[10] = 0xff;
		eventex.GUID[11] = 0xff;
		eventex.GUID[12] = 0xff;
		eventex.GUID[13] = 0xff;
		eventex.GUID[14] = 0xff;
		eventex.GUID[15] = 0xff;

		eventex.sizeData = 4;
		eventex.data[ 0 ] = 01;
		eventex.data[ 1 ] = 02;
		eventex.data[ 2 ] = 03;
		eventex.data[ 3 ] = 04;
		
		if ( tcpif.doBinarySendEx( &eventex ) ) {
			writeLog("doBinarySendEx success.\r\n");
		}
		else {
			writeLog("doBinarySendEx failure.\r\n");
		}

	}
	else {
		AfxMessageBox("Unable to enter binary mode!");
	}

	

	if ( tcpif.doBinaryClose() ) {
		writeLog("Binary Close OK.\r\n");
	}
	else {
		writeLog("Binary Close failed.\r\n");
	}

	if ( tcpif.doCmdNOOP() ) {
		writeLog("Ending NOOP OK.\r\n");
	}
	else {
		writeLog("Ending NOOP failed.\r\n");
	}

	// Close TCP/IP interface
	if ( tcpif.doCmdClose() ) {
		writeLog("TCP/IP i/f clode OK.\r\n");
	}
	else {
		writeLog("TCP/IP i/f clode failed.\r\n");
	}

	writeLog("Done.\r\n");
}



///////////////////////////////////////////////////////////////////////////////
//  OnButtonBinaryReceive
//

void CTesttcpinterfaceDlg::OnButtonBinaryReceive() 
{
	VscpTcpIf tcpif;
	vscpEvent vscpevent;
	
	m_strLog = "";
	UpdateData( false );

	AfxMessageBox(" Will now wait for Level II event");

	// Open channel
  wxString str(_("admin;secret;"));
  str += m_Hostname;
  str += _("\0");
	tcpif.doCmdOpen( str );


	// * * * BINARY * * *
	if ( tcpif.doCmdBinary() ) {
		
		// Wait for event
		while ( !tcpif.doBinaryReceive( &vscpevent ) ) {
			Sleep( 200 );
		}

		char wrkbuf[80];
		sprintf( wrkbuf, 
					"Message received; class=%d, Type=%d, data cnt=%d\r\n",
					vscpevent.vscp_class,
					vscpevent.vscp_type,
					vscpevent.sizeData );
		writeLog(wrkbuf);
		if ( NULL != vscpevent.pdata ) delete vscpevent.pdata;

		if ( tcpif.doBinaryClose() ) {
			writeLog("Binary Close OK.\r\n");
		}
		else {
			writeLog("Binary Close failed.\r\n");
		}

		if ( tcpif.doCmdNOOP() ) {
			writeLog("Ending NOOP OK.\r\n");
		}
		else {
			writeLog("Ending NOOP failed.\r\n");
		}

		// Close TCP/IP interface
		if ( tcpif.doCmdClose() ) {
			writeLog("TCP/IP i/f clode OK.\r\n");
		}
		else {
			writeLog("TCP/IP i/f clode failed.\r\n");
		}
	}
	else {
		writeLog("Failed to open channel.\r\n");
	}

	writeLog("Done.\r\n");
}




///////////////////////////////////////////////////////////////////////////////
//  OnButtonStandardReceive
//

void CTesttcpinterfaceDlg::OnButtonStandardReceive() 
{
	VscpTcpIf tcpif;
	vscpEvent event; 

	m_strLog = "";
	UpdateData( false );

	AfxMessageBox("Received Loop will now be started. About 1 minute.");

	// Open channel
  wxString str(_("admin;secret;"));
  str += m_Hostname;
  str += _("\0");
	tcpif.doCmdOpen( str );

	wxDateTime start,now;
	start.SetToCurrent();
	do {

		wxTimeSpan tt = wxDateTime::Now() - start;

		// Check for timeout
		if ( tt.GetSeconds() > 60 ) {
			break;
		}
		
		if ( tcpif.doCmdReceive( &event ) ) {
			if ( NULL != event.pdata ) delete event.pdata;
			writeLog("Message received.");
			break;
		}

		Sleep( 200 );

	} while ( true );


	// Close channel
	tcpif.doCmdClose();

}




///////////////////////////////////////////////////////////////////////////////
//  OnButtonMultiOpen
//

void CTesttcpinterfaceDlg::OnButtonMultiOpen() 
{
	int i;
	char wrkbuf[80];
	VscpTcpIf tcpif;
	
	m_strLog = "";
	UpdateData( false );

	// Open channel
  wxString str(_("admin;secret;"));
  str += m_Hostname;
  str += _("\0");
	tcpif.doCmdOpen( str );

	for ( i=0; i<25; i++ ) {
		if ( tcpif.doCmdOpen( str ) ) {
			
			sprintf( wrkbuf, 
						"Channel opened. %d\r\n",
						i );
			writeLog( wrkbuf );

			if ( tcpif.doCmdClose() ) {
				writeLog("Close channel success.\r\n");
			}
			else {
				writeLog("Failed to close channel..\r\n");
			}

		}
		else {
			writeLog("Failed to open channel..\r\n");
		}
	}


	writeLog("Done.\r\n");
}
