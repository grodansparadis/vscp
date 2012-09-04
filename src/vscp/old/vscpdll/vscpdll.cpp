// vscpdll.cpp : Defines the initialization routines for the DLL.
//


#include "stdafx.h"
#include "winsock.h"
#include "vscpdll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL WINAPI EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CVscpdllApp

BEGIN_MESSAGE_MAP(CVscpdllApp, CWinApp)
	//{{AFX_MSG_MAP(CVscpdllApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVscpdllApp construction

CVscpdllApp::CVscpdllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVscpdllApp object

CVscpdllApp theApp;


///////////////////////////////////////////////////////////////////////////////
//                             V S C P -  A P I
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	vscpOpen
//
//

extern "C" long WINAPI EXPORT vscpOpen( char *pDevice, unsigned long flags )
{
	wxString str;
    str.FromAscii( pDevice );
    return theApp.m_vscpif.doCmdOpen( str, flags );
}


///////////////////////////////////////////////////////////////////////////////
//	vscpClose
// 

extern "C" int WINAPI EXPORT vscpClose( void )
{
	return theApp.m_vscpif.doCmdClose();
}

///////////////////////////////////////////////////////////////////////////////
//	vscpSend
//

extern "C" int WINAPI EXPORT vscpSend( vscpEvent *pvscpEvent  )
{
	return theApp.m_vscpif.doCmdSend( pvscpEvent );
}


///////////////////////////////////////////////////////////////////////////////
//	vscpReceive
//

extern "C" int WINAPI EXPORT vscpReceive( vscpEvent *pvscpEvent  )
{
	return theApp.m_vscpif.doCmdReceive( pvscpEvent );
}

///////////////////////////////////////////////////////////////////////////////
//	vscpDataAvailable
//

extern "C" int WINAPI EXPORT vscpDataAvailable( void )
{
	return theApp.m_vscpif.doCmdDataAvailable();
}

///////////////////////////////////////////////////////////////////////////////
//	vscpSetFilter
//

extern "C" int WINAPI EXPORT vscpSetFilter(  vscpEventFilter *pFilter )
{
	return theApp.m_vscpif.doCmdFilter( pFilter );
}








