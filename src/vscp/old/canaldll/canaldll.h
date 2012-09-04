///////////////////////////////////////////////////////////////////////////////
// canaldll.h : main header file for the CANALDLL DLL
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: canaldll.h,v $                                       
// $Date: 2005/01/05 12:16:07 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CANALDLL_H__6968021E_7AB9_4E96_A9E3_E32A8E603932__INCLUDED_)
#define AFX_CANALDLL_H__6968021E_7AB9_4E96_A9E3_E32A8E603932__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "../common/vscptcpif.h"

// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		100

// This is the vendor string - Change to your own value
#define CANAL_DLL_VENDOR "eurosource, Sweden, http://www.eurosource.se"

/////////////////////////////////////////////////////////////////////////////
// CCanaldllApp
// See canaldll.cpp for the implementation of this class
//

class CCanaldllApp : public CWinApp
{
public:
	CCanaldllApp();
  
  VscpTcpIf m_canalif;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCanaldllApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCanaldllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANALDLL_H__6968021E_7AB9_4E96_A9E3_E32A8E603932__INCLUDED_)
