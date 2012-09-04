// udpdrv.h : main header file for the UDPDRV DLL
//

#if !defined(AFX_UDPDRV_H__E25CBD40_602A_4B1A_8853_EB6231C64E1A__INCLUDED_)
#define AFX_UDPDRV_H__E25CBD40_602A_4B1A_8853_EB6231C64E1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "udpdrvobj.h"

// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		1

// This is the vendor string - Change to your own value
#define CANAL_DLL_VENDOR "eurosource, Sweden, http://www.eurosource.se"

#define CANAL_UDP_PORT_ID 1682

/////////////////////////////////////////////////////////////////////////////
// CUdpdrvApp
// See udpdrv.cpp for the implementation of this class
//

class CUdpdrvApp : public CWinApp
{
public:
	CUdpdrvApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUdpdrvApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CUdpdrvApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UDPDRV_H__E25CBD40_602A_4B1A_8853_EB6231C64E1A__INCLUDED_)
