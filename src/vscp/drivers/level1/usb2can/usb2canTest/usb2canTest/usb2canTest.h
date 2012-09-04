// usb2canTest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cusb2canTestApp:
// See usb2canTest.cpp for the implementation of this class
//

class Cusb2canTestApp : public CWinApp
{
public:
	Cusb2canTestApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cusb2canTestApp theApp;