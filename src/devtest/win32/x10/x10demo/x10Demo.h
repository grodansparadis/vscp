// x10Demo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// Cx10DemoApp:
// See x10Demo.cpp for the implementation of this class
//

class Cx10DemoApp : public CWinApp
{
public:
	Cx10DemoApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cx10DemoApp theApp;