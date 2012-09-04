// test245r.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Ctest245rApp:
// See test245r.cpp for the implementation of this class
//

class Ctest245rApp : public CWinApp
{
public:
	Ctest245rApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Ctest245rApp theApp;