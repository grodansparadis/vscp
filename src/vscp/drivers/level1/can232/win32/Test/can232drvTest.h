// can232drvTest.h : main header file for the CAN232DRVTEST application
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

#if !defined(AFX_CAN232DRVTEST_H__2A77A4CA_1307_496A_A139_BD0B2B3781CC__INCLUDED_)
#define AFX_CAN232DRVTEST_H__2A77A4CA_1307_496A_A139_BD0B2B3781CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCan232drvTestApp:
// See can232drvTest.cpp for the implementation of this class
//

class CCan232drvTestApp : public CWinApp
{
public:
	CCan232drvTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCan232drvTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCan232drvTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAN232DRVTEST_H__2A77A4CA_1307_496A_A139_BD0B2B3781CC__INCLUDED_)
