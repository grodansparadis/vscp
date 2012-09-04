// mr26.h : main header file for the MR26 application
//

#if !defined(AFX_MR26_H__01BEEDAB_BAB9_407C_9090_086CA958BBA0__INCLUDED_)
#define AFX_MR26_H__01BEEDAB_BAB9_407C_9090_086CA958BBA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMr26App:
// See mr26.cpp for the implementation of this class
//

class CMr26App : public CWinApp
{
public:
	CMr26App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMr26App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMr26App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MR26_H__01BEEDAB_BAB9_407C_9090_086CA958BBA0__INCLUDED_)
