// udpdrv.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "udpdrv.h"

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
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
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
// CUdpdrvApp

BEGIN_MESSAGE_MAP(CUdpdrvApp, CWinApp)
	//{{AFX_MSG_MAP(CUdpdrvApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUdpdrvApp construction

CUdpdrvApp::CUdpdrvApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUdpdrvApp object

CUdpdrvApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUdpdrvApp initialization

BOOL CUdpdrvApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

extern "C" long WINAPI EXPORT CanalOpen( const char *pDevice, unsigned long flags )
{
	long h = NULL;

	// Arguments
	int port = CANAL_UDP_PORT_ID;	// UDP Port
	bool bLateTimeStamp = true;	    // Timestamp set in this driver if true


	/*
	if ( NULL != pDevice ) {
		port = atoi( pDevice );	
		if ( theApp.m_udpsrv.init( port ) ) {
			h = theApp.m_instanceCounter;
		}
	}
	else {
		if ( theApp.m_udpsrv.init() ) {
			h = theApp.m_instanceCounter;
		}
	}
	*/	

	return h;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalClose
// 

extern "C" int WINAPI EXPORT CanalClose( long handle )
{
	int rv = 0;


	return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long WINAPI EXPORT CanalGetLevel( long handle )
{
	return CANAL_LEVEL_STANDARD;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int WINAPI EXPORT CanalSend( long handle, PCANALMSG pCanalMsg  )
{
	int rv = 0;

	//if ( !theApp.m_udpsrv.transmit( pCanalMsg ) ) { 
	//	rv = false;
	//}

	return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int WINAPI EXPORT CanalReceive( long handle,  PCANALMSG pCanalMsg  )
{
	int rv = 0;

	
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int WINAPI EXPORT CanalDataAvailable(  long handle  )
{
	int rv = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int WINAPI EXPORT CanalGetStatus( long handle,  PCANALSTATUS pCanalStatus  )
{
	int rv = CANAL_ERROR_SUCCESS;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int WINAPI EXPORT CanalGetStatistics( long handle,  PCANALSTATISTICS pCanalStatistics  )
{
	int rv = CANAL_ERROR_SUCCESS;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int WINAPI EXPORT CanalSetFilter(  long handle, unsigned long filter )
{
	int rv = CANAL_ERROR_SUCCESS;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int WINAPI EXPORT CanalSetMask(  long handle, unsigned long mask )
{
	int rv = CANAL_ERROR_SUCCESS;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int WINAPI EXPORT CanalSetBaudrate(  long handle, unsigned long baudrate )
{
	int rv = CANAL_ERROR_SUCCESS;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

unsigned long WINAPI EXPORT CanalGetVersion( void )
{
	unsigned long version;
	unsigned char *p = (unsigned char *)&version;

	*p = CANAL_MAIN_VERSION;
	*(p+1) = CANAL_MINOR_VERSION;
	*(p+2) = CANAL_SUB_VERSION;
	*(p+3) = 0;
	return version;
}


///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

unsigned long WINAPI EXPORT CanalGetDllVersion( void )
{
	return DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

const char * WINAPI EXPORT CanalGetVendorString( void )
{
	return CANAL_DLL_VENDOR;
}