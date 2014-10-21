// libvscphelper.h : main header file for the canallogger.dll
// Linux version
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser  General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
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
#define VSCP_DLL_VERSION	1

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR "Grodans Paradis AB, Sweden, http://www.grodansparadis.com"

// Max number of open connections
#define VSCP_INTERFACE_MAX_OPEN	256

/////////////////////////////////////////////////////////////////////////////
// CVSCPLApp
// See vscpl1.cpp for the implementation of this class
//

class CVSCPLApp
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
