// adaptercheck.h : main header file for the ADAPTERCHECK application
//

#if !defined(AFX_ADAPTERCHECK_H__02202448_B277_47CC_B938_EE54EB7D3E39__INCLUDED_)
#define AFX_ADAPTERCHECK_H__02202448_B277_47CC_B938_EE54EB7D3E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAdaptercheckApp:
// See adaptercheck.cpp for the implementation of this class
//

class CAdaptercheckApp : public CWinApp
{
public:
	CAdaptercheckApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdaptercheckApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAdaptercheckApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADAPTERCHECK_H__02202448_B277_47CC_B938_EE54EB7D3E39__INCLUDED_)
