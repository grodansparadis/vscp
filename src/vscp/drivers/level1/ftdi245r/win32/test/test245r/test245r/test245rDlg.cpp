// test245rDlg.cpp : implementation file
//

#include "stdafx.h"
#include "test245r.h"
#include "test245rDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Ctest245rDlg dialog




Ctest245rDlg::Ctest245rDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Ctest245rDlg::IDD, pParent)
    , m_bOpen(false)
    , m_output(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_bOpen = false;
    m_output = 0x00;
}

void Ctest245rDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON1, m_btnOpen);
    DDX_Control(pDX, IDC_CHECK1, m_chkD0);
    DDX_Control(pDX, IDC_CHECK2, m_chkD1);
    DDX_Control(pDX, IDC_CHECK3, m_chkD2);
    DDX_Control(pDX, IDC_CHECK4, m_chkD3);
    DDX_Control(pDX, IDC_CHECK5, m_chkD4);
    DDX_Control(pDX, IDC_CHECK6, m_chkD5);
    DDX_Control(pDX, IDC_CHECK7, m_chkD6);
    DDX_Control(pDX, IDC_CHECK8, m_chkD7);
}

BEGIN_MESSAGE_MAP(Ctest245rDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON1, &Ctest245rDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_CHECK1, &Ctest245rDlg::OnBnClickedCheck1)
    ON_BN_CLICKED(IDC_CHECK2, &Ctest245rDlg::OnBnClickedCheck2)
    ON_BN_CLICKED(IDC_CHECK3, &Ctest245rDlg::OnBnClickedCheck3)
    ON_BN_CLICKED(IDC_CHECK4, &Ctest245rDlg::OnBnClickedCheck4)
    ON_BN_CLICKED(IDC_BUTTON2, &Ctest245rDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// Ctest245rDlg message handlers

BOOL Ctest245rDlg::OnInitDialog()
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

void Ctest245rDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Ctest245rDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Ctest245rDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void Ctest245rDlg::OnBnClickedButton1()
{
    FT_STATUS ftStatus;

    if ( !m_bOpen ) {
        
        m_bOpen = true;

        m_btnOpen.SetWindowTextW( _T("Disconnect") );

        // Open the device
        ftStatus = FT_Open( 0, &m_ftHandle );

        // Set asynchronious mode
        //
        // D0 - Output = 1
        // D1 - Output = 1
        // D2 - Output = 1
        // D3 - Output = 1
        // D4 - Input = 0
        // D5 - Input = 0
        // D6 - Input = 0
        // D7 - Input = 0
        FT_SetBitMode( m_ftHandle, 0x0f, 0x01 );

        // Set baudrate
        // 16 * 300 = 4800 samples per second
        FT_SetBaudRate( m_ftHandle, 300 ); 

        char buf[1];
        DWORD nWritten = 0;
        *buf = m_output;
        FT_Write( m_ftHandle, buf, 1, &nWritten  );

    }
    else {
        
        m_bOpen = false;

        m_btnOpen.SetWindowTextW( _T("Connect") );

        // Close the device
        ftStatus = FT_Close( m_ftHandle );

    }
}

void Ctest245rDlg::writeValue()
{
    // Read checkboxes
    int chk0 = m_chkD0.GetCheck();
    int chk1 = m_chkD1.GetCheck();
    int chk2 = m_chkD2.GetCheck();
    int chk3 = m_chkD3.GetCheck();

    char buf[1];
    DWORD nWritten = 0;
    m_output = chk0 + chk1 * 2 + chk2 * 4 + chk3 * 8;
    *buf = m_output;
    FT_Write( m_ftHandle, buf, 1, &nWritten  );
}

void Ctest245rDlg::OnBnClickedCheck1()
{
    writeValue();
}

void Ctest245rDlg::OnBnClickedCheck2()
{
    writeValue();
}

void Ctest245rDlg::OnBnClickedCheck3()
{
    writeValue();
}

void Ctest245rDlg::OnBnClickedCheck4()
{
    writeValue();
}

void Ctest245rDlg::OnBnClickedButton2()
{
    //while ( 1 )
    {

    char buf[100000];

    *buf = m_output;
    DWORD nWritten = 0;
    //FT_Write( m_ftHandle, buf, 1, &nWritten  );

    FT_Purge( m_ftHandle, FT_PURGE_RX );

    DWORD nRead = 0;
    FT_Read( m_ftHandle, buf, 1, &nRead  );

    if ( *buf & 16 ) {
        m_chkD4.SetCheck( 0x01 );
    }
    else {
        m_chkD4.SetCheck( 0x00 );
    }

    if ( *buf & 32 ) {
        m_chkD5.SetCheck( 0x01 );
    }
    else {
        m_chkD5.SetCheck( 0x00 );
    }

    if ( *buf & 64 ) {
        m_chkD6.SetCheck( 0x01 );
    }
    else {
        m_chkD6.SetCheck( 0x00 );
    }

    if ( *buf & 128 ) {
        m_chkD7.SetCheck( 0x01 );
    }
    else {
        m_chkD7.SetCheck( 0x00 );
    }

    UpdateData( false );
    Sleep( 10 );

}
}
