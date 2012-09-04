// usb2canTestDlg.h : header file
//

#pragma once
#include "afxwin.h"

//#include <map>
//#include <list>

//using namespace std;

#include "../../../../common/canal.h"

#define  WM_CAN_MSG					WM_USER + 400
#define  WM_USB_EMERGENCY_MSG		WM_USER + 401
#define  WM_TEST                    WM_USER + 402

#define RECEIVE_MUTEX			"___RECEIVE_MUTEX____"
#define LOCK_MUTEX( x )		( WaitForSingleObject( x, INFINITE ) )
#define UNLOCK_MUTEX( x )	( ReleaseMutex( x ) )

void workThreadReceive( void *pObject );

// Cusb2canTestDlg dialog
class Cusb2canTestDlg : public CDialog
{
// Construction
public:
	Cusb2canTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_USB2CANTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CButton m_ctrlBtnOpen;
	afx_msg void OnBnClickedButtonOpen();
	CButton m_ctrlBtnClearData;
	CButton m_ctrlBtnSendMsg;
	CButton m_ctrlBtnSendBurst;
	CButton m_ctrlBtnGetStatus;
	CButton m_ctrlBtnGetStatistics;
	CString m_InitString;
	CString m_MsgId;
	DWORD m_BurstCnt;
	CString m_Data0;
	CString m_Data1;
	CString m_Data2;
	CString m_Data3;
	CString m_Data4;
	CString m_Data5;
	CString m_Data6;
	CString m_Data7;
	BOOL m_loopback;
	BOOL m_silent;
	BOOL m_dar;
	BOOL m_enMsgStatus;
	BOOL m_extended;
	BOOL m_rtr;
	CString m_hardware;
	CString m_firmware;
	CString m_canal;
	CString m_canaldll;
	DWORD m_ReceivedFrame;
	DWORD m_ReceivedData;
	DWORD m_TransmitedFrames;
	DWORD m_TransmitedData;
	DWORD m_Overruns;
	DWORD m_BusWarnings;
	DWORD m_BusOff;
	DWORD m_TxErr;
	DWORD m_RxErr;
	DWORD m_Send;
	DWORD m_Received;
	int m_radio;
	afx_msg void OnBnClickedButtonClearData();

	/// Handle for Open connection Icon
	HICON m_hIconOpen;
	
	/// Handle for Closed connection Icon
	HICON m_hIconClosed;

	CListBox m_MessagesList;
	CStatic m_ctrlConnectionState;
	CButton m_ctrlClearList;

    // application open.close state
	bool m_bOpen;
	// run threads
    bool m_bRun;

	// Msg for Receive Thread
	canalMsg m_msg;

	// Canal DLL device handle 
	int m_drvHandle;

	// Receive thread handle
	HANDLE  m_hTreadReceive;

	// receive mutex
	HANDLE  m_receiveMutex;

    CList<canalMsg,canalMsg&> m_RxCanMsg;

    unsigned long getDataValue( const char *szData );
    bool getDataValues( canalMsg *pMsg );
    void writeDataToDialog( canalMsg *pMsg );
	void msgToStatusList( canalMsg *pmsg, bool bDirOut );
	LRESULT DisplayCanData(WPARAM wParam, LPARAM  lParam);
	bool getVersions(void);
	bool clearVersions(void);
    bool clearStatistic(void);

	afx_msg void OnBnClickedButtonSendMsg();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonSendBurst();
	afx_msg void OnBnClickedButtonGetStatistics();
	CStatic m_ctrlHardware;
	CStatic m_ctrlFirmware;
	CStatic m_ctrlCanal;
	CStatic m_ctrlCanalDll;
	afx_msg void OnStnClickedStaticHardware();
	afx_msg void OnStnClickedStaticFirmware();
	CString m_vendor;
	afx_msg void OnStnClickedStaticReceivedData();
	afx_msg void OnBnClickedButtonGetStatus();
	
};
