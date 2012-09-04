// PhoneLog.h : main header file for the PHONELOG application
//

#if !defined(AFX_PHONELOG_H__2EA28993_C8AD_49D5_B1EC_D3CA34EF5272__INCLUDED_)
#define AFX_PHONELOG_H__2EA28993_C8AD_49D5_B1EC_D3CA34EF5272__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPhoneLogApp:
// See PhoneLog.cpp for the implementation of this class
//

class CPhoneLogApp : public CWinApp
{
public:
	CPhoneLogApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneLogApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPhoneLogApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONELOG_H__2EA28993_C8AD_49D5_B1EC_D3CA34EF5272__INCLUDED_)
