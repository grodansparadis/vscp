// dlldrvobj.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#if !defined(CHELPDLLOBJ__INCLUDED_)
#define CHELPDLLOBJ__INCLUDED_


// This is the version info for this DLL - Change to your own value
#define DLL_VERSION		0x000001

// This is the vendor string - Change to your own value
#define DLL_VENDOR "Grodans Paradis AB, Sweden, http://www.grodansparaids.com"
							
// Max number of open connections
#define VSCP_HELPER_MAX_OPEN	256

#include <canal_macro.h>
//#include <canalsuperwrapper.h>
#include <vscpremotetcpif.h>

/////////////////////////////////////////////////////////////////////////////
// CHelpDllObj
// 
//

class CHelpDllObj
{

public:

	/// Constructor
	CHelpDllObj();
	
	/// Destructor
	~CHelpDllObj();

	/*!
		Add a driver object

		@parm plog Object to add
		@return handle or 0 for error
	*/
	long addDriverObject( VscpRemoteTcpIf *pObj );

	/*!
		Get a driver object from its handle

		@param handle for object
		@return pointer to object or NULL if invalid
				handle.
	*/
	VscpRemoteTcpIf *getDriverObject( long h );

	/*!
		Remove a driver object

		@parm handle for object.
	*/
	void removeDriverObject( long h );
	
	/*!
		The log file object
		This is the array with driver objects (max 256 objects
	*/
	VscpRemoteTcpIf *m_drvObjArray[ VSCP_HELPER_MAX_OPEN ];

	/// Mutex for open/close	
    pthread_mutex_t m_mutex;

	/// Counter for users of the interface
	unsigned long m_instanceCounter;

public:
	BOOL InitInstance();

};



#endif // !defined(DLLDRVOBJ_H__A388C093_AD35_4672_8BF7_DBC702C6B0C8__INCLUDED_)
