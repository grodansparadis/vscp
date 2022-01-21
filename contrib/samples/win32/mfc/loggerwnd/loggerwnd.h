// loggerWnd.h : main header file for the LOGGERWND application
//

#if !defined(LOGGERWND_H__B38025D1_B5E4_4E4F_91E3_2AA787075D4C__INCLUDED_)
#define LOGGERWND_H__B38025D1_B5E4_4E4F_91E3_2AA787075D4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <pch.h>
#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLoggerWndApp:
// See loggerWnd.cpp for the implementation of this class
//

class CLoggerWndApp : public CWinApp
{

public:

	CLoggerWndApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoggerWndApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLoggerWndApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(LOGGERWND_H__B38025D1_B5E4_4E4F_91E3_2AA787075D4C__INCLUDED_)
