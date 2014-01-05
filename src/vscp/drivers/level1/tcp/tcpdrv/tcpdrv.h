// tcpdrv.h : main header file for the TCPDRV DLL
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

#if !defined(AFX_TCPDRV_H__1F4301DA_F5D2_46AD_8CA3_71AE18D44E22__INCLUDED_)
#define AFX_TCPDRV_H__1F4301DA_F5D2_46AD_8CA3_71AE18D44E22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "tcpdrvobj.h"

// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		1

// This is the vendor string - Change to your own value
#define CANAL_DLL_VENDOR "eurosource, Sweden, http://www.eurosource.se"

#define CANAL_TCP_PORT_ID 1681

/////////////////////////////////////////////////////////////////////////////
// CTcpdrvApp
// See tcpdrv.cpp for the implementation of this class
//

class CTcpdrvApp : public CWinApp
{
public:
	CTcpdrvApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTcpdrvApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTcpdrvApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TCPDRV_H__1F4301DA_F5D2_46AD_8CA3_71AE18D44E22__INCLUDED_)
