// testPipeIf.cpp : Defines the class behaviors for the application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part is part of CANAL (CAN Abstraction Layer)
//  (https://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: testIf.cpp,v $                                       
// $Date: 2005/01/05 12:16:21 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include "stdafx.h"
#include "testIf.h"
#include "testIfDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfApp

BEGIN_MESSAGE_MAP(CTestPipeIfApp, CWinApp)
	//{{AFX_MSG_MAP(CTestPipeIfApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfApp construction

CTestPipeIfApp::CTestPipeIfApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTestPipeIfApp object

CTestPipeIfApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfApp initialization

BOOL CTestPipeIfApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CTestPipeIfDlg dlg;
	m_pMainWnd = &dlg;

	// Load resources
  //dlg.m_hIconOpen = GetApp()->LoadIcon( NULL, IDC_TRAY_STATE0 );
  //dlg.m_hIconClosed = ::LoadIcon( NULL, IDC_TRAY_STATE1 );


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
