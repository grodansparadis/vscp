// tesifdll.h : main header file for the TESIFDLL application
//

#if !defined(AFX_TESIFDLL_H__206AA3A6_A83D_4912_8468_493523E930D3__INCLUDED_)
#define AFX_TESIFDLL_H__206AA3A6_A83D_4912_8468_493523E930D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTesifdllApp:
// See tesifdll.cpp for the implementation of this class
//

class CTesifdllApp : public CWinApp
{
public:
	CTesifdllApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTesifdllApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTesifdllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESIFDLL_H__206AA3A6_A83D_4912_8468_493523E930D3__INCLUDED_)
