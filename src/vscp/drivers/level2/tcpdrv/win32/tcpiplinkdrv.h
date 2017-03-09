///////////////////////////////////////////////////////////////////////////////
// tcpdrv.h:
//
// This file is part is part of VSCP  & Friends
// http://www.vscp.org
//
// Copyright (C) 2000-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifndef tcpiplinkdrv_h
#define tcpiplinkdrv_h

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>
#include <vscpremotetcpif.h>


// Forward declarations
class CtcpdrvTxTread;
class CtcpdrvRxTread;
class VscpTcpIf;



#include "../common/vscptcpiplink.h"

// This is the version info for this DLL - Change to your own value
#define VSCP_DLL_VERSION		                0x000001

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR "Grodans Paradis AB, Sweden, http://www.grodansparadis.com"
							
// Driver information.
#define VSCP_TCPIPLINK_DRIVERINFO "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" \
"<!-- Version 0.0.1    2011-01-20   -->" \
"<config level=\"1|2\"blocking\"true|false\" description=\"bla bla bla bla\">" \
"   <item pos=\"0\" type=\"string\" description\"Serial number for Tellstick\"/>" \
"   <item pos=\"1\" type=\"path\" description\"Path to configuration file\"/>" \
"</config>"

// Max number of open connections
#define VSCP_TCPIPLINK_DRIVER_MAX_OPEN	    256

/////////////////////////////////////////////////////////////////////////////
// CVSCPDrvApp
//

class CVSCPDrvApp
{

public:

	/// Constructor
	CVSCPDrvApp();
	
	/// Destructor
	~CVSCPDrvApp();

	/*!
		Add a driver object

		@parm plog Object to add
		@return handle or 0 for error
	*/
	long addDriverObject( CTcpipLink *pObj );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	CTcpipLink *getDriverObject( long h );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long h );
	
	/*!
		The log file object
		This is the array with driver objects 
	*/
	CTcpipLink *m_drvObjArray[ VSCP_TCPIPLINK_DRIVER_MAX_OPEN ];

	/// Mutex for open/close
#ifdef WIN32	
	HANDLE m_objMutex;
#else
	pthread_mutex_t m_objMutex;
#endif

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

public:
	BOOL InitInstance();

};



#endif
