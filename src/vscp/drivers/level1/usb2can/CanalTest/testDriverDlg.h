// testDriverDlg.h : header file
//
// Copyright (C) 2000-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

#if !defined(AFX_TESTDRIVERDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_)
#define AFX_TESTDRIVERDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 

//#include "dos_can.h"
#include "../../../common/canal.h"

#define  WM_CAN_MSG					WM_USER + 400
#define  WM_USB_EMERGENCY_MSG		WM_USER + 401

typedef struct {
    //uint32_t  id;
	unsigned char   channel;
	unsigned char   command;
	unsigned char   len;
	unsigned char   data[256];
}cmdMsg;

typedef struct TXSTRUCT {
	bool bRun;
	char szSpeed[ 64 ];
	unsigned long flags;
	char szDevice[ 64 ];
	char log[ 32000 ];
} txstruct;

/////////////////////////////////////////////////////////////////////////////
// CTestDriverDlg dialog

class CTestDriverDlg : public CDialog
{
// Construction
public:
	
	bool m_bRun;

	// Statistics
	long m_cntSentFrames;
	long m_cntReceivedFrames;

	// Flag for open channel
	bool m_bOpen;

    canalMsg m_msg;

	HANDLE m_hTreadReceive;

	// Handle for driver
	CANHANDLE m_drvHandle;

	// Driver object
	//ChannelObj m_channelObj;

	/// Standard constructor
	CTestDriverDlg(CWnd* pParent = NULL);	
	
	/// Destructor
	~CTestDriverDlg();

	/**
		Get a string data value (hex or decimal).

		@param Data to convert.
		@return unsigned long representing the value.
	*/
	unsigned long getDataValue( const char *szData );

	/**
		Get data from all message fields

		@param CAN message struct
		@return true for cuccess, false otherwise.
	*/
	bool getDataValues( canalMsg *pMsg );

	/**
		Write data from a CAN structure to the dialog.
		
		@param CAN message struct
	*/
	void writeDataToDialog( canalMsg *pMsg );

	/**
		Load persistent data from the registry
	*/
	void loadRegistryData( void );

	/**
		Save persistent data to the registry
	*/
	void saveRegistryData( void );

	/*!
		Put message on statuslist
	*/
	void msgToStatusList( canalMsg *pMsg, bool bDirOut=true );

	/// Handle for Open connection Icon
	HICON m_hIconOpen;
	
	/// Handle for Closed connection Icon
	HICON m_hIconClosed;

// Dialog Data
	//{{AFX_DATA(CTestDriverDlg)
	enum { IDD = IDD_TESTDRIVER_DIALOG };
	CEdit	m_ctrlStatus;
	CEdit	m_ctrlSelectedFlags;
	CEdit	m_ctrlSelectedID;
	CStatic	m_ctrlConnectionState;
	CStatic	m_ctrlOption2Label;
	CStatic	m_ctrlOption1Label;
	CEdit	m_ctrlEditOption2;
	CButton	m_ctrlCheckError;
	CButton	m_ctrlCheckRTR;
	CButton	m_ctrlExtended;
	CComboBox	m_ctrlComboMsgSelect;
	CButton	m_ctrBtnOpen;
	CComboBox	m_ctrlComboMsg;
	CString	m_EditStatus;
	CString	m_MsgId;
	CString	m_Data0;
	CString	m_Data1;
	CString	m_Data2;
	CString	m_Data3;
	CString	m_Data4;
	CString	m_Data5;
	CString	m_Data6;
	CString	m_Data7;
	CString	m_szOption2;
	CString	m_szOption1;
	CString	m_strConfiguration;
	CString	m_EditBurstCount;
	CString	m_sentFrames;
	CString	m_receivedFrames;
	BOOL	m_bTimeStamp;
	BOOL	m_checkNoFetch;
	BOOL	m_bQueueReplace;
	BOOL	m_bBlockingMode;
	BOOL	m_bSlowMode;
	BOOL	m_bNoLocalSend;
	BOOL	m_bTestCallback;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDriverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestDriverDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	afx_msg void OnButtonSendMsg();
	afx_msg void OnButtonOpen();
	afx_msg void OnButtonClearData();
	afx_msg void OnButtonGetProperties();
	afx_msg void OnButtonGetVersion();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonClearStatus();
	afx_msg void OnButtonSendBurts();
	afx_msg void OnButtonReadfirst();
	afx_msg void OnCheckNofetch();
	afx_msg void OnButtonBigBurst();
	afx_msg void OnButtonVirtualTest();
	afx_msg void OnButtonGetAdapters();
	afx_msg void OnButtonTransfer();
	afx_msg void OnCheckTestCallBack();
	afx_msg void OnChangeEditSerial();
	afx_msg LRESULT DisplayCanData(WPARAM wParam, LPARAM  lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen2();
	afx_msg void OnStnClickedStaticSendCount();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDRIVERDLG_H__C2A9C8BA_4FD0_4137_9408_3CF60AE86FD1__INCLUDED_)
