// vc6_simple_buttonDlg.cpp : implementation file
//

// For compilers that support precompilation, includes "wx/wx.h".


#include <pch.h>

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

#ifdef WIN32
#include "winsock.h"
#endif
//#include "wx/wxprec.h"
//#include "wx/wx.h"
#include "vc6_simple_button.h"
#include "vc6_simple_buttonDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonDlg dialog

CVc6_simple_buttonDlg::CVc6_simple_buttonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVc6_simple_buttonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVc6_simple_buttonDlg)
	m_szHostName = _T("localhost");
	m_nPort = 9598;
	m_szUserName = _T("admin");
	m_szPassword = _T("secret");
	m_zone = 2;
	m_subzone = 1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_bConnected = false;   // Not connected
}

void CVc6_simple_buttonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVc6_simple_buttonDlg)
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_ctrlBtnConnect);
	DDX_Control(pDX, IDC_BUTTON_OFF, m_ctrlBtnOff);
	DDX_Control(pDX, IDC_BUTTON_ON, m_ctrlBtnOn);
	DDX_Text(pDX, IDC_EDIT_HOST, m_szHostName);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_szUserName);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_szPassword);
	DDX_Text(pDX, IDC_EDIT_ZONE, m_zone);
	DDX_Text(pDX, IDC_EDIT_SUBZONE, m_subzone);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVc6_simple_buttonDlg, CDialog)
	//{{AFX_MSG_MAP(CVc6_simple_buttonDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_ON, OnButtonOn)
	ON_BN_CLICKED(IDC_BUTTON_OFF, OnButtonOff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonDlg message handlers

BOOL CVc6_simple_buttonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVc6_simple_buttonDlg::OnPaint() 
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
HCURSOR CVc6_simple_buttonDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CVc6_simple_buttonDlg::OnButtonConnect() 
{
    UpdateData();

    if ( m_bConnected ) {
        m_tcpif.doCmdClose();
        m_bConnected = false;
        m_ctrlBtnOn.EnableWindow( false );
        m_ctrlBtnOff.EnableWindow( false );
        m_ctrlBtnConnect.SetWindowText("Connect");
    }
    else {
        // Connect/Disconnect to/from the host
        if ( m_tcpif.doCmdOpen( (LPCTSTR)m_szHostName,
                                        m_nPort,
                                        (LPCTSTR)m_szUserName,
                                        (LPCTSTR)m_szPassword ) ) {
            m_bConnected = true;
            m_ctrlBtnOn.EnableWindow();
            m_ctrlBtnOff.EnableWindow();
            m_ctrlBtnConnect.SetWindowText("Disconnect");
        }
        else {
            AfxMessageBox("Failed to connect to host!");
        }
    }
	
}



void CVc6_simple_buttonDlg::OnButtonOn() 
{
	if ( m_bConnected ) {

        vscpEventEx event;
        
        UpdateData();
        event.head = VSCP_PRIORITY_NORMAL;
        event.vscp_class = VSCP_CLASS1_INFORMATION;
        event.vscp_type = VSCP_TYPE_INFORMATION_ON; 
        // Set guid for this test node (this program)
        getGuidFromStringToArray( event.GUID,
                                    "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01" );
        event.sizeData = 3;
        event.data[ 0 ] = 0;            // Index 
        event.data[ 1 ] = m_zone;       // Zone   
        event.data[ 2 ] = m_subzone;    // SubZone
        if ( CANAL_ERROR_SUCCESS != m_tcpif.doCmdSendEx( &event ) ) {
            AfxMessageBox("Failed to send ON event!");
        }
    }
}

void CVc6_simple_buttonDlg::OnButtonOff() 
{
	if ( m_bConnected ) {
        vscpEventEx event;
        
        UpdateData();
        event.head = VSCP_PRIORITY_NORMAL;
        event.vscp_class = VSCP_CLASS1_INFORMATION;
        event.vscp_type = VSCP_TYPE_INFORMATION_OFF;
        // Set guid for this test node (this program)
        getGuidFromStringToArray( event.GUID,
                                    "01:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" );
        event.sizeData = 3;
        event.data[ 0 ] = 0;            // Index 
        event.data[ 1 ] = m_zone;       // Zone   
        event.data[ 2 ] = m_subzone;    // SubZone
        if ( CANAL_ERROR_SUCCESS != m_tcpif.doCmdSendEx( &event ) ) {
            AfxMessageBox("Failed to send ON event!");
        }
    }
}

void CVc6_simple_buttonDlg::OnOK() 
{
	if ( m_bConnected ) {
        m_tcpif.doCmdClose();
    }
	
	CDialog::OnOK();
}

void CVc6_simple_buttonDlg::OnCancel() 
{
	if ( m_bConnected ) {
        m_tcpif.doCmdClose();
    }
	
	CDialog::OnCancel();
}
