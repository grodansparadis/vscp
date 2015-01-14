// testpcanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "testpcan.h"
#include "testpcanDlg.h"
#include <canal.h>

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
// CTestpcanDlg dialog

CTestpcanDlg::CTestpcanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestpcanDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestpcanDlg)
	m_Status = _T("");
	m_flags = 0;
	m_DeviceString = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestpcanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestpcanDlg)
	DDX_Text(pDX, IDC_EDIT_STATUS, m_Status);
	DDX_Text(pDX, IDC_EDIT_FLAGS, m_flags);
	DDX_Text(pDX, IDC_EDIT_DEVICE_STRING, m_DeviceString);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestpcanDlg, CDialog)
	//{{AFX_MSG_MAP(CTestpcanDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnButtonTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestpcanDlg message handlers

BOOL CTestpcanDlg::OnInitDialog()
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
	
	m_Status = "Set device parameters\r\nPress test."; 
	m_flags = 1;
	m_DeviceString = "CANUSB;125";
	UpdateData( false );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestpcanDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestpcanDlg::OnPaint() 
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
HCURSOR CTestpcanDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestpcanDlg::OnButtonTest() 
{
	char buf[200 ];
	canalStatus canalstatus;

	CWaitCursor cw;
	UpdateData( true );
	m_peakObj.open( m_DeviceString, m_flags );
	m_peakObj.getStatus( &canalstatus );
	sprintf( buf, "Open call: errorcode=%lu suberrorcode=%lu, String='%s'",
			canalstatus.lasterrorcode,
			canalstatus.lasterrorsubcode,
			canalstatus.lasterrorstr );
	m_Status = "\r\n" + m_Status; 
	m_Status = buf + m_Status; 
	UpdateData( false );
	
	m_peakObj.close();
	m_peakObj.getStatus( &canalstatus );
	sprintf( buf, "Close call: errorcode=%lu suberrorcode=%lu, String='%s'",
			canalstatus.lasterrorcode,
			canalstatus.lasterrorsubcode,
			canalstatus.lasterrorstr );
	m_Status = "\r\n" + m_Status; 
	m_Status = buf + m_Status; 
	UpdateData( false );
}
