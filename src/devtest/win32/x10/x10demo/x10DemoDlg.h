// x10DemoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "X10.h"

// Cx10DemoDlg dialog
class Cx10DemoDlg : public CDialog
{
private:
  CX10 m_x10;
  void EnableControls(BOOL fEnable = TRUE);
  char GetHouseCode();
  int GetUnitCode();
  
// Construction
public:
	Cx10DemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_X10DEMO_DIALOG };

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
  afx_msg void OnBnClickedBtnOn();
  afx_msg void OnBnClickedBtnOff();
  afx_msg void OnBnClickedBtnDim();
  afx_msg void OnBnClickedBtnBrighten();
  afx_msg void OnBnClickedBtnAllLightsOn();
  afx_msg void OnBnClickedBtnAllLightsOff();
  
private:
  CListBox m_lstHouseCodes;
  CListBox m_lstUnitCodes;
  CButton m_btnOn;
  CButton m_btnOff;
  CButton m_btnDim;
  CButton m_btnBrighten;
  CButton m_btnAllUnitsOff;
  CButton m_btnAllLightsOff;
  CButton m_btnAllLightsOn;
public:
  afx_msg void OnBnClickedBtnAllunitsoff();
};
