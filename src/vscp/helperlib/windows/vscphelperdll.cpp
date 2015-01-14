// VSCP helper dll.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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


#include "wx/wxprec.h"
#include "wx/wx.h"

#include <stdio.h>
#include <stdlib.h>

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/utils.h>

#include "../../common/vscphelper.h"
#include "../../common/vscpremotetcpif.h"
#include "dlldrvobj.h"
#include "vscphelperdll.h"

#ifdef _MSC_VER
   #include <wx/msw/msvcrt.h>
#endif

static HANDLE hThisInstDll = NULL;
CHelpDllObj theApp;

///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hInstDll, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved )
{
	switch( ul_reason_for_call ) {

		case DLL_PROCESS_ATTACH:
			hThisInstDll = hInstDll;
            wxInitialize();
			theApp.InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
            wxUninitialize();
 			break;
   }

	return TRUE;

}



