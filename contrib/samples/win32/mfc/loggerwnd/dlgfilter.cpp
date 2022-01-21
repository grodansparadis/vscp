// DlgFilter.cpp : implementation file
//

#include <pch.h>
#include "loggerWnd.h"
#include "DlgFilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFilter dialog


CDlgFilter::CDlgFilter(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFilter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFilter)
	m_szMask = _T("");
	m_szFilter = _T("");
	//}}AFX_DATA_INIT
}


void CDlgFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFilter)
	DDX_Text(pDX, IDC_EDIT_MASK, m_szMask);
	DDX_Text(pDX, IDC_EDIT_FILTER, m_szFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFilter, CDialog)
	//{{AFX_MSG_MAP(CDlgFilter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFilter message handlers

void CDlgFilter::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CDlgFilter::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CDlgFilter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_szMask = "0";
	m_szFilter = "0";	
	UpdateData( false );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
