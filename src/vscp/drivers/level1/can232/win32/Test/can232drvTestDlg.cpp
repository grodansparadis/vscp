// can232drvTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "can232drvTest.h"
#include "can232drvTestDlg.h"

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
// CCan232drvTestDlg dialog

CCan232drvTestDlg::CCan232drvTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCan232drvTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCan232drvTestDlg)
	m_strStatus = _T("");
	m_strMask = _T("");
	m_strCode = _T("");
	m_strData0 = _T("");
	m_strData1 = _T("");
	m_strData2 = _T("");
	m_strData3 = _T("");
	m_strData4 = _T("");
	m_strData5 = _T("");
	m_strData6 = _T("");
	m_strData7 = _T("");
	m_strDLC = _T("");
	m_strCANID = _T("");
	m_bExteded = FALSE;
	m_strOpenArg = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = NULL; // AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCan232drvTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCan232drvTestDlg)
	DDX_Text(pDX, IDC_EDIT_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_EDIT_MASK, m_strMask);
	DDX_Text(pDX, IDC_EDIT_CODE, m_strCode);
	DDX_Text(pDX, IDC_EDIT_DATA0, m_strData0);
	DDX_Text(pDX, IDC_EDIT_DATA1, m_strData1);
	DDX_Text(pDX, IDC_EDIT_DATA2, m_strData2);
	DDX_Text(pDX, IDC_EDIT_DATA3, m_strData3);
	DDX_Text(pDX, IDC_EDIT_DATA4, m_strData4);
	DDX_Text(pDX, IDC_EDIT_DATA5, m_strData5);
	DDX_Text(pDX, IDC_EDIT_DATA6, m_strData6);
	DDX_Text(pDX, IDC_EDIT_7, m_strData7);
	DDX_Text(pDX, IDC_EDIT_DLC, m_strDLC);
	DDX_Text(pDX, IDC_EDIT_CANID, m_strCANID);
	DDX_Check(pDX, IDC_CHECK_EXTENDED, m_bExteded);
	DDX_Text(pDX, IDC_EDIT_ARGS_OPEN, m_strOpenArg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCan232drvTestDlg, CDialog)
	//{{AFX_MSG_MAP(CCan232drvTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_POLL, OnButtonPoll)
	ON_BN_CLICKED(IDC_BUTTON_FILTER, OnButtonFilter)
	ON_BN_CLICKED(IDC_BUTTON2, OnButtonStatus)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_DATA, OnButtonCheckData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCan232drvTestDlg message handlers
//

BOOL CCan232drvTestDlg::OnInitDialog()
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

void CCan232drvTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCan232drvTestDlg::OnPaint() 
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
// OnQueryDragIcon
//
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.

HCURSOR CCan232drvTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/////////////////////////////////////////////////////////////////////////////
// OnButtonOpen

void CCan232drvTestDlg::OnButtonOpen() 
{
	CWaitCursor cw;
	UpdateData();
	if ( m_drvobj.open( (const char *)m_strOpenArg, 0 ) ) {
		m_strStatus = "Channel Open.\r\n" + m_strStatus;	
	}
	else {
		m_strStatus = "Failed to Open channel.\r\n" + m_strStatus;		
	}

	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonClose

void CCan232drvTestDlg::OnButtonClose() 
{
	CWaitCursor cw;
	if ( m_drvobj.close()) {
		m_strStatus = "Channel Closed.\r\n" + m_strStatus;
	}
	else {
		m_strStatus = "Failed to close channel.\r\n" + m_strStatus;
	}
	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonSend

void CCan232drvTestDlg::OnButtonSend() 
{
	unsigned char data[8];

	UpdateData();

	data[ 0 ] = atoi( m_strData0 );
	data[ 1 ] = atoi( m_strData1 );
	data[ 2 ] = atoi( m_strData2 );
	data[ 3 ] = atoi( m_strData3 );
	data[ 4 ] = atoi( m_strData4 );
	data[ 5 ] = atoi( m_strData5 );
	data[ 6 ] = atoi( m_strData6 );
	data[ 7 ] = atoi( m_strData7 );
	
	if ( m_drvobj.writeMsg( ( m_bExteded ?  true : false ), 
							atol( m_strCANID ), 
							atoi( m_strDLC ), 
							data ) ) {
		m_strStatus = "Data sent.\r\n" + m_strStatus;			
	}
	else {
		m_strStatus = "Failed to send data.\r\n" + m_strStatus;
	}

	UpdateData( false );

}

/////////////////////////////////////////////////////////////////////////////
// OnButtonPoll

void CCan232drvTestDlg::OnButtonPoll() 
{
	canalMsg Msg;

	if ( m_drvobj.readMsg( &Msg ) ) {
		char buf[256];
		
		sprintf( buf, 
					"Flag =%08X Id=%08X Flag Timestamp = %08X DLC = %1X ", 
					Msg.flags,
					Msg.id,
					Msg.timestamp,
					Msg.sizeData );

		for ( int i=0; i<Msg.sizeData; i++ ) {
			char wrk[20];
			sprintf( wrk, "%02X ", Msg.data[ i ] );
			strcat( buf, wrk );
		}

		strcat( buf, "\r\n" );
		m_strStatus = buf + m_strStatus;
	}
	else {
		m_strStatus = "No Data available.\r\n" + m_strStatus;	
	}
	
	UpdateData( false );

}

/////////////////////////////////////////////////////////////////////////////
// OnButtonFilter

void CCan232drvTestDlg::OnButtonFilter() 
{ 
	unsigned long filter = 0;
	unsigned long mask = 0xffffffff;

	UpdateData();

	sscanf( m_strCode, "%lX", &filter );
	sscanf( m_strMask, "%lX", &mask );

	if ( m_drvobj.setFilter( mask, mask ) ) {
		m_strStatus = "Filter set.\r\n" + m_strStatus;			
	}
	else {
		m_strStatus = "Failed to set filter.\r\n" + m_strStatus;
	}

	UpdateData( false );
}

/////////////////////////////////////////////////////////////////////////////
// OnOK

void CCan232drvTestDlg::OnOK() 
{
    m_drvobj.close();	
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// OnCancel

void CCan232drvTestDlg::OnCancel() 
{
	m_drvobj.close();
	CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonStatus
//

void CCan232drvTestDlg::OnButtonStatus() 
{
	 	
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonCheckData
//

void CCan232drvTestDlg::OnButtonCheckData() 
{
	int cnt;
	char buf[ 80 ];

	if ( ( cnt = m_drvobj.dataAvailable() ) ) {
		sprintf( buf, "Data available (%i messages).\r\n", cnt );
		m_strStatus = buf + m_strStatus;	
	}
	else {
		m_strStatus = "No data.\r\n" + m_strStatus;
	}

	UpdateData( false );	 
}
