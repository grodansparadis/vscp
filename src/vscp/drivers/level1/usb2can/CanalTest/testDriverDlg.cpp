///////////////////////////////////////////////////////////////////////////////
// testDriverDlg.cpp : implementation file
//
// Copyright (c) 2005-2011 eurosource, Sweden

#include "stdafx.h"
//#include "inttypes.h"
//#include "vscp_serial.h"
//#include "com_win32.h"
#include "testDriver.h"
#include "testDriverDlg.h"
#include ".\testdriverdlg.h"

//#include "wx/ctb-0.13/getopt.h"
#if ( GPIB )
# include "wx/ctb-0.13/gpib.h"
#endif
//#include "wx/ctb-0.13/iobase.h"
//#include "wx/ctb-0.13/serport.h"
//#include "wx/ctb-0.13/timer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Prototypes
void workThreadReceive( void *pObject );

txstruct txObj; // For tranmsmit thread
char szDeviceArray[64][32];

// Global stuff for CallBack test
BOOL gbUpdate;
char gDisplayBuf[32000];
unsigned long gReceiveCount;

// Prototype for Receive callback function
void  __stdcall ReceiveCallback( canalMsg *pmsg );

extern CTestDriverApp theApp;



//int WINAPI EXPORT CanalBlockingReceive( long handle, PCANALMSG pCanalMsg, unsigned long timeout );



//extern int WINAPI EXPORT CanalCmd( long handle, LONG nr , UCHAR *data );

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
// CTestDriverDlg dialog

CTestDriverDlg::CTestDriverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDriverDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestDriverDlg)
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
	m_szOption2 = _T("");
	m_szOption1 = _T("");
	m_strConfiguration = _T("ED000002;1000");
	m_EditBurstCount = _T("");
	m_sentFrames = _T("");
	m_receivedFrames = _T("");
	m_bTimeStamp = FALSE;
	m_checkNoFetch =  FALSE;
	m_bQueueReplace = FALSE;
	m_bBlockingMode = FALSE;
	m_bSlowMode = FALSE;
	m_bNoLocalSend = FALSE;
	m_bTestCallback = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	*gDisplayBuf = 0;

	//m_cntSentFrames = 0;
    //m_cntReceivedFrames = 0;

	m_bOpen = false;
	m_drvHandle = 0;

    //m_sentFrames = "0";
	//m_receivedFrames = "0";

	m_hIconOpen = AfxGetApp()->LoadIcon( IDC_TRAY_STATE0 );
	m_hIconClosed = AfxGetApp()->LoadIcon( IDC_TRAY_STATE1 );
}

CTestDriverDlg::~CTestDriverDlg()
{
  CanalClose( m_drvHandle );
}

void CTestDriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestDriverDlg)
	DDX_Control(pDX, IDC_EDIT_STATUS, m_ctrlStatus);
	DDX_Control(pDX, IDC_EDIT_SELECTIV_FLAGS, m_ctrlSelectedFlags);
	DDX_Control(pDX, IDC_EDIT_SELECTIVE_ID, m_ctrlSelectedID);
	DDX_Control(pDX, IDC_CONECTION_STATE, m_ctrlConnectionState);
	DDX_Control(pDX, IDC_CHECK_RTR, m_ctrlCheckRTR);
	DDX_Control(pDX, IDC_CHECK_EXTENDED, m_ctrlExtended);
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
	DDX_Text(pDX, IDC_EDIT_SERIAL, m_strConfiguration);
	DDX_Text(pDX, IDC_EDIT1, m_EditBurstCount);
	DDX_Text(pDX, IDC_STATIC_SEND_COUNT, m_sentFrames);
	DDX_Text(pDX, IDC_STATIC_RECEIVE_COUNT, m_receivedFrames);
	DDX_Check(pDX, IDC_CHECK_TIMESTAMP, m_bTimeStamp);
	DDX_Check(pDX, IDC_CHECK_NOFETCH, m_checkNoFetch);
	DDX_Check(pDX, IDC_CHECK_QUEUE_REPLACE, m_bQueueReplace);
	DDX_Check(pDX, IDC_CHECK_BLOCKING_MODE, m_bBlockingMode);
	DDX_Check(pDX, IDC_CHECK_SECURE_MODE, m_bSlowMode);
	DDX_Check(pDX, IDC_CHECK_LOCAL_SEND, m_bNoLocalSend);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestDriverDlg, CDialog)
	//{{AFX_MSG_MAP(CTestDriverDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND_MSG, OnButtonSendMsg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_DATA, OnButtonClearData)
	ON_BN_CLICKED(IDC_BUTTON_GET_PROPERTIES, OnButtonGetProperties)
	ON_BN_CLICKED(IDC_BUTTON_GET_VERSION, OnButtonGetVersion)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_STATUS, OnButtonClearStatus)
	ON_BN_CLICKED(IDC_BUTTON_SEND_BURTS, OnButtonSendBurts)
	ON_BN_CLICKED(IDC_BUTTON_READFIRST, OnButtonReadfirst)
	ON_BN_CLICKED(IDC_CHECK_NOFETCH, OnCheckNofetch)
	ON_BN_CLICKED(IDC_BUTTON_BIG_BURST, OnButtonBigBurst)
	ON_BN_CLICKED(IDC_BUTTON_VIRTUAL_TEST, OnButtonVirtualTest)
	ON_BN_CLICKED(IDC_BUTTON_GET_ADAPTERS, OnButtonGetAdapters)
	ON_BN_CLICKED(IDC_CHECK_LOCAL_SEND2, OnCheckTestCallBack)
	ON_EN_CHANGE(IDC_EDIT_SERIAL, OnChangeEditSerial)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN2, OnBnClickedButtonOpen2)
	ON_MESSAGE(WM_CAN_MSG, DisplayCanData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDriverDlg message handlers

BOOL CTestDriverDlg::OnInitDialog()
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

	// Get connected CANUSB adapters
	OnButtonGetAdapters();

	// Load registry data
	loadRegistryData();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


/////////////////////////////////////////////////////////////////////////////
//

void CTestDriverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestDriverDlg::OnPaint() 
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

HCURSOR CTestDriverDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//////////////////////////////////////////////////////////////////////////////
// OnCancel
//

void CTestDriverDlg::OnCancel() 
{
	saveRegistryData();	
	CDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////////
// OnButtonOpen
//

void CTestDriverDlg::OnButtonOpen() 
{
	unsigned char *p;
	unsigned char a,b,c,d;
	unsigned long flags = 0x00000000;
	DWORD rv;

	CWaitCursor wait;

	UpdateData( true );

/*
	flags =  ( ( m_bQueueReplace ) ? CANUSB_FLAG_QUEUE_REPLACE : 0 ) |
			( ( m_bBlockingMode ) ? CANUSB_FLAG_BLOCK : 0 ) |
			( ( m_bSlowMode ) ? CANUSB_FLAG_SLOW : 0 ) |
			( ( m_bNoLocalSend ) ? CANUSB_FLAG_NO_LOCAL_SEND : 0 );
*/
	flags = 0;

	if ( m_bOpen )
	{

		m_bOpen = false;

		//if ( CANAL_ERROR_SUCCESS != canusb_Flush( m_drvHandle, FLUSH_WAIT ) ) {
		//	// Failed to flush
		//	AfxMessageBox("Failed to flush output queue (timeout)." );
		//}

        m_bRun = false;

       // KillTimer( IDD_TIMER_READ );

	// Wait for receive thread to terminate

	   while ( true )
	    {
	   	 GetExitCodeThread( m_hTreadReceive, &rv );
	  	 if ( STILL_ACTIVE != rv ) break;
	    }	        

		if ( CANAL_ERROR_SUCCESS != CanalClose( m_drvHandle ) )
		{
			// Failed to close driver
			AfxMessageBox("Failed to close CANUSB interface." );
			return;
		}
		
		m_ctrBtnOpen.SetWindowText("Open");
		m_ctrlConnectionState.SetIcon( m_hIconClosed );
	}
	else 
	{

		m_cntSentFrames = 0;
		m_cntReceivedFrames = 0;

		p = (unsigned char *)&flags;
		a = *p;
		b = *(p+1);
		c = *(p+2);
		d = *(p+3);
	
		canalMsg Msg;
		getDataValues( &Msg );

		//CanalOpen( m_strConfiguration, flags );

		if ( 0 >= ( m_drvHandle = CanalOpen( m_strConfiguration, flags ) ) )
		{		
			// Failed to open driver
			AfxMessageBox("Failed to open CANUSB interface." );
			return;
		}


	//	SetTimer( IDD_TIMER_READ, 10, NULL ); //
    //    SetTimer( IDD_TIMER_READ, USER_TIMER_MINIMUM, NULL );


    m_bRun = true;

	DWORD threadId;

	// Start read thread 
	if ( NULL == ( m_hTreadReceive = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) workThreadReceive,this,0,&threadId )))
	{

	}
	else
	{
	  //SetThreadPriority( m_hTreadReceive,THREAD_PRIORITY_TIME_CRITICAL );
	  m_bOpen = true;
	  m_ctrBtnOpen.SetWindowText("Close");
	  m_ctrlConnectionState.SetIcon( m_hIconOpen );
	}
  }
}

//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CTestDriverDlg::getDataValue( const char *szData )
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

bool CTestDriverDlg::getDataValues( canalMsg *pMsg )
{
	bool rv = false;
	unsigned char nData = 0;

	// Initialize message structure
	pMsg->flags = 0;
	pMsg->id = 0;
	pMsg->sizeData = 0;
	pMsg->timestamp = 0;

	for ( int i=0; i<8; i++ ) {
		pMsg->data[ i ] = 0;
	}

	// Must have somewhere to store data
	if ( NULL == pMsg ) return rv;

	UpdateData( true );

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

void CTestDriverDlg::writeDataToDialog( canalMsg *pMsg )
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
// OnButtonSendMsg
//

void CTestDriverDlg::OnButtonSendMsg() 
{
	canalMsg msg;
	int  rv;
  
	// Must be open
	if ( !m_bOpen ){

       AfxMessageBox("Canal is closed");
	   return;
	}

	UpdateData( true );
	getDataValues( &msg );
	if ( CANAL_ERROR_SUCCESS != ( rv = CanalBlockingSend( m_drvHandle, &msg, 500 )) ) 
	{
			if ( CANAL_ERROR_FIFO_FULL == rv )
			{
				AfxMessageBox("Failed to send message - Transmit queue full.");
			}
			else {
				AfxMessageBox("Failed to send message");
			}
			return;
	}
	else {
		m_cntSentFrames++;
		ltoa( m_cntSentFrames, m_sentFrames.GetBuffer(80), 10 );
		UpdateData( false );
		msgToStatusList( &msg, false );
	}

	UpdateData( false );
}


//////////////////////////////////////////////////////////////////////////////
// OnButtonClearData
//

void CTestDriverDlg::OnButtonClearData() 
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


///////////////////////////////////////////////////////////////////////////////
//	OnButtonGetProperties
//
//

void CTestDriverDlg::OnButtonGetProperties() 
{
	unsigned char    status;
	char buf[ 1024 ], wrkbuf[ 1024 ];
	canalStatistics  stat;

	// Must be open
	if ( !m_bOpen ) return;
	
	//status = 0;
	//status = canusb_Status( m_drvHandle );

    CanalGetStatistics(m_drvHandle,&stat);

    sprintf( buf,"Received frames=%d \r\n",stat.cntReceiveFrames);
    sprintf( wrkbuf,"Received data=%d \r\n",stat.cntReceiveData);
    strcat( buf, wrkbuf ); 
    sprintf( wrkbuf,"Overruns=%d \r\n",stat.cntOverruns);
    strcat( buf, wrkbuf ); 

    strcat( buf, "\r\n" );

	//sprintf( buf, "status=%02X\r\n", status );
	UpdateData( true );
	m_EditStatus = buf + m_EditStatus;
	UpdateData( false );
}


///////////////////////////////////////////////////////////////////////////////
//	OnButtonGetVersion
//
//

void CTestDriverDlg::OnButtonGetVersion() 
{
	char buf[ 512 ];
//	char szVersion[ 255 ];

	// Must be open
	if ( !m_bOpen ) return;

	//if ( CANAL_ERROR_SUCCESS != canusb_VersionInfo( m_drvHandle, szVersion ) ) {
	//	AfxMessageBox("Failed to get version information");
	//	return;
	//}

//	sprintf( buf, "version=%s\r\n", szVersion );
	UpdateData( true );
	m_EditStatus = buf + m_EditStatus;
	UpdateData( false );
	
}


///////////////////////////////////////////////////////////////////////////////
//	OnTimer
//
//

void CTestDriverDlg::OnTimer(UINT nIDEvent) 
{

	canalMsg msg;
	unsigned char cnt = 0;
	unsigned short len;
	unsigned short x;
/* 
    if(  CANAL_ERROR_SUCCESS == CanalReceive( m_drvHandle, &msg ) )
	 {
      m_cntReceivedFrames++;
	  msgToStatusList( &msg, true );
	 }
*/


   if( m_bOpen )
	   {
         if( (len = CanalDataAvailable( m_drvHandle ) > 0 ))
		 {
            for(x = 0; x < len;x++)
		     {
                 if(  CANAL_ERROR_SUCCESS == CanalReceive( m_drvHandle, &msg ) )
				  {
                   	m_cntReceivedFrames++;
		            msgToStatusList( &msg, true );
				  }
		    }
		 }
	   }

/*
	if ( m_bTestCallback ) {
		if ( gbUpdate  ) {
			gbUpdate = FALSE;
			UpdateData( true );
			ltoa( gReceiveCount, m_receivedFrames.GetBuffer(80), 10 );
			m_EditStatus = gDisplayBuf;
			UpdateData( false );
		}
	}
	else {
		while ( m_bOpen &&				// Should be open
				( cnt < 10 ) &&			// Max ten messages each run
				!m_checkNoFetch &&		// No auto message fetch
				( CANAL_ERROR_SUCCESS == CanalReceive( m_drvHandle, &msg ) ) ) {
			m_cntReceivedFrames++;
			msgToStatusList( &msg, true );
			cnt++;
		}
	}
*/
	CDialog::OnTimer(nIDEvent);
}


///////////////////////////////////////////////////////////////////////////////
//	msgToStatusList
//
// 

void CTestDriverDlg::msgToStatusList( canalMsg *pmsg, bool bDirOut )
{
	int i;
	//char buf[ 256 ], wrkbuf[ 10 ];
	char buf[ 1024 ], wrkbuf[ 1024 ];
	char cExtended;
	char cRTR;


    buf[0] = 0;
/*
	cExtended = 'S';
	if ( pmsg->flags & CANAL_IDFLAG_EXTENDED ) {
		cExtended = 'E';
	}

	cRTR = ' ';
	if ( pmsg->flags & CANAL_IDFLAG_RTR ) {
		cRTR = 'R';
	}

	if ( bDirOut ) {
		sprintf( buf, "<- %c%c id = %08X timestamp = %08X len = %d data =", cExtended, cRTR, pmsg->id, pmsg->timestamp, pmsg->sizeData );
	}
	else {
		sprintf( buf, "-> %c%c id = %08X timestamp = %08X len = %d data =", cExtended, cRTR, pmsg->id, pmsg->timestamp, pmsg->sizeData );
	}
	
	for ( i=0; i<pmsg->sizeData; i++ ) {
		sprintf( wrkbuf, "%02x ", pmsg->data[ i ] );
		strcat( buf, wrkbuf );
	}

	strcat( buf, "\r\n" );

*/
	UpdateData( true );
	ltoa( m_cntReceivedFrames, m_receivedFrames.GetBuffer(80), 10 );
	m_EditStatus = buf + m_EditStatus;
	UpdateData( false );
}

///////////////////////////////////////////////////////////////////////////////
//	OnButtonClearStatus
//
//

void CTestDriverDlg::OnButtonClearStatus() 
{

	m_cntSentFrames = 0;
    m_cntReceivedFrames = 0;

    m_sentFrames = "0";
	m_receivedFrames = "0";

	m_EditStatus = "";
	UpdateData( false );
}

///////////////////////////////////////////////////////////////////////////////
//	OnButtonSendBurts
//
//

void CTestDriverDlg::OnButtonSendBurts() 
{
	char buf[512];
	canalMsg msg;
	long cnt = 1;

	// Must be open
	if ( !m_bOpen ) return;

	UpdateData( true );
	getDataValues( &msg );
	cnt = getDataValue( m_EditBurstCount );
	int rv;
	for ( long i=0; i<cnt; i++ ) {

		if ( CANAL_ERROR_SUCCESS != ( rv = CanalBlockingSend( m_drvHandle, &msg , 500 ) ) ) 
		{
			if ( CANAL_ERROR_FIFO_FULL == rv )			
			  AfxMessageBox("Failed to send message - Transmit queue full.");			
            else if( CANAL_ERROR_TIMEOUT == rv )			
	          AfxMessageBox("Failed to send message - Timeout");			
			else			
			  AfxMessageBox("Failed to send message");
			
			return;
		}
		
	}

	UpdateData( true );
	m_cntSentFrames += cnt;
	ltoa( m_cntSentFrames, m_sentFrames.GetBuffer(80), 10 );
	sprintf( buf, "Burst sent of message above : cnt = %d\r\n", cnt );
	m_EditStatus = buf + m_EditStatus;
	UpdateData( false );
	msgToStatusList( &msg, false );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	OnButtonBigBurst
//
//

void CTestDriverDlg::OnButtonBigBurst() 
{
	
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonReadfirst
//

void CTestDriverDlg::OnButtonReadfirst() 
{
	char bufId[32], bufFlags[32];
	//canalMsg msg;

	UpdateData( true );	// Get dialog data
	m_ctrlSelectedID.GetWindowText( bufId,  sizeof( bufId ) );
	m_ctrlSelectedFlags.GetWindowText( bufFlags, sizeof( bufFlags ) );

/*
	if ( m_bOpen && 
			( CANAL_ERROR_SUCCESS == canusb_ReadFirst( m_drvHandle, 
														getDataValue( bufId ),
														(unsigned char)getDataValue( bufFlags ),
														&msg ) ) ) {
		m_cntReceivedFrames++;
		msgToStatusList( &msg, true );
	}
*/
}

/////////////////////////////////////////////////////////////////////////////
// OnCheckNofetch
//

void CTestDriverDlg::OnCheckNofetch() 
{
	UpdateData( true );	// Get dialog data	
}


/////////////////////////////////////////////////////////////////////////////
// OnButtonVirtualTest
//
// Opens a second virtual channel to an open interface
//

void CTestDriverDlg::OnButtonVirtualTest() 
{	
}

/////////////////////////////////////////////////////////////////////////////
// loadRegistryData
//

void CTestDriverDlg::loadRegistryData()	
{

	HKEY hk;
	DWORD val;
	DWORD lv;
	DWORD type;
	char nDataBytes = 0;
	unsigned char buf[ 512 ];
	
	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\eurosource\\testDriver",
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
			if ( ERROR_SUCCESS == RegQueryValueEx(	hk,	
														"data4",
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

 
		// * * * Timestamp check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bTimeStamp",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_bTimeStamp = 1;
		}

		// * * * Queue replace check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bQueueReplace",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_bQueueReplace = 1;
		}


		// * * * Blocking mode check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bBlockingMode",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_bBlockingMode = 1;
		}

		// * * * Slow mode check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bSlowMode",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_bSlowMode = 1;
		}

		// * * * No local send check box * * *
		lv = sizeof( DWORD );
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"bNoLocalSend",
												NULL,
												&type,
												(unsigned char *)&val,
												&lv ) ) {
			if ( val )  m_bNoLocalSend = 1;
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

		// * * * id * * *
		lv = sizeof( buf );
		*buf = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"DriverID",
												NULL,
												&type,
												buf,
												&lv ) ) {
			m_strConfiguration = buf;
		}


		// * * * burst count * * *
		lv = sizeof( buf );
		*buf = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"BurstCount",
												NULL,
												&type,
												buf,
												&lv ) ) {
			m_EditBurstCount = buf;
		}


		
		// * * * SelectedId * * * 
		lv = sizeof( buf );
		*buf = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"SelectedId",
												NULL,
												&type,
												buf,
												&lv ) ) {

			m_ctrlSelectedID.SetWindowText( (char *)buf );
		}
		
		
		// * * * SelectedFlags * * * 
		lv = sizeof( buf );
		*buf = 0;
		if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
												"SelectedFlags",
												NULL,
												&type,
												buf,
												&lv ) ) {

			m_ctrlSelectedFlags.SetWindowText( (char *)buf );
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

void CTestDriverDlg::saveRegistryData()
{
	
	canalMsg canmsg;
	HKEY hk;
	char buf[ 512 ];


	// Get screen data
	getDataValues( &canmsg );

	
	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software",
											NULL,
											KEY_ALL_ACCESS,
											&hk ) )
	{
		
		HKEY hk_preopt;
		DWORD predisp;
 
		RegCreateKeyEx( hk, "eurosource", 0L, 
							REG_OPTION_NON_VOLATILE, 
							NULL,
							KEY_ALL_ACCESS,
							NULL,
							&hk_preopt,
							&predisp );
		
		
		
		HKEY hk_opt;
		DWORD disp;
 
		RegCreateKeyEx( hk_preopt, "testDriver", 0L, 
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

		// Timestamp check box
		val =  m_bTimeStamp;
		RegSetValueEx( hk_opt,
						"bTimeStamp",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )	);

		// QueueReplace check box
		val =  m_bQueueReplace;
		RegSetValueEx( hk_opt,
						"bQueueReplace",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )	);


		// Blocking mode check box
		val =  m_bBlockingMode;
		RegSetValueEx( hk_opt,
						"bBlockingMode",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )	);

		// Slow mode check box
		val =  m_bSlowMode;
		RegSetValueEx( hk_opt,
						"bSlowMode",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )	);

		// Local send check box
		val =  m_bNoLocalSend;
		RegSetValueEx( hk_opt,
						"bNoLocalSend",
						NULL,
						REG_DWORD,
						(CONST BYTE *)&val,
						sizeof( val )	);

		// DriverID
		strcpy( buf, m_strConfiguration );
		RegSetValueEx( hk_opt,
						"DriverID",
						NULL,
						REG_SZ,
						(const unsigned char *)buf,
						strlen( buf )	);


		// BurstCount
		strcpy( buf, m_EditBurstCount );
		RegSetValueEx( hk_opt,
						"BurstCount",
						NULL,
						REG_SZ,
						(const unsigned char *)buf,
						strlen( buf )	);
		

		m_ctrlSelectedID.GetWindowText( buf,  sizeof( buf ) );
		RegSetValueEx( hk_opt,
						"SelectedId",
						NULL,
						REG_SZ,
						(const unsigned char *)buf,
						strlen( buf )	);
		
		m_ctrlSelectedFlags.GetWindowText( buf, sizeof( buf ) );
		RegSetValueEx( hk_opt,
						"SelectedFlags",
						NULL,
						REG_SZ,
						(const unsigned char *)buf,
						strlen( buf )	);
	 

		RegCloseKey( hk );
		RegCloseKey( hk_opt ); 
		RegCloseKey( hk_preopt ); 
	}
	
}

LRESULT CTestDriverDlg::DisplayCanData(WPARAM wParam, LPARAM  lParam)
{
   m_cntReceivedFrames++;
   msgToStatusList( &m_msg, true );

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//	workThreadTransmit
//	

void workThreadReceive( void *pObject )
{
	DWORD errorCode = 0;
	CTestDriverDlg *pobj = (CTestDriverDlg *)pObject;
	
	bool blocking = true;

    while(pobj->m_bRun)
	{

      if( blocking )
      {
		  if(  CANAL_ERROR_SUCCESS == CanalBlockingReceive(pobj->m_drvHandle, &pobj->m_msg , 500) )
		   {
	        pobj->PostMessage(WM_CAN_MSG, (WPARAM)1, (LPARAM)2 );           	
		   }
	  }
	   else
		 {
           if( CanalDataAvailable( pobj->m_drvHandle ) > 0 )
			   {
		         if(  CANAL_ERROR_SUCCESS == CanalReceive(pobj->m_drvHandle, &pobj->m_msg ) )
		          {
	                pobj->PostMessage(WM_CAN_MSG, (WPARAM)1, (LPARAM)2 );  
			      }
			   }
		   Sleep(1);
		 }
	} // while

	ExitThread( errorCode );
}


///////////////////////////////////////////////////////////////////////////////
//	OnButtonGetAdapters
//

void CTestDriverDlg::OnButtonGetAdapters() 
{
}

///////////////////////////////////////////////////////////////////////////////
//	OnCheckTestCallBack
//

void CTestDriverDlg::OnCheckTestCallBack() 
{
	int rv = 0;

	UpdateData( true );	// Get dialog data

	if ( m_bOpen ) {
		if ( m_bTestCallback ) {
			*gDisplayBuf = 0;
			gReceiveCount = 0;
			gbUpdate = TRUE;
			//rv = canusb_setReceiveCallBack( m_drvHandle, (LPFNDLL_RECEIVE_CALLBACK)ReceiveCallback );	
		}
		else {
			//rv = canusb_setReceiveCallBack( m_drvHandle, NULL );
		}
	}
	
}

void CTestDriverDlg::OnBnClickedButtonOpen2()
{
}




void CTestDriverDlg::OnChangeEditSerial() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}
