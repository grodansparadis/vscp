// CanalDiagnostic.h : main header file for the CANALDIAGNOSTIC application
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: CanalDiagnostic.h,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_CANALDIAGNOSTIC_H__D7E6521D_8D1A_49E9_9F38_36244F430B38__INCLUDED_)
#define AFX_CANALDIAGNOSTIC_H__D7E6521D_8D1A_49E9_9F38_36244F430B38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticApp:
// See CanalDiagnostic.cpp for the implementation of this class
//

class CCanalDiagnosticApp : public CWinApp
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CCanalDiagnosticApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCanalDiagnosticApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CCanalDiagnosticApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANALDIAGNOSTIC_H__D7E6521D_8D1A_49E9_9F38_36244F430B38__INCLUDED_)
