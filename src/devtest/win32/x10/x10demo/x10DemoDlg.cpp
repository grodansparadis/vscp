// x10DemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "x10Demo.h"
#include "x10DemoDlg.h"

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


// Cx10DemoDlg dialog



Cx10DemoDlg::Cx10DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cx10DemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cx10DemoDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LST_HOUSECODES, m_lstHouseCodes);
  DDX_Control(pDX, IDC_LST_UNITCODES, m_lstUnitCodes);
  DDX_Control(pDX, IDC_BTN_ON, m_btnOn);
  DDX_Control(pDX, IDC_BTN_OFF, m_btnOff);
  DDX_Control(pDX, IDC_BTN_DIM, m_btnDim);
  DDX_Control(pDX, IDC_BTN_BRIGHTEN, m_btnBrighten);
  DDX_Control(pDX, IDC_BTN_ALLUNITSOFF, m_btnAllUnitsOff);
  DDX_Control(pDX, IDC_BTN_ALLLIGHTSOFF, m_btnAllLightsOff);
  DDX_Control(pDX, IDC_BTN_ALLLIGHTSON, m_btnAllLightsOn);
}

BEGIN_MESSAGE_MAP(Cx10DemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BTN_ON, OnBnClickedBtnOn)
  ON_BN_CLICKED(IDC_BTN_OFF, OnBnClickedBtnOff)
  ON_BN_CLICKED(IDC_BTN_DIM, OnBnClickedBtnDim)
  ON_BN_CLICKED(IDC_BTN_BRIGHTEN, OnBnClickedBtnBrighten)
  ON_BN_CLICKED(IDC_BTN_ALLLIGHTSON, OnBnClickedBtnAllLightsOn)
  ON_BN_CLICKED(IDC_BTN_ALLLIGHTSOFF, OnBnClickedBtnAllLightsOff)
  ON_BN_CLICKED(IDC_BTN_ALLUNITSOFF, OnBnClickedBtnAllunitsoff)
END_MESSAGE_MAP()


// Cx10DemoDlg message handlers

BOOL Cx10DemoDlg::OnInitDialog()
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

	// Open com port
  try
  {
    m_x10.Open("COM1");
  }
  catch(...)
  {
    AfxMessageBox(_T("Error opening COM port"),MB_OK,0);
    return FALSE;
  }
  
  // Initialize listboxes
  CString s;
  for (TCHAR c = _T('A');c <= _T('P');c++)
  {
    s.Format(_T("%C"),c);
    m_lstHouseCodes.AddString(s);
  }

  for (int i = 1;i <= 16;i++)
  {
    s.Format(_T("%d"),i);
    m_lstUnitCodes.AddString(s);
  }
  
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cx10DemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cx10DemoDlg::OnPaint() 
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
HCURSOR Cx10DemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cx10DemoDlg::EnableControls(BOOL fEnable)
{
  m_lstHouseCodes.EnableWindow(fEnable);
  m_lstUnitCodes.EnableWindow(fEnable);

  m_btnOn.EnableWindow(fEnable);
  m_btnOff.EnableWindow(fEnable);
  m_btnDim.EnableWindow(fEnable);
  m_btnBrighten.EnableWindow(fEnable);
  m_btnAllLightsOn.EnableWindow(fEnable);
  m_btnAllLightsOff.EnableWindow(fEnable);
  m_btnAllUnitsOff.EnableWindow(fEnable);
}

char Cx10DemoDlg::GetHouseCode()
{
  return (char) m_lstHouseCodes.GetCurSel() + 'A';
}

int Cx10DemoDlg::GetUnitCode()
{
  return m_lstUnitCodes.GetCurSel() + 1;
}

void Cx10DemoDlg::OnBnClickedBtnOn()
{
  EnableControls(FALSE);
  m_x10.TurnOn(GetHouseCode(),GetUnitCode());
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnOff()
{
  EnableControls(FALSE);
  m_x10.TurnOff(GetHouseCode(),GetUnitCode());
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnDim()
{
  EnableControls(FALSE);
  m_x10.Dim();
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnBrighten()
{
  EnableControls(FALSE);
  m_x10.Brighten();
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnAllLightsOn()
{
  EnableControls(FALSE);
  m_x10.AllLightsOn(GetHouseCode());
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnAllLightsOff()
{
  EnableControls(FALSE);
  m_x10.AllLightsOff(GetHouseCode());
  EnableControls();
}

void Cx10DemoDlg::OnBnClickedBtnAllunitsoff()
{
  EnableControls(FALSE);
  m_x10.AllUnitsOff(GetHouseCode());
  EnableControls();
}
