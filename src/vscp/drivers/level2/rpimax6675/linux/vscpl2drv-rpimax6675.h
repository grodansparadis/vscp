// vscpl2drv-max6675.h : main header file for the canallogger.dll
// Linux version
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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


#if !defined(MAX6675_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
#define MAX6675_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include "../../../../common/canal.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/canal_macro.h"

#include "max6675.h"
#include "max6675obj.h"

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
#define VSCP_DLL_VERSION        0x000001

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR "Grodans Paradis AB, Sweden, httpa://www.grodansparadis.com"

// Driver information.
#define VSCP_RPIMAX6675_DRIVERINFO "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" \
"<!-- Version 0.0.1    2013-05-11   -->" \
"<config level=\"1|2\"blocking\"true|false\" description=\"bla bla bla bla\">" \
"   <item pos=\"0\" type=\"string\" description\"Serial number for Tellstick\"/>" \
"   <item pos=\"1\" type=\"path\" description\"Path to configuration file\"/>" \
"</config>"

// Max number of open connections
#define VSCP_RPIMAX6675_DRIVER_MAX_OPEN	    256

/////////////////////////////////////////////////////////////////////////////
// CVSCPDrvApp
// See vscpl1.cpp for the implementation of this class
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
	long addDriverObject( CRpiMax6675 *pmax6675 );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	CRpiMax6675 *getDriverObject( long h );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long h );

	/*!
		The log file object
		This is the array with driver objects (max 256 objects
	*/
	CRpiMax6675 *m_pMax6675Array[ VSCP_RPIMAX6675_DRIVER_MAX_OPEN ];
	
	
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
	CVSCPDrvApp *CreateObject( void );
}

#endif // !defined(MAX6675_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
