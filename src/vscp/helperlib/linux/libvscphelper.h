// libvscphelper.h : main header file for the canallogger.dll
// Linux version
//
// VSCP (Very Simple Control Protocol)
// http://www.vscp.org
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017 Ake Hedman, 
// Grodans Paradis AB <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//  This file is part of VSCP - Very Simple Control Protocol
//  http://www.vscp.org
//

#if !defined(AFX_VSCPL1_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
#define AFX_VSCPL1_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include "../../common/canal.h"
#include "../../common/vscpremotetcpif.h"
#include "../../common/canal_macro.h"

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// This is the version info for this DLL - Change to your own value
#define VSCP_DLL_VERSION	2

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR "Grodans Paradis AB, Sweden, http://www.grodansparadis.com"

// Max number of open connections
#define VSCP_INTERFACE_MAX_OPEN	256

/////////////////////////////////////////////////////////////////////////////
// CVSCPLApp
// See vscpl1.cpp for the implementation of this class
//

class CVSCPLApp /*: public wxApp*/
{

public:

	/// Constructor
	CVSCPLApp();
	
	/// Destructor
	~CVSCPLApp();

	/*!
		Add a driver object

		@parm plog Object to add
		@return handle or 0 for error
	*/	
	long addDriverObject( VscpRemoteTcpIf  *pvscpif );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	VscpRemoteTcpIf *getDriverObject( long handle );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long handle );

	/*!
		The log file object
		This is the array with driver objects (max 256 objects
	*/
	VscpRemoteTcpIf *m_pvscpifArray[ VSCP_INTERFACE_MAX_OPEN ];
	
	
	/// Mutex for open/close
	pthread_mutex_t m_objMutex;

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

public:
	BOOL InitInstance();

};

///////////////////////////////////////////////////////////////////////////////
// CreateObject
//

extern "C"
{
	CVSCPLApp *CreateObject( void );
}

#endif // !defined(AFX_VSCPL1_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
