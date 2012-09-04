// usb2canTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "usb2canTest.h"
#include "usb2canTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cusb2canTestApp

BEGIN_MESSAGE_MAP(Cusb2canTestApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Cusb2canTestApp construction

Cusb2canTestApp::Cusb2canTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Cusb2canTestApp object

Cusb2canTestApp theApp;


// Cusb2canTestApp initialization

BOOL Cusb2canTestApp::InitInstance()
{
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	Cusb2canTestDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
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
