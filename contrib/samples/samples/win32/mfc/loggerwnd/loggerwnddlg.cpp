// loggerWndDlg.cpp : implementation file
//

#include <pch.h>
#include "loggerWnd.h"
#include "loggerWndDlg.h"
#include "dlgfilter.h"

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
// CLoggerWndDlg dialog

CLoggerWndDlg::CLoggerWndDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoggerWndDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoggerWndDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_idx = 0;
}


void CLoggerWndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoggerWndDlg)
	DDX_Control(pDX, IDC_LIST_AREA, m_ctrFrame);
	DDX_Control(pDX, IDC_EDIT1, m_ctrlLog);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLoggerWndDlg, CDialog)
	//{{AFX_MSG_MAP(CLoggerWndDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDFILTER, OnFilter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoggerWndDlg message handlers

BOOL CLoggerWndDlg::OnInitDialog()
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

	if ( !m_canalif.doCmdOpen( "ctrl", 1 ) ) {
		AfxMessageBox("Unable to open channel to daemon! Please start canal server/service.");
		exit(-1);
	}
	

	DWORD dwStyle = LVS_AUTOARRANGE | LVS_SINGLESEL | LVS_ALIGNTOP | LVS_REPORT;
	CRect rc;
	m_ctrFrame.GetClientRect( &rc );

	m_ctrlList.Create( dwStyle,
							rc,
							this,
							IDC_LIST );

	m_ctrFrame.GetWindowRect( &rc );
	ScreenToClient( &rc );
	m_ctrlList.MoveWindow( &rc );

	m_ctrlList.SetBkColor( LIST_BACK_COLOR );
	m_ctrlList.SetTextBkColor( LIST_BACK_COLOR );
	m_ctrlList.SetTextColor( LIST_FONT_COLOR );
	m_ctrlList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	m_ctrlList.InsertColumn( 0, _T("Time"), LVCFMT_LEFT, LIST_WIDTH_TIME );
	m_ctrlList.InsertColumn( 1, _T("Timestamp"), LVCFMT_LEFT, LIST_WIDTH_TIMESTAMP );
	m_ctrlList.InsertColumn( 2, _T("Flags"), LVCFMT_LEFT, LIST_WIDTH_FLAGS );
	m_ctrlList.InsertColumn( 3, _T("Id"), LVCFMT_LEFT, LIST_WIDTH_ID );
	m_ctrlList.InsertColumn( 4, _T("Data"), LVCFMT_LEFT, rc.Width() - 
														LIST_WIDTH_TIME - 
														LIST_WIDTH_TIMESTAMP - 
														LIST_WIDTH_FLAGS - LIST_WIDTH_ID );

	m_ctrlList.ShowWindow( SW_SHOW );
	m_ctrlList.SetExtendedStyle( LVS_REPORT | LVS_EX_FULLROWSELECT | 
										LVS_EX_ONECLICKACTIVATE | 
										LVS_EX_UNDERLINEHOT | 
										LVS_EX_GRIDLINES );



	m_uTimerID = SetTimer( ID_MAIN_TIMER, 500, NULL );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLoggerWndDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLoggerWndDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLoggerWndDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CLoggerWndDlg::OnTimer(UINT nIDEvent) 
{
	fillData();
	
	CDialog::OnTimer(nIDEvent);
}

void CLoggerWndDlg::OnOK() 
{
	m_canalif.doCmdClose();	
	CDialog::OnOK();
}

void CLoggerWndDlg::OnClose() 
{
	m_canalif.doCmdClose();
	CDialog::OnClose();
}

void CLoggerWndDlg::fillData( void )
{
	int cnt;
	canalMsg Msg;

	if ( cnt = m_canalif.doCmdDataAvailable() ) {
		for ( int i=0; i<cnt; i++ ) {
			if ( m_canalif.doCmdReceive( &Msg ) ) {
				insertItem( &Msg, m_idx++ );
			}
		}
	}
}


void CLoggerWndDlg::insertItem( canalMsg *pMsg, int idx )
{
	LVITEM lvi;
	char buf[ 1024 ];
	char smallbuf[ 32 ];
	char tempbuf[ 32 ];
	time_t ltime;

	idx = 0;

	/* Get UNIX-style time and display as number and string. */
    time( &ltime );
	strcpy( tempbuf, ctime( &ltime ) );
	tempbuf[ strlen( tempbuf ) - 1 ] = 0; // Remove /n

	m_ctrlList.InsertItem( idx, "" );
	
	// Time
	sprintf( buf, "%s", tempbuf );
	lvi.mask = LVIF_TEXT;
	lvi.iItem = idx;
	lvi.iSubItem = 0;
	lvi.lParam = 1;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Timestamp
	sprintf( buf, "%08X", pMsg->timestamp );
	lvi.iItem = idx;
	lvi.iSubItem = 1;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Flags
	sprintf( buf, "%08X", pMsg->flags );
	lvi.iItem = idx;
	lvi.iSubItem = 2;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Id
	sprintf( buf, "%08X", pMsg->id );
	lvi.iItem = idx;
	lvi.iSubItem = 3;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );

	// Data
	sprintf( buf, "(%i) - " , pMsg->sizeData ); 
	for ( int i=0; i < 8; i++ ) {
		if ( i < pMsg->sizeData ) {
			sprintf( smallbuf, "%02X ", pMsg->data[ i ] ); 
			strcat( buf, smallbuf );
		}
	}
	lvi.iItem = idx;
	lvi.iSubItem = 4;
	lvi.pszText = buf;
	m_ctrlList.SetItem( &lvi );	


	if ( m_ctrlList.GetItemCount( ) > MAX_LISTITEMS ) {
		m_ctrlList.DeleteItem( MAX_LISTITEMS );
	}
}

void CLoggerWndDlg::OnFilter() 
{
	CDlgFilter dlg;

	if ( IDOK == dlg.DoModal() ) {
		unsigned long filter = atol( dlg.m_szMask );
		unsigned long mask = atol( dlg.m_szMask );
		m_canalif.doCmdMask( mask );
		m_canalif.doCmdFilter( filter );
	}
}
