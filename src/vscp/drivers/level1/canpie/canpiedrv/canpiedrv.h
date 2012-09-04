// canpiedrv.h : main header file for the CANPIEDRV DLL
//

#if !defined(AFX_CANPIEDRV_H__31622506_DF14_4D55_A8EA_61625A3955D3__INCLUDED_)
#define AFX_CANPIEDRV_H__31622506_DF14_4D55_A8EA_61625A3955D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCanpiedrvApp
// See canpiedrv.cpp for the implementation of this class
//

class CCanpiedrvApp : public CWinApp
{
public:
	CCanpiedrvApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCanpiedrvApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCanpiedrvApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANPIEDRV_H__31622506_DF14_4D55_A8EA_61625A3955D3__INCLUDED_)
