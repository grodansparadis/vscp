// testtcpinterface.h : main header file for the TESTTCPINTERFACE application
//

#if !defined(AFX_TESTTCPINTERFACE_H__D87A56C9_5312_47CA_BF1A_F3C593482566__INCLUDED_)
#define AFX_TESTTCPINTERFACE_H__D87A56C9_5312_47CA_BF1A_F3C593482566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTesttcpinterfaceApp:
// See testtcpinterface.cpp for the implementation of this class
//

class CTesttcpinterfaceApp : public CWinApp
{
public:
	CTesttcpinterfaceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTesttcpinterfaceApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTesttcpinterfaceApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTTCPINTERFACE_H__D87A56C9_5312_47CA_BF1A_F3C593482566__INCLUDED_)
