// vc6_simple_button.h : main header file for the VC6_SIMPLE_BUTTON application
//

#if !defined(AFX_VC6_SIMPLE_BUTTON_H__EB34A10B_FAC1_476E_A36E_0A80F54A7586__INCLUDED_)
#define AFX_VC6_SIMPLE_BUTTON_H__EB34A10B_FAC1_476E_A36E_0A80F54A7586__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVc6_simple_buttonApp:
// See vc6_simple_button.cpp for the implementation of this class
//

class CVc6_simple_buttonApp : public CWinApp
{
public:
	CVc6_simple_buttonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVc6_simple_buttonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVc6_simple_buttonApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VC6_SIMPLE_BUTTON_H__EB34A10B_FAC1_476E_A36E_0A80F54A7586__INCLUDED_)
