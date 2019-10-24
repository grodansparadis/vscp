// testDaemon.h : main header file for the TESTDAEMON application
//
// FILE: vscp.h 
//
// Copyright (C) 2002-2011 Ake Hedman akhe@eurosource.se 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: testDaemon.h,v $                                       
// $Date: 2005/01/05 12:50:56 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
//
// HISTORY:
//		021107 - AKHE	Started this file
//

#if !defined(AFX_TESTDAEMON_H__D20C9AFE_A5C1_4F52_AE85_F8802F2C201D__INCLUDED_)
#define AFX_TESTDAEMON_H__D20C9AFE_A5C1_4F52_AE85_F8802F2C201D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestDaemonApp:
// See testDaemon.cpp for the implementation of this class
//

class CTestDaemonApp : public CWinApp
{
public:
	CTestDaemonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDaemonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestDaemonApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDAEMON_H__D20C9AFE_A5C1_4F52_AE85_F8802F2C201D__INCLUDED_)
