///////////////////////////////////////////////////////////////////////////////
// canaldll.cpp : Defines the initialization routines for the DLL
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
// $RCSfile: canaldll.cpp,v $                                       
// $Date: 2005/01/05 12:16:07 $                                  
// $Author: akhe $                                              
// $Revision: 1.6 $ 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "winsock.h"
#include "../common/canal.h"
#include "canaldll.h"

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
// CCanaldllApp

BEGIN_MESSAGE_MAP(CCanaldllApp, CWinApp)
	//{{AFX_MSG_MAP(CCanaldllApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCanaldllApp construction

CCanaldllApp::CCanaldllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCanaldllApp object

CCanaldllApp theApp;


///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//	CanalOpen
//
//

extern "C" long WINAPI EXPORT CanalOpen( const char *pDevice, unsigned long flags )
{
	return theApp.m_canalif.doCmdOpen( pDevice, flags );
}


///////////////////////////////////////////////////////////////////////////////
//	CanalClose
// 

extern "C" int WINAPI EXPORT CanalClose( long handle )
{
	return theApp.m_canalif.doCmdClose();
}


///////////////////////////////////////////////////////////////////////////////
//	CanalGetLevel
// 

extern "C" unsigned long WINAPI EXPORT CanalGetLevel( long handle )
{
	return theApp.m_canalif.doCmdGetLevel();
}


///////////////////////////////////////////////////////////////////////////////
//	CanalSend
//

extern "C" int WINAPI EXPORT CanalSend( long handle, PCANALMSG pCanalMsg  )
{
	return theApp.m_canalif.doCmdSendLevel1( pCanalMsg );
}


///////////////////////////////////////////////////////////////////////////////
//	CanalReceive
//

extern "C" int WINAPI EXPORT CanalReceive( long handle,  PCANALMSG pCanalMsg  )
{
	return theApp.m_canalif.doCmdReceiveLevel1( pCanalMsg );
}

///////////////////////////////////////////////////////////////////////////////
//	CanalDataAvailable
//

extern "C" int WINAPI EXPORT CanalDataAvailable(  long handle  )
{
	return theApp.m_canalif.doCmdDataAvailable();
}

///////////////////////////////////////////////////////////////////////////////
//	CanalGetStatus
//

extern "C" int WINAPI EXPORT CanalGetStatus( long handle, PCANALSTATUS pCanalStatus  )
{
	return theApp.m_canalif.doCmdStatus( pCanalStatus);
}


///////////////////////////////////////////////////////////////////////////////
//	CanalGetStatistics
//

extern "C" int WINAPI EXPORT CanalGetStatistics( long handle,  
													PCANALSTATISTICS pCanalStatistics  )
{
	return theApp.m_canalif.doCmdStatistics( pCanalStatistics );
}


///////////////////////////////////////////////////////////////////////////////
//	CanalSetFilter
//

extern "C" int WINAPI EXPORT CanalSetFilter(  long handle, unsigned long filter )
{
	return theApp.m_canalif.doCmdFilter( filter );
}


///////////////////////////////////////////////////////////////////////////////
//	CanalSetMask
//

extern "C" int WINAPI EXPORT CanalSetMask(  long handle, unsigned long mask )
{
	return theApp.m_canalif.doCmdMask( mask );
}


///////////////////////////////////////////////////////////////////////////////
//	CanalSetBaudrate
//

extern "C" int WINAPI EXPORT CanalSetBaudrate(  long handle, unsigned long baudrate )
{
	return theApp.m_canalif.doCmdSetBaudrate( baudrate );
}

///////////////////////////////////////////////////////////////////////////////
//	CanalGetVersion
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
//	CanalGetDllVersion
//

unsigned long WINAPI EXPORT CanalGetDllVersion( void )
{
	return DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
//	CanalGetVendorString
//

const char * WINAPI EXPORT CanalGetVendorString( void )
{
	return CANAL_DLL_VENDOR;
}

