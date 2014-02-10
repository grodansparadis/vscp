// testDriver.h : main header file 
//
// Copyright (C) 2000-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>



#if !defined(AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_)
#define AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "testDriverDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTestDriverApp:
// See testPipeIf.cpp for the implementation of this class
//

class CTestDriverApp : public CWinApp
{
public:
	CTestDriverApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDriverApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	// Main dialog
	CTestDriverDlg *m_pdlg;

	//{{AFX_MSG(CTestDriverApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_)
