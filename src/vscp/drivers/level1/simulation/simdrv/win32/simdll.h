///////////////////////////////////////////////////////////////////////////////
// simdll.h : main header file for the SIMDLL DLL
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: simdll.h,v $                                       
// $Date: 2005/01/05 12:16:17 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMDLL_H__1FEE3193_1708_4369_82B6_76845106FB6B__INCLUDED_)
#define AFX_SIMDLL_H__1FEE3193_1708_4369_82B6_76845106FB6B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
//#include "log.h"

// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		1

// This is the vendor string - Change to your own value
#define CANAL_DLL_VENDOR "eurosource, Sweden, http://www.eurosource.se"

/////////////////////////////////////////////////////////////////////////////
// CSimdllApp
// See simdll.cpp for the implementation of this class
//

class CSimdllApp : public CWinApp
{
public:
	CSimdllApp();

	//CLog m_log;

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimdllApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSimdllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:			
	
	
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMDLL_H__1FEE3193_1708_4369_82B6_76845106FB6B__INCLUDED_)
