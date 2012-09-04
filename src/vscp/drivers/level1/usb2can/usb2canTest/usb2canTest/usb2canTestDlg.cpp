// usb2canTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "usb2canTest.h"
#include "usb2canTestDlg.h"

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


// Cusb2canTestDlg dialog


Cusb2canTestDlg::Cusb2canTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cusb2canTestDlg::IDD, pParent)
	, m_InitString(_T("ED000000 ; 125"))
	, m_MsgId(_T("0x1FFFFFFF"))
	, m_BurstCnt(100)
	, m_Data0(_T("0x00"))
	, m_Data1(_T("0x01"))
	, m_Data2(_T("0x02"))
	, m_Data3(_T("0x03"))
	, m_Data4(_T("0x04"))
	, m_Data5(_T("0x05"))
	, m_Data6(_T("0x06"))
	, m_Data7(_T("0x07"))
	, m_loopback(FALSE)
	, m_silent(FALSE)
	, m_dar(FALSE)
	, m_enMsgStatus(FALSE)
	, m_extended(TRUE)
	, m_rtr(FALSE)
	, m_hardware(_T("-"))
	, m_firmware(_T("-"))
	, m_canal(_T("-"))
	, m_canaldll(_T("-"))
	, m_ReceivedFrame(0)
	, m_ReceivedData(0)
	, m_TransmitedFrames(0)
	, m_TransmitedData(0)
	, m_Overruns(0)
	, m_BusWarnings(0)
	, m_BusOff(0)
	, m_TxErr(0)
	, m_RxErr(0)
	, m_Send(0)
	, m_Received(0)
	, m_vendor(_T("-"))
	, m_radio(3)
	{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIconOpen = AfxGetApp()->LoadIcon( IDI_ICON2 );
	m_hIconClosed = AfxGetApp()->LoadIcon( IDI_ICON3 );

	m_receiveMutex = CreateMutex( NULL, true, RECEIVE_MUTEX );
	UNLOCK_MUTEX( m_receiveMutex );

	m_bOpen = false;
	m_bRun = false;

}

void Cusb2canTestDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_BUTTON_OPEN, m_ctrlBtnOpen);
DDX_Control(pDX, IDC_BUTTON_CLEAR_DATA, m_ctrlBtnClearData);
DDX_Control(pDX, IDC_BUTTON_SEND_MSG, m_ctrlBtnSendMsg);
DDX_Control(pDX, IDC_BUTTON_SEND_BURST, m_ctrlBtnSendBurst);
DDX_Control(pDX, IDC_BUTTON_GET_STATUS, m_ctrlBtnGetStatus);
DDX_Control(pDX, IDC_BUTTON_GET_STATISTICS, m_ctrlBtnGetStatistics);
DDX_Text(pDX, IDC_EDIT_INIT_STRING, m_InitString);
DDX_Text(pDX, IDC_EDIT_MSG_ID, m_MsgId);
DDV_MaxChars(pDX, m_MsgId, 10);
DDX_Text(pDX, IDC_EDIT_BURST_CNT, m_BurstCnt);
DDX_Text(pDX, IDC_EDIT_DATA0, m_Data0);
DDV_MaxChars(pDX, m_Data0, 4);
DDX_Text(pDX, IDC_EDIT_DATA1, m_Data1);
DDV_MaxChars(pDX, m_Data1, 4);
DDX_Text(pDX, IDC_EDIT_DATA2, m_Data2);
DDV_MaxChars(pDX, m_Data2, 4);
DDX_Text(pDX, IDC_EDIT_DATA3, m_Data3);
DDV_MaxChars(pDX, m_Data3, 4);
DDX_Text(pDX, IDC_EDIT_DATA4, m_Data4);
DDV_MaxChars(pDX, m_Data4, 4);
DDX_Text(pDX, IDC_EDIT_DATA5, m_Data5);
DDV_MaxChars(pDX, m_Data5, 4);
DDX_Text(pDX, IDC_EDIT_DATA6, m_Data6);
DDV_MaxChars(pDX, m_Data6, 4);
DDX_Text(pDX, IDC_EDIT_DATA7, m_Data7);
DDV_MaxChars(pDX, m_Data7, 4);
DDX_Check(pDX, IDC_CHECK_LOOPBACK, m_loopback);
DDX_Check(pDX, IDC_CHECK_SILENT, m_silent);
DDX_Check(pDX, IDC_CHECK_DISABLE_RETRANSMIT, m_dar);
DDX_Check(pDX, IDC_CHECK_ENABLE_STATUS, m_enMsgStatus);
DDX_Check(pDX, IDC_CHECK_EXTENDED, m_extended);
DDX_Check(pDX, IDC_CHECK_RTR, m_rtr);
DDX_Text(pDX, IDC_STATIC_HARDWARE, m_hardware);
DDX_Text(pDX, IDC_STATIC_FIRMWARE, m_firmware);
DDX_Text(pDX, IDC_STATIC_CANAL, m_canal);
DDX_Text(pDX, IDC_STATIC_CANALDLL, m_canaldll);
DDX_Text(pDX, IDC_STATIC_RECEIVED_FRAMES, m_ReceivedFrame);
DDX_Text(pDX, IDC_STATIC_RECEIVED_DATA, m_ReceivedData);
DDX_Text(pDX, IDC_STATIC_TRANSMITED_FRAMES, m_TransmitedFrames);
DDX_Text(pDX, IDC_STATIC_TRANSMITED_DATA, m_TransmitedData);
DDX_Text(pDX, IDC_STATIC_OVERRUNS, m_Overruns);
DDX_Text(pDX, IDC_STATIC_BUS_WARNINGS, m_BusWarnings);
DDX_Text(pDX, IDC_STATIC_BUS_OFF, m_BusOff);
DDX_Text(pDX, IDC_STATIC_TX_ERR, m_TxErr);
DDX_Text(pDX, IDC_STATIC_RX_ERR, m_RxErr);
DDX_Text(pDX, IDC_STATIC_SEND, m_Send);
DDX_Text(pDX, IDC_STATIC_RECEIVED, m_Received);
DDX_Control(pDX, IDC_LIST_MESSGES, m_MessagesList);
DDX_Control(pDX, IDC_STATIC_ICON, m_ctrlConnectionState);
DDX_Control(pDX, IDC_BUTTON1, m_ctrlClearList);
DDX_Control(pDX, IDC_STATIC_HARDWARE, m_ctrlHardware);
DDX_Control(pDX, IDC_STATIC_FIRMWARE, m_ctrlFirmware);
DDX_Control(pDX, IDC_STATIC_CANAL, m_ctrlCanal);
DDX_Control(pDX, IDC_STATIC_CANALDLL, m_ctrlCanalDll);
DDX_Text(pDX, IDC_STATIC_VENDOR, m_vendor);
DDX_Radio(pDX, IDC_RADIO1_BUSOFF, m_radio);
	}

BEGIN_MESSAGE_MAP(Cusb2canTestDlg, CDialog)
	ON_MESSAGE(WM_CAN_MSG, &DisplayCanData)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &Cusb2canTestDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_DATA, &Cusb2canTestDlg::OnBnClickedButtonClearData)
	ON_BN_CLICKED(IDC_BUTTON_SEND_MSG, &Cusb2canTestDlg::OnBnClickedButtonSendMsg)
	ON_BN_CLICKED(IDC_BUTTON1, &Cusb2canTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_SEND_BURST, &Cusb2canTestDlg::OnBnClickedButtonSendBurst)
    ON_BN_CLICKED(IDC_BUTTON_GET_STATISTICS, &Cusb2canTestDlg::OnBnClickedButtonGetStatistics)
	ON_BN_CLICKED(IDC_BUTTON_GET_STATUS, &Cusb2canTestDlg::OnBnClickedButtonGetStatus)	
END_MESSAGE_MAP()


// Cusb2canTestDlg message handlers

BOOL Cusb2canTestDlg::OnInitDialog()
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

void Cusb2canTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cusb2canTestDlg::OnPaint()
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
HCURSOR Cusb2canTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cusb2canTestDlg::OnBnClickedButtonOpen()
	{

	 DWORD    threadId;
	 DWORD    rv;
	 ULONG	  flags;
	 CString  str;

     UpdateData(true);

     flags = ( ( m_loopback )    ? 1 : 0 ) |
	 		 ( ( m_silent )      ? 2 : 0 ) |
			 ( ( m_dar )         ? 4 : 0 ) |
			 ( ( m_enMsgStatus ) ? 8 : 0 );    

     
	if (! m_bOpen )
	{		
	  if ( ( m_drvHandle = CanalOpen( m_InitString, flags ) ) <= 0 )
		  {
			AfxMessageBox(_T("Failed to open USB2CAN interface.") );
			m_drvHandle = 0;
			return;
		  }
		
		m_bRun = true;

        if ( NULL == ( m_hTreadReceive = CreateThread(NULL,0,
			                    (LPTHREAD_START_ROUTINE) workThreadReceive,this,0,&threadId )))

			{
             m_bRun = false; 
			 AfxMessageBox(_T("Failed to open USB2CAN interface (RX thread error).") );
			 m_drvHandle = 0;
			 return;
			}

        SetThreadPriority( m_hTreadReceive,THREAD_PRIORITY_TIME_CRITICAL );

        m_bOpen = true;
		m_ctrlBtnOpen.SetWindowText(_T("Close"));
		m_ctrlConnectionState.SetIcon( m_hIconOpen );
		m_RxCanMsg.RemoveAll();
		getVersions();
	}
	else
	{
		m_bRun = false;

		while ( true )
	    {
	   	 GetExitCodeThread( m_hTreadReceive, &rv );
	  	 if ( STILL_ACTIVE != rv ) break;
	    }

		if ( CANAL_ERROR_SUCCESS != CanalClose( m_drvHandle ) )
		{
		 AfxMessageBox("Failed to close USB2CAN interface." );
		 return;
		}

        m_drvHandle = 0;
		m_bOpen = false;

        clearVersions();
		clearStatistic();
		m_ctrlBtnOpen.SetWindowText(_T("Open"));
		m_ctrlConnectionState.SetIcon( m_hIconClosed );
	}

	 UpdateData(false);

	}

void Cusb2canTestDlg::OnBnClickedButtonClearData()
	{
	 m_Data0 = "";
	 m_Data1 = "";
	 m_Data2 = "";
	 m_Data3 = "";
	 m_Data4 = "";
	 m_Data5 = "";
	 m_Data6 = "";
	 m_Data7 = "";	      

     UpdateData(false);
	}

//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long Cusb2canTestDlg::getDataValue( const char *szData )
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

bool Cusb2canTestDlg::getDataValues( canalMsg *pMsg )
{
	bool rv = false;
	unsigned char nData = 0;

	// Must have somewhere to store data
	if ( NULL == pMsg )
		return rv;

	// Initialize message structure
	pMsg->flags = 0;
	pMsg->id = 0;
	pMsg->sizeData = 0;
	pMsg->timestamp = 0;

	for ( int i=0; i<8; i++ ) {
		pMsg->data[ i ] = 0;
	}

	UpdateData( true );

	pMsg->id = getDataValue( m_MsgId );

	// Is the ID valid?
	if ( pMsg->id  > 0x1FFFFFFF )
		return false;
	
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

    if ( m_extended )
         pMsg->flags |= CANAL_IDFLAG_EXTENDED; 

    if( m_rtr )
        pMsg->flags |= CANAL_IDFLAG_RTR; 

	// Write back data to dialog
	writeDataToDialog( pMsg );

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeDataToDialog
//

void Cusb2canTestDlg::writeDataToDialog( canalMsg *pMsg )
{

	// Must have somewhere to store data
	if ( NULL == pMsg )
		return;
	
	_ultoa_s( pMsg->id, m_MsgId.GetBufferSetLength( 32 ),32, 16 );	
	m_MsgId.Format("0x%08X",pMsg->id);

	if ( pMsg->flags & CANAL_IDFLAG_EXTENDED )
	   m_extended = true;        
	else
	   m_extended = false;	   

	if ( pMsg->id  > 0x7ff ) 
		m_extended = true;


	// RTR flag
	if ( pMsg->flags & CANAL_IDFLAG_RTR )
		m_rtr = true;	
	else
		m_rtr = false;	

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
        m_Data0.Format("0x%02X",pMsg->data[ 0 ]);
	}


    if ( pMsg->sizeData > 1 ) {
        m_Data1 = "";
        m_Data1.Format("0x%02X",pMsg->data[ 1 ]);
	}

    if ( pMsg->sizeData > 2 ) {
        m_Data2 = "";
        m_Data2.Format("0x%02X",pMsg->data[ 2 ]);
	}

    if ( pMsg->sizeData > 3 ) {
        m_Data3 = "";
        m_Data3.Format("0x%02X",pMsg->data[ 3 ]);
	}

    if ( pMsg->sizeData > 4 ) {
        m_Data4 = "";
        m_Data4.Format("0x%02X",pMsg->data[ 4 ]);
	}

    if ( pMsg->sizeData > 5 ) {
        m_Data5 = "";
        m_Data5.Format("0x%02X",pMsg->data[ 5 ]);
	}

    if ( pMsg->sizeData > 6 ) {
        m_Data6 = "";
        m_Data6.Format("0x%02X",pMsg->data[ 6 ]);
	}

    if ( pMsg->sizeData > 7 ) {
        m_Data7 = "";
        m_Data7.Format("0x%02X",pMsg->data[ 7 ]);
	}

	UpdateData( false );	
}


void Cusb2canTestDlg::OnBnClickedButtonSendMsg()
	{
	canalMsg msg;
	int  rv;
  
	// Must be open
	if ( !m_bOpen )
		{
         AfxMessageBox("Canal is closed");
	     return;
	    }

	getDataValues( &msg );

	if ( CANAL_ERROR_SUCCESS != ( rv = CanalBlockingSend( m_drvHandle, &msg, 200 )) ) 
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
	else
		{		 
		 msgToStatusList( &msg, false );
		 m_Send++;
    	}

	UpdateData( false );
}

///////////////////////////////////////////////////////////////////////////////
//	msgToStatusList
//
// 

void Cusb2canTestDlg::msgToStatusList( canalMsg *pmsg, bool bDirOut )
{
	int i;
	char wrkbuf[ 10 ];
	char cExtended;
	char cRTR;	

    CString message;
   

	// Must have somewhere to store data
	if ( NULL == pmsg )
		return;

    UCHAR StatusCode  = pmsg->data[0];
	UCHAR StatusRxCnt = pmsg->data[1];
	UCHAR StatusTxCnt = pmsg->data[2];

    if( pmsg->flags & CANAL_IDFLAG_STATUS )
		{

         switch( StatusCode )
			 {
			 case	CANAL_STATUSMSG_OK:
				 message.Format("<- Status: CANAL_STATUSMSG_OK RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_OVERRUN:
                 message.Format("<- Status: CANAL_STATUSMSG_OVERRUN RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_BUSLIGHT:
                 message.Format("<- Status: CANAL_STATUSMSG_BUSLIGHT RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_BUSHEAVY:
                 message.Format("<- Status: CANAL_STATUSMSG_BUSHEAVY RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_BUSOFF:
                 message.Format("<- Status: CANAL_STATUSMSG_BUSOFF RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUS_PHY_FAULT:
                 message.Format("<- Status: CANAL_STATUS_PHY_FAULT RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_STUFF:
                 message.Format("<- Status: CANAL_STATUSMSG_STUFF RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_FORM:
                 message.Format("<- Status: CANAL_STATUSMSG_FORM RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_ACK:
                 message.Format("<- Status: CANAL_STATUSMSG_ACK RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_BIT1:
                 message.Format("<- Status: CANAL_STATUSMSG_BIT1 RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_BIT0:
                 message.Format("<- Status: CANAL_STATUSMSG_BIT0 RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;
			 case   CANAL_STATUSMSG_CRC:
                 message.Format("<- Status: CANAL_STATUSMSG_CRC RxCnt = %d TxCnt = %d", StatusRxCnt,StatusTxCnt);
				 break;			 
		     default:
                 message.Format("<- Status: UNKNOWN STATUSMSG Code = %02X RxCnt = %d TxCnt = %d",StatusCode, StatusRxCnt,StatusTxCnt);
			 break;
			 }   
		   //message.Format("<- Status: UNKNOWN STATUSMSG Code = %02X RxCnt = %d TxCnt = %d",StatusCode, StatusRxCnt,StatusTxCnt);
		}
	else
		{
	       cExtended = 'S';
	       if ( pmsg->flags & CANAL_IDFLAG_EXTENDED )
			 {
		      cExtended = 'E';
	         }

	       cRTR = ' ';
	      if ( pmsg->flags & CANAL_IDFLAG_RTR )
			  {
	         	cRTR = 'R';
	          }

	      if ( bDirOut )			  
	         message.Format("<- %c%c id = %08X timestamp = %08X len = %d data =", cExtended, cRTR, pmsg->id, pmsg->timestamp, pmsg->sizeData);     	      
	      else 		
		     message.Format("-> %c%c id = %08X timestamp = %08X len = %d data =", cExtended, cRTR, pmsg->id, pmsg->timestamp, pmsg->sizeData );	         
	
	      for ( i=0; i<pmsg->sizeData; i++ ) {
		    sprintf_s( wrkbuf, "%02x ", pmsg->data[ i ] );
			message.Append(wrkbuf);
	      }		  		 
	   }
    
	m_MessagesList.InsertString(0, message );

    if (m_MessagesList.GetCount() > 50 )
		{
	  	 m_MessagesList.DeleteString( 50 );
		}

	UpdateData( false );
}

void Cusb2canTestDlg::OnBnClickedButton1()
	{
	 m_Send = 0;
	 m_Received = 0;

	 m_MessagesList.ResetContent();	 	

	 UpdateData(false);
	}


LRESULT Cusb2canTestDlg::DisplayCanData(WPARAM wParam, LPARAM  lParam)
{
   canalMsg msg;

   m_Received++;  

   //LOCK_MUTEX( m_receiveMutex );

   while( m_RxCanMsg.GetSize() > 0)
	   {
	    msg = m_RxCanMsg.GetHead();        
        msgToStatusList( &msg, true );
		m_RxCanMsg.RemoveHead();
	   }

   //UNLOCK_MUTEX( m_receiveMutex );

   UpdateData(false);

  return true;
}


void Cusb2canTestDlg::OnBnClickedButtonSendBurst()
	{
	canalMsg msg;
	long cnt = 1;
	CString	message;

	// Must be open
	if ( !m_bOpen ){

       AfxMessageBox("Canal is closed");
	   return;
	}

    getDataValues( &msg );

    cnt = m_BurstCnt;

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

	 m_Send += cnt;

	 message.Format("Burst sent of message above : cnt = %d", cnt );
	 m_MessagesList.InsertString(0, message );

    if (m_MessagesList.GetCount() > 100 )
		{
	  	 m_MessagesList.DeleteString( 100 );
		}

	 UpdateData( false );
	 msgToStatusList( &msg, false );	
	}


bool  Cusb2canTestDlg::getVersions(void)
	{
     CString message;
	 char *str = NULL;
	 char *pch = NULL;
  
	  str  = (char*)CanalGetVendorString();

	  if( str != NULL )
		{
           pch = strtok (str,";");
           m_hardware.Format("%s",pch);
           pch = strtok (NULL,";");
           m_firmware.Format("%s",pch);
           pch = strtok (NULL,";");
           m_canal.Format("%s",pch);
           pch = strtok (NULL,";");
           m_canaldll.Format("%s",pch);
           pch = strtok (NULL,";");
           m_vendor.Format("%s",pch);

	       UpdateData(false);
	    }
	  else
		   return false;
	  	  
	  return true;
	}

bool  Cusb2canTestDlg::clearVersions(void)
	{
  	 m_hardware.Format("-");
	 m_firmware.Format("-");
	 m_canal.Format("-");
	 m_canaldll.Format("-");
	 m_vendor.Format("-");

     UpdateData( false );

	 return true;
	}

bool  Cusb2canTestDlg::clearStatistic(void)
	{
     m_ReceivedFrame =    0;
	 m_ReceivedData  =    0;
	 m_TransmitedFrames = 0;
	 m_TransmitedData =   0;
	 m_Overruns =         0;
	 m_BusWarnings =      0;
	 m_BusOff =           0;

     UpdateData( false );

	 return true;
	}

void Cusb2canTestDlg::OnBnClickedButtonGetStatistics()
{
	 CString  message;
	 canalStatistics  stat;	 
  
	// Must be open
	if ( !m_bOpen )
		{
         AfxMessageBox("Canal is closed");
	     return;
	    }

	if ( CANAL_ERROR_SUCCESS == CanalGetStatistics( m_drvHandle,&stat ) ) 
	{
         m_ReceivedFrame =    stat.cntReceiveFrames;
		 m_ReceivedData  =    stat.cntReceiveData;
		 m_TransmitedFrames = stat.cntTransmitFrames;
		 m_TransmitedData =   stat.cntTransmitData;
		 m_Overruns =         stat.cntOverruns;
		 m_BusWarnings =      stat.cntBusWarnings;
		 m_BusOff =           stat.cntBusOff;
	}
	else
	{
        AfxMessageBox("Failed");
        return;
	}

	 UpdateData( false );
}


void Cusb2canTestDlg::OnBnClickedButtonGetStatus()
{
    canalStatus	 status;

	// Must be open
	if ( !m_bOpen )
		{
         AfxMessageBox("Canal is closed");
	     return;
	    }

	if ( CANAL_ERROR_SUCCESS == CanalGetStatus( m_drvHandle,&status ) ) 
	{
	 if( status.channel_status & CANAL_STATUS_BUS_OFF )
		 m_radio = 0;
	 else if( status.channel_status & CANAL_STATUS_PASSIVE )
		 m_radio = 1;
	 else if( status.channel_status & CANAL_STATUS_BUS_WARN )
		 m_radio = 2;
	 else
		 m_radio = 3;

     m_RxErr = (( status.channel_status >> 8 ) & 0xFF );
	 m_TxErr = ( status.channel_status & 0xFF );

     UpdateData( false );
	}
	else
		{
         AfxMessageBox("Failed");
         return;
		}

}

///////////////////////////////////////////////////////////////////////////////
//	workThreadReceive
//	

void workThreadReceive( void *pObject )
{
    canalMsg  msg;
	DWORD errorCode = 0;
	Cusb2canTestDlg *pobj = (Cusb2canTestDlg *)pObject;
	
	bool blocking = true;

    while(pobj->m_bRun)
	{

      if( blocking )
      {
	      if(  CANAL_ERROR_SUCCESS == CanalBlockingReceive(pobj->m_drvHandle, &msg , 500) )
		   {
		    //LOCK_MUTEX( pobj->m_receiveMutex );
		    pobj->m_RxCanMsg.AddTail(msg);
			//UNLOCK_MUTEX( pobj->m_receiveMutex );
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
	     }

	} // while

	ExitThread( errorCode );
}
