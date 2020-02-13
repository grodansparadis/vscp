// testPipeIf.h : main header file for the TESTPIPEIF application
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part is part of CANAL (CAN Abstraction Layer)
//  (https://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: testIf.h,v $                                       
// $Date: 2005/01/05 12:16:21 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_)
#define AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestPipeIfApp:
// See testPipeIf.cpp for the implementation of this class
//

class CTestPipeIfApp : public CWinApp
{
public:
	CTestPipeIfApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestPipeIfApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestPipeIfApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPIPEIF_H__E70EF2BB_21EB_4F30_9629_A60DA344AEDC__INCLUDED_)
