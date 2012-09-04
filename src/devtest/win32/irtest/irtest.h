// irtest.h : main header file for the IRTEST application
//

#if !defined(AFX_IRTEST_H__D7C4E02F_0435_42CF_935C_69D8E3737466__INCLUDED_)
#define AFX_IRTEST_H__D7C4E02F_0435_42CF_935C_69D8E3737466__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIrtestApp:
// See irtest.cpp for the implementation of this class
//

class CIrtestApp : public CWinApp
{
public:
	CIrtestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIrtestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIrtestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IRTEST_H__D7C4E02F_0435_42CF_935C_69D8E3737466__INCLUDED_)
