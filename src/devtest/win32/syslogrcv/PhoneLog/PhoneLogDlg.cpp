// PhoneLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PhoneLog.h"
#include "PhoneLogDlg.h"
#include "sysloglistner.h"

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
// CPhoneLogDlg dialog

CPhoneLogDlg::CPhoneLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPhoneLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhoneLogDlg)
	m_ctrlEditStatus = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPhoneLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhoneLogDlg)
	DDX_Text(pDX, IDC_EDIT_STATUS, m_ctrlEditStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPhoneLogDlg, CDialog)
	//{{AFX_MSG_MAP(CPhoneLogDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneLogDlg message handlers

BOOL CPhoneLogDlg::OnInitDialog()
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
	
	m_listner.Create( 514/*7777*/, &m_ctrlEditStatus );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPhoneLogDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPhoneLogDlg::OnPaint() 
{
	if ( IsIconic() )
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
HCURSOR CPhoneLogDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPhoneLogDlg::OnButton1() 
{
	//VK_VOLUME_MUTE (0xAD) 
	// 0xAE = Down, ;
	// B3 Media Stop
	/*
	#define VK_ATTN           0xF6
	#define VK_CRSEL          0xF7
	#define VK_EXSEL          0xF8
	#define VK_EREOF          0xF9
	#define VK_PLAY           0xFA
	#define VK_ZOOM           0xFB
	#define VK_NONAME         0xFC
	#define VK_PA1            0xFD
	#define VK_OEM_CLEAR      0xFE

  
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = VK_CRSEL; //VK_VOLUME_MUTE;
	input.ki.wScan = 1;
	input.ki.dwFlags = 0; 
	input.ki.dwExtraInfo = GetMessageExtraInfo();
	::SendInput( 1, &input, sizeof( input ) );


	for ( int i= 0; i< 255; i++ ) {
		INPUT input;
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0xAE; //VK_VOLUME_MUTE;
		input.ki.wScan = 1;
		input.ki.dwFlags = 0; 
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		::SendInput( 1, &input, sizeof( input ) );

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0xAE; //VK_VOLUME_MUTE (0xAD) 0xAE = Down, ;
		input.ki.wScan = 1;
		input.ki.dwFlags = KEYEVENTF_KEYUP; 
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		::SendInput( 1, &input, sizeof( input ) );
	}
	*/

	 /*
	 
	 APPCOMMAND_BROWSER_BACKWARD      = 1;
     APPCOMMAND_BROWSER_FORWARD       = 2;
     APPCOMMAND_BROWSER_REFRESH       = 3;
     APPCOMMAND_BROWSER_STOP          = 4;
     APPCOMMAND_BROWSER_SEARCH        = 5;
     APPCOMMAND_BROWSER_FAVORITES     = 6;
     APPCOMMAND_BROWSER_HOME          = 7;
     APPCOMMAND_VOLUME_MUTE           = 8;
     APPCOMMAND_VOLUME_DOWN           = 9;
     APPCOMMAND_VOLUME_UP             = 10;
     APPCOMMAND_MEDIA_NEXTTRACK       = 11;
     APPCOMMAND_MEDIA_PREVIOUSTRACK   = 12;
     APPCOMMAND_MEDIA_STOP            = 13;    // bass down
     APPCOMMAND_MEDIA_PLAY_PAUSE      = 4143;
     APPCOMMAND_MEDIA_PLAY            = 4142;
     APPCOMMAND_MEDIA_PAUSE           = 4143;
     APPCOMMAND_MEDIA_RECORD          = 4144;
     APPCOMMAND_MEDIA_FASTFORWARD     = 4145;
     APPCOMMAND_MEDIA_REWIND          = 4146;
     APPCOMMAND_MEDIA_CHANNEL_UP      = 4147;
     APPCOMMAND_MEDIA_CHANNEL_DOWN    = 4148;

	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/UserInput/KeyboardInput/KeyboardInputReference/KeyboardInputMessages/WM_APPCOMMAND.asp
	APPCOMMAND_VOLUME_MUTE
	Using the Remote Control Device with Windows
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/remote_control.asp

	*/
	// Try a global action with a "Post Message" command with message 
    // ID=0x319, WParam=0, LParam=0x80000. 
    // (That's WM_APPCOMMAND and APPCOMMAND_VOLUME_MUTE.) Should work 
    // anywhere in Win2000 and WinXP.
	PostMessage( 0x319, 0, 0x80000 );
}
