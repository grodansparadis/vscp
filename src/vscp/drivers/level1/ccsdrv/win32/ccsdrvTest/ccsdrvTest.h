// ccsdrvTest.h : main header file for the CCSDRVTEST application
//

#if !defined(AFX_CCSDRVTEST_H__724C1137_14BA_4E60_9610_6AF56E960F69__INCLUDED_)
#define AFX_CCSDRVTEST_H__724C1137_14BA_4E60_9610_6AF56E960F69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCcsdrvTestApp:
// See ccsdrvTest.cpp for the implementation of this class
//

class CCcsdrvTestApp : public CWinApp
{
public:
	CCcsdrvTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCcsdrvTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCcsdrvTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCSDRVTEST_H__724C1137_14BA_4E60_9610_6AF56E960F69__INCLUDED_)
