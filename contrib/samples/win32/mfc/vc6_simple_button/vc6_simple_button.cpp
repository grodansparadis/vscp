// vc6_simple_button.cpp : Defines the class behaviors for the application.
//

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

#include "vc6_simple_button.h"
#include "vc6_simple_buttonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonApp

BEGIN_MESSAGE_MAP(CVc6_simple_buttonApp, CWinApp)
	//{{AFX_MSG_MAP(CVc6_simple_buttonApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonApp construction

CVc6_simple_buttonApp::CVc6_simple_buttonApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVc6_simple_buttonApp object

CVc6_simple_buttonApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonApp initialization

BOOL CVc6_simple_buttonApp::InitInstance()
{
	AfxEnableControlContainer();

    if ( !::wxInitialize() ) {
	    AfxMessageBox( "Failed to initialize the wxWindows library, aborting.");
		return FALSE;      
    }

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CVc6_simple_buttonDlg dlg;
	m_pMainWnd = &dlg;
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
