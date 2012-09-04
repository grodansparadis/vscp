// testPipeIf.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 2005-2011 eurosource, Sweden

#include "stdafx.h"
#include "testDriver.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestDriverApp

BEGIN_MESSAGE_MAP(CTestDriverApp, CWinApp)
	//{{AFX_MSG_MAP(CTestDriverApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDriverApp construction

CTestDriverApp::CTestDriverApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTestDriverApp object

CTestDriverApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTestDriverApp initialization

BOOL CTestDriverApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.



	CTestDriverDlg dlg;
	m_pMainWnd = &dlg;
	m_pdlg = &dlg;

	// Load resources
	dlg.m_hIconOpen = LoadIcon( IDC_TRAY_STATE0 );
	dlg.m_hIconClosed = LoadIcon( IDC_TRAY_STATE1 );

	
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
