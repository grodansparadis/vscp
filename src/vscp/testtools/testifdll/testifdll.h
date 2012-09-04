// testifdll.h : main header file for the TESTIFDLL application
//

#if !defined(AFX_TESTIFDLL_H__722FB5BF_5ED6_4F19_8E49_9619AEB1CF5F__INCLUDED_)
#define AFX_TESTIFDLL_H__722FB5BF_5ED6_4F19_8E49_9619AEB1CF5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestifdllApp:
// See testifdll.cpp for the implementation of this class
//

class CTestifdllApp : public CWinApp
{
public:
	CTestifdllApp();

	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestifdllApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestifdllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTIFDLL_H__722FB5BF_5ED6_4F19_8E49_9619AEB1CF5F__INCLUDED_)
